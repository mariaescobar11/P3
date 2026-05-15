/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <algorithm> 
#include <vector>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -p, --pot FLOAT       llindar de potència per la decisió sonor/sord [Default: -51]
    -1, --r1norm FLOAT    llindar de correlació de 1 per la decisió sonor/sord [Default: 0.38]
    -M, --rmaxnorm FLOAT  llindar de correlació al max secundari per la decisió sonor/sord [Default: 0.38]
    -z, --zcr FLOAT       llindar de taxa de zero cross rate [Default: 0.24]
    -h, --help  Show this screen
    --version   Show the version of the project
    
Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();
  float llindar_pot = stof(args["--pot"].asString());
  float llindar_zcr = stof(args["--zcr"].asString());
  float llindar_r1norm = stof(args["--r1norm"].asString());
  float llindar_rmaxnorm = stof(args["--rmaxnorm"].asString());

  
  
  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer --> Constructor, passar llindars
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::HAMMING, 50, 500, llindar_pot, llindar_r1norm, llindar_rmaxnorm, llindar_zcr);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.

  
// 1. Encontrar el valor máximo absoluto del frame para calcular el umbral
/*float max_val = 0;
for (const auto& sample : x) {
    if (fabs(sample) > max_val) max_val = fabs(sample);
}

// 2. Definir el umbral (típicamente entre el 10% y el 30%, probemos con 30% -> 0.3)
float clipping_threshold = max_val * 0.1;

// 3. Aplicar el clipping a cada muestra del frame
for (auto& sample : x) {
    if (sample > clipping_threshold) {
        sample -= clipping_threshold;
    } else if (sample < -clipping_threshold) {
        sample += clipping_threshold;
    } else {
        sample = 0;
    }
}
*/

//Preprocesado: filtro paso-bajo
if (x.size() > 5) {
      vector<float> x_filtered = x;
      int navg = 5; // Tamaño de la ventana del filtro
      
      for (size_t i = navg; i < x.size(); ++i) {
          float sum = 0.0;
          for (int j = 0; j < navg; ++j) {
              sum += x[i - j];
          }
          x_filtered[i] = sum / navg;
      }
      x = x_filtered; // Sustituimos la señal original por la filtrada
  }


  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.

  /// \DONE
  /// Hemos implementado un filtro de mediana de longitud 3 como etapa de postprocesado. 
  /// Este filtro analiza cada valor junto su anterior y su posterior valor para eliminar errores atípicos.
  /// Al seleccionar el valor central de la ventana ordenada, conseguimos suavizar la curva de f0
  /// y reducir el Gross Voiced Error.

  /// Postprocess: Filtro de mediana de longitud 3
  vector<float> f0_filtered = f0; // Copia para no modificar mientras leemos

  for (size_t i = 1; i < f0.size() - 1; ++i) {
      // Tenemos que crear un vector con tres valores para poder hacer la mediana --> valor anterior, el actual y el siguiente
      vector<float> vectdemediana = {f0[i-1], f0[i], f0[i+1]};
      
      // Ordenamos los 3 valores
      sort(vectdemediana.begin(), vectdemediana.end());
      
      // El valor del medio (índice 1) es la mediana
      f0_filtered[i] = vectdemediana[1];
  }
  f0 = f0_filtered; // Actualizamos el vector original


  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
