/// @file

#include <iostream>
#include <cmath>
#include <ffft/FFTReal.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {

    for (unsigned int l = 0; l < r.size(); ++l) {
    /// \TODO Compute the autocorrelation r[l]
     /** 
      \DONE Autocorrelación calculada
     \f[
     r[l] = \frac{1}{N} \sum_{n=l}^{n=N} x[n] \cdot x[n-l]
     \f]


     1. Inicialitzem \f$r[l]\f$ a zero
     2. Acumulem el producte de \f$x[n]\f$ per \f$x[n-l]\f$ per a \f$n\f$ des de \f$l\f$ fins a \f$N-1\f$
     3. Dividim \f$r[l]\f$ entre \f$N\f$.
     */

      r[l]=0.0F;
      for (unsigned int n=l; n<x.size();n++){
        r[l] += x[n]*x[n-l];
      }
      r[l] = r[l]/x.size();
      
    }

    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \TODO Implement the Hamming window
      /** 
      \DONE Ventana de Hamming implementada
      \f[
      w[n] = 0.54 - 0.46 \cdot \cos\left(\frac{2\pi n}{N-1}\right)
      \f]*/
      
      for (unsigned int n = 0; n < frameLen; ++n) {
          window[n] = 0.54 - 0.46 * cos(2 * M_PI * n / (frameLen - 1));
      }

      break;
    case RECT:
    default:
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm, float zcr) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.
    
    /** * \DONE
     * Implementación de la decisión sonoro/sordo (voiced/unvoiced):
     * La señal se considera sonora (false) si la correlación de primer orden (r1norm)
     * o la correlación en el máximo secundario (rmaxnorm) superan el umbral de 0.6.
     * En caso contrario, se considera sorda (true).
     * Además la potencia i la zcr también se tiene en cuenta:
     * Si la potencia es inferior al umbral es sordo.
     * Si la tasa de cruce por cero es alta es sordo
     */
    
      /// Si la potencia es inferior al umbral es sordo
      if (pot < llindar_pot) {
          return true; 
      }

      /// Si la periodicidad es baja, es sordo (unvoiced)
      /// Usamos los nombres de variables definidas en el docopt
      if (r1norm < llindar_r1norm || rmaxnorm < llindar_rmaxnorm) {
          return true;
        }
    /// Si la tasa de cruce por cero es alta es sordo
      if (zcr > llindar_zcr) {
        return true;
        }

      return false;

  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    if (x.size() != frameLen)
      return -1.0F;

    /// Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max), c(npitch_max);


    float pot = 0.0f;
    for (unsigned int i=0; i<x.size(); ++i) pot += x[i]*x[i];
    pot = 10 * log10(pot / x.size() + 1e-10f);

      autocorrelation(x, r);
        // busquem maxim secundari de c 
      if (activar_ceps)cepstrum(x, c);

   
   vector<float>::const_iterator iR, iRMax, iter;

    /// \TODO 
	  /// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
	  /// Choices to set the minimum value of the lag are:
  	///    - The first negative value of the autocorrelation.
  	///    - The lag corresponding to the maximum value of the pitch.
    ///	   .
  	/// In either case, the lag should not exceed that of the minimum value of the pitch.

   /***\DONE 
   * Hemos localizado el lag del valor máximo de la autocorrelación fuera del origen.
   * Se ha optado por iniciar la búsqueda en npitch_min, que corresponde al periodo 
   * de la frecuencia de pitch máxima permitida (opción 2 del enunciado).
   */

    bool usar_cepstrum = activar_ceps;
    iter = usar_cepstrum ? c.begin() : r.begin();
    for(iR= iRMax = iter + npitch_min ; iR < iter + npitch_max ; iR++){
          if (*iR > *iRMax){
             iRMax =iR;
         }
      }
    
    unsigned int lag = iRMax - iter;

    // Si hem usat el cepstrum, el pic d'autocorrelació pot estar lleugerament desplaçat.
    // Busquem el màxim local de l'autocorrelació al voltant del lag trobat.
    float r_max_val = r[lag];
    if (activar_ceps) {
        int search_range = 3; // Marge de cerca
        for (int k = -search_range; k <= search_range; ++k) {
            int current_lag = lag + k;
            if (current_lag >= 0 && current_lag < (int)r.size()) {
                if (r[current_lag] > r_max_val) {
                    r_max_val = r[current_lag];
                }
            }
        }
    }

    // pot already calculated
    
    // Calcular el numero de cruces por cero
    float zcr = 0.0f;
    for (unsigned int i = 1; i < x.size(); ++i) {
        if (x[i] * x[i-1] < 0.0f) {
            zcr += 1.0f;
        }
    }
    zcr = zcr / x.size();

    // Unvoiced decision using power, r[1]/r[0], r_max_val/r[0], and zcr
    if (unvoiced(pot, r[1]/r[0], r_max_val/r[0], zcr)) {
        return 0;
    }

    return (float) samplingFreq/(float) lag;
  }

  void PitchAnalyzer::cepstrum(const vector<float> &x, vector<float> &c) const {
    // 1. Zero Padding (cal fer-la amb mida potència de 2, tipus 2^ceil(log2(N)))
    // 2. |X| = sqrt(real^2 + imag^2)
    // 3. log(|X| + epsilon)
    // 4. IFFT del log-espectre
    // 5. c[n] = part real de la IFFT
   
   // Assegurar que N cobreix fins a lag=320 sense fer aliasing (N > 2*npitch_max)
   unsigned int N = 1024;
   while (N < x.size() * 2) N <<= 1;
   ffft::FFTReal<float> fft(N);
   // 1. Pre-èmfasi i Zero-padding
      vector<float> buf(N, 0.0f);
      buf[0] = x[0];
      for(unsigned int i = 1; i < x.size(); ++i) {
          buf[i] = x[i] - 0.97f * x[i-1]; // Filtre de pre-èmfasi
      }
    
   // 2. FFT
      vector<float> spec(N);
      fft.do_fft(spec.data(), buf.data());

   // 3. Log-magnitud (format packed)
      vector<float> logmag(N/2 + 1);
      for (unsigned int k = 0; k <= N/2; ++k) {
        float re = spec[k];
        float im = (k == 0 || k == N/2) ? 0.0f : spec[N/2 + k];
        logmag[k] = log(sqrt(re*re + im*im) + 1e-10f);
      }

   // 4. Omplir part real, imag = 0
      fill(spec.begin(), spec.end(), 0.0f);
      copy(logmag.begin(), logmag.end(), spec.begin());

   // 5. IFFT → cepstrum
      fft.do_ifft(spec.data(), buf.data());
      fft.rescale(buf.data());

   // 6. Copiar a c
      for (unsigned int i = 0; i < c.size(); ++i)
        c[i] = buf[i];
}

    


}

