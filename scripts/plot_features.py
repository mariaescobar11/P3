import sys
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 4:
    print("Ús: python plot_features.py <arxiu_features.txt> <el_teu_pitch.f0> <pitch_referencia.f0ref>")
    sys.exit(1)

features_file = sys.argv[1]
f0_file = sys.argv[2]
f0ref_file = sys.argv[3]

# 1. LLEGIM LES DADES
# get_pitch imprimeix a stdout: pot, r1norm, rmaxnorm, zcr
try:
    features = np.loadtxt(features_file)
    pot = features[:, 0]
    r1norm = features[:, 1]
    rmaxnorm = features[:, 2]
    zcr = features[:, 3]
except Exception as e:
    print(f"Error llegint features: {e}")
    sys.exit(1)

# El .f0 té un 0 extra a l'inici i al final afegits per get_pitch.cpp
f0 = np.loadtxt(f0_file)
if len(f0) == len(pot) + 2:
    f0 = f0[1:-1] # Retallem els extrems perquè coincideixi amb les features

f0ref = np.loadtxt(f0ref_file)

# Ajustem les mides per si hi ha petites diferències de longitud
min_len = min(len(f0), len(pot), len(f0ref))
f0 = f0[:min_len]
f0ref = f0ref[:min_len]
pot = pot[:min_len]
r1norm = r1norm[:min_len]
rmaxnorm = rmaxnorm[:min_len]
zcr = zcr[:min_len]

# L'eix X és el temps (cada trama avança 15ms segons l'enunciat)
temps = np.arange(min_len) * 0.015

# ==========================================
# GRÀFICA 1: COMPARATIVA DE PITCH (El teu vs Referència/Wavesurfer)
# ==========================================
plt.figure(figsize=(10, 5))
# Piquem els zeros com a NaN perquè no surtin línies verticals lletges
f0_plot = np.where(f0 == 0, np.nan, f0)
f0ref_plot = np.where(f0ref == 0, np.nan, f0ref)

plt.plot(temps, f0ref_plot, 's', color='lightgray', markersize=6, label='Referència (Wavesurfer)')
plt.plot(temps, f0_plot, '.', color='blue', markersize=4, label='El teu Pitch')
plt.title('Comparativa Estimador de Pitch')
plt.xlabel('Temps (s)')
plt.ylabel('F0 (Hz)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('grafica_comparativa_pitch.png', dpi=300)
print("-> Gràfica desada com a 'grafica_comparativa_pitch.png'")

# ==========================================
# GRÀFICA 2: ELS CRITERIS DE SONORITAT (pot, r1, rmax, zcr)
# ==========================================
fig, axs = plt.subplots(4, 1, figsize=(10, 10), sharex=True)

# Potència
axs[0].plot(temps, pot, 'g-')
axs[0].axhline(y=-52, color='r', linestyle='--', alpha=0.7, label='Llindar (-52 dB)')
axs[0].set_ylabel('Potència (dB)')
axs[0].set_title('Evolució de les característiques (Criteris de Sonoritat)')
axs[0].legend(loc="upper right")
axs[0].grid(True)

# r1norm
axs[1].plot(temps, r1norm, 'm-')
axs[1].axhline(y=0.25, color='r', linestyle='--', alpha=0.7, label='Llindar (0.25)')
axs[1].set_ylabel('r1norm')
axs[1].legend(loc="upper right")
axs[1].grid(True)

# rmaxnorm
axs[2].plot(temps, rmaxnorm, 'c-')
axs[2].axhline(y=0.25, color='r', linestyle='--', alpha=0.7, label='Llindar (0.25)')
axs[2].set_ylabel('rmaxnorm')
axs[2].legend(loc="upper right")
axs[2].grid(True)

# ZCR
axs[3].plot(temps, zcr, 'k-')
axs[3].axhline(y=0.10, color='r', linestyle='--', alpha=0.7, label='Llindar (0.10)')
axs[3].set_ylabel('ZCR')
axs[3].set_xlabel('Temps (s)')
axs[3].legend(loc="upper right")
axs[3].grid(True)

plt.tight_layout()
plt.savefig('grafica_criteris_sonoritat.png', dpi=300)
print("-> Gràfica desada com a 'grafica_criteris_sonoritat.png'")
