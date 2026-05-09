import matplotlib
matplotlib.use('Agg')
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

# Llegir l'arxiu d'àudio
fs, data = wavfile.read('prueba.wav')

# Assegurar-nos que és mono
if len(data.shape) > 1:
    data = data[:, 0]

# Si és sencer de 16 bits, convertim a float i normalitzem
if data.dtype == np.int16:
    data = data.astype(np.float32) / 32768.0
else:
    data = data.astype(np.float32)
data = data / np.max(np.abs(data))

# Buscarem un segment sonor de manera automàtica: el de més energia
duracio = 0.030 # 30 ms
mida_finestra = int(duracio * fs)
energia = np.convolve(data**2, np.ones(mida_finestra), mode='valid')
inici_mostra = np.argmax(energia)
final_mostra = inici_mostra + mida_finestra
segment = data[inici_mostra:final_mostra]

start_time = inici_mostra / fs

# Vector de temps pel segment
temps_segment = np.linspace(0, duracio, len(segment), endpoint=False) * 1000 # en ms

# Càlcul de l'autocorrelació
autocorr = np.correlate(segment, segment, mode='full')
# Ens quedem amb la meitat dreta (lags positius)
autocorr = autocorr[len(autocorr)//2:]
# Normalitzem respecte al màxim (a lag 0)
if np.max(autocorr) != 0:
    autocorr = autocorr / np.max(autocorr)

# Trobem el primer màxim secundari (el pitch)
# Trobem on la derivada canvia de negativa a positiva (mínim) o passa per zero
zero_crossings = np.where(np.diff(np.signbit(autocorr)))[0]
if len(zero_crossings) > 0:
    first_zero = zero_crossings[0]
else:
    first_zero = 20 # valor per defecte si no creua zero

# Limitem la cerca a freqüències humanes raonables (ex. 50Hz a 500Hz)
min_lag = int(fs / 500) # correspond a 500Hz
max_lag = int(fs / 50)  # correspond a 50Hz

# Busquem a partir del primer pas per zero o mínim raonable
start_search = max(first_zero, min_lag)
end_search = min(max_lag, len(autocorr))

if start_search < end_search:
    max_idx = np.argmax(autocorr[start_search:end_search]) + start_search
else:
    max_idx = start_search

pitch_period_samples = max_idx
pitch_period_ms = (pitch_period_samples / fs) * 1000
pitch_freq = fs / pitch_period_samples if pitch_period_samples > 0 else 0

# Creació de la figura
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

# Subplot 1: Senyal temporal
ax1.plot(temps_segment, segment, color='b')
ax1.set_title(f'Segment sonor de {duracio*1000:.0f} ms (Inici: {start_time:.3f} s)')
ax1.set_xlabel('Temps (ms)')
ax1.set_ylabel('Amplitud')
ax1.grid(True)

# Marcar el període de pitch en el senyal temporal
# Dibuixem línies verticals separades pel període de pitch estimat
if pitch_period_ms > 0:
    num_periods = int(duracio * 1000 / pitch_period_ms)
    for i in range(1, num_periods + 1):
        ax1.axvline(x=i * pitch_period_ms, color='r', linestyle='--', alpha=0.7)
    # Afegim llegenda manualment per a les línies de pitch
    ax1.plot([], [], color='r', linestyle='--', label=f'Període de pitch (T={pitch_period_ms:.2f} ms)')
    ax1.legend()

# Subplot 2: Autocorrelació
lags_ms = (np.arange(len(autocorr)) / fs) * 1000
ax2.plot(lags_ms, autocorr, color='g')
ax2.set_title('Autocorrelació del segment')
ax2.set_xlabel('Retard (ms)')
ax2.set_ylabel('Autocorrelació normalitzada')
ax2.grid(True)

# Marcar el primer màxim secundari
if pitch_period_ms > 0:
    ax2.plot(lags_ms[max_idx], autocorr[max_idx], 'ro', markersize=8, 
             label=f'Màxim secundari\n(T = {pitch_period_ms:.2f} ms, F0 = {pitch_freq:.1f} Hz)')
    ax2.legend()

plt.tight_layout()
plt.savefig('grafica_pitch.png', dpi=150)
print(f'Gràfica guardada a grafica_pitch.png')
print(f'Freqüència fonamental estimada: {pitch_freq:.1f} Hz')
