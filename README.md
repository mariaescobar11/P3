PAV - P3: estimación de pitch
=============================

Esta práctica se distribuye a través del repositorio GitHub [Práctica 3](https://github.com/albino-pav/P3).
Siga las instrucciones de la [Práctica 2](https://github.com/albino-pav/P2) para realizar un `fork` de la
misma y distribuir copias locales (*clones*) del mismo a los distintos integrantes del grupo de prácticas.

Recuerde realizar el *pull request* al repositorio original una vez completada la práctica.

Ejercicios básicos
------------------

- Complete el código de los ficheros necesarios para realizar la estimación de pitch usando el programa
  `get_pitch`.

   * Complete el cálculo de la autocorrelación e inserte a continuación el código correspondiente.
   
   ```cpp
      for (unsigned int l = 0; l < r.size(); ++l) {
             r[l]=0.0F;
            for (unsigned int n=l; n<x.size();n++){
              r[l] += x[n]*x[n-l];
            }
            r[l] = r[l]/x.size();  
    }
   ```
   * Inserte una gŕafica donde, en un *subplot*, se vea con claridad la señal temporal de un segmento de
     unos 30 ms de un fonema sonoro y su periodo de pitch; y, en otro *subplot*, se vea con claridad la
	 autocorrelación de la señal y la posición del primer máximo secundario.
    
    ![senyal i correlació](img/grafica_pitch.png)

	 NOTA: es más que probable que tenga que usar Python, Octave/MATLAB u otro programa semejante para
	 hacerlo. Se valorará la utilización de la biblioteca matplotlib de Python.

   * Determine el mejor candidato para el periodo de pitch localizando el primer máximo secundario de la
     autocorrelación. Inserte a continuación el código correspondiente.

    ```cpp
    for(iR= r.begin() + npitch_min; iR < r.begin() + npitch_max ; iR++){
        if (*iR > *iRMax){
           iRMax =iR;
         }
    }
    
    unsigned int lag = iRMax - r.begin();

    float pot = 10 * log10(r[0]);
    ```

   * Implemente la regla de decisión sonoro o sordo e inserte el código correspondiente.

    ```cpp
    if (r1norm > 0.6 || rmaxnorm > 0.6){
      return false;
    }
      return true;
    ```
    Implementación de la decisión sonoro/sordo (voiced/unvoiced):
     * La señal se considera sonora (false) si la correlación de primer orden (r1norm) o la correlación en el máximo secundario (rmaxnorm) superan el umbral de 0.6. En caso contrario, se considera sorda (true).

   * Puede serle útil seguir las instrucciones contenidas en el documento adjunto `código.pdf`.

- Una vez completados los puntos anteriores, dispondrá de una primera versión del estimador de pitch. El 
  resto del trabajo consiste, básicamente, en obtener las mejores prestaciones posibles con él.

  * Utilice el programa `wavesurfer` para analizar las condiciones apropiadas para determinar si un
    segmento es sonoro o sordo. 
	
	  - Inserte una gráfica con la estimación de pitch incorporada a `wavesurfer` y, junto a ella, los 
	    principales candidatos para determinar la sonoridad de la voz: el nivel de potencia de la señal
		(r[0]), la autocorrelación normalizada de uno (r1norm = r[1] / r[0]) y el valor de la
		autocorrelación en su máximo secundario (rmaxnorm = r[lag] / r[0]).

		Puede considerar, también, la conveniencia de usar la tasa de cruces por cero.

	    Recuerde configurar los paneles de datos para que el desplazamiento de ventana sea el adecuado, que
		en esta práctica es de 15 ms.

      - Use el estimador de pitch implementado en el programa `wavesurfer` en una señal de prueba y compare
	    su resultado con el obtenido por la mejor versión de su propio sistema.  Inserte una gráfica
		ilustrativa del resultado de ambos estimadores.
     
		Aunque puede usar el propio Wavesurfer para obtener la representación, se valorará
	 	el uso de alternativas de mayor calidad (particularmente Python).
  
  * Optimice los parámetros de su sistema de estimación de pitch e inserte una tabla con las tasas de error
    y el *score* TOTAL proporcionados por `pitch_evaluate` en la evaluación de la base de datos 
	`pitch_db/train`..

Para maximizar la precisión del estimador de pitch, hemos ajustado los umbrales de decisión sonor/sord (unvoiced) a los valores óptimos de -52 dB para la potencia y 0.6 para las correlaciones (Correlación al primer desplazamiento (r1norm) y máximo de la autocorrelación secundaria (rmaxnorm)), además de implementar la ventana de Hamming.

Originalmente, el sistema solo evaluaba la periodicidad mediante la autocorrelación. Hemos mejorado esto añadiendo un umbral de potencia que actúa como filtro previo para eliminar el ruido de fondo. Al descartar los fragmentos con baja energía antes de analizar la autocorrelación, hemos conseguido eliminar prácticamente todos los falsos positivos en las zonas de silencio o ruido.

Además hemos cambiado la lógica cuando miramos la autocorrelación para detectar si es sordo o sonoro, ya que haciéndolo de la forma de antes (si el señal superaba el umbral se le asignaba como señal sonoro) era mucho más permisivo que haciéndolo al revés (si el señal no supera el umbral se asigna como sordo). 

Esta nueva forma es mucho más robusta porque, para que un frame sea detectado como sonoro, ahora debe cumplir todas las condiciones simultáneamente (energía suficiente y alta periodicidad en ambos parámetros).

Con estos cambios hemos pasado de un 64% a un 93%.

Nueva regla de decisión:

```cpp
    if (pot < llindar_pot) {
        return true; 
    }

    if (r1norm < llindar_r1norm || rmaxnorm < llindar_rmaxnorm) {
        return true;
    }
    return false;
```


Tabla con la tasa de error y el *score* TOTAL:

| Métrica | Resultado |
| :--- | :--- |
| Unvoiced frames as voiced | 5/113 (4.42 %) |
| Voiced frames as unvoiced | 4/87 (4.60 %) |
| Gross voiced errors (+20.00 %) | 0/83 (0.00 %) |
| MSE of fine errors | 2.54 % |
| **TOTAL SCORE** | **93.00 %** |

### Parámetros finales utilizados:
* **Umbral de potencia (`-p`):** -52 dB
* **Umbral de rmaxnorm (`-M`):** 0.6
* **Umbral de r1norm (`-1`):** 0.6
* **Ventana:** Hamming



Ejercicios de ampliación
------------------------

- Usando la librería `docopt_cpp`, modifique el fichero `get_pitch.cpp` para incorporar los parámetros del
  estimador a los argumentos de la línea de comandos.
  
  Esta técnica le resultará especialmente útil para optimizar los parámetros del estimador. Recuerde que
  una parte importante de la evaluación recaerá en el resultado obtenido en la estimación de pitch en la
  base de datos.

  * Inserte un *pantallazo* en el que se vea el mensaje de ayuda del programa y un ejemplo de utilización
    con los argumentos añadidos.

- Implemente las técnicas que considere oportunas para optimizar las prestaciones del sistema de estimación
  de pitch.

  Entre las posibles mejoras, puede escoger una o más de las siguientes:

  * Técnicas de preprocesado: filtrado paso bajo, diezmado, *center clipping*, etc.
  * Técnicas de postprocesado: filtro de mediana, *dynamic time warping*, etc.
  * Métodos alternativos a la autocorrelación: procesado cepstral, *average magnitude difference function*
    (AMDF), etc.
  * Optimización **demostrable** de los parámetros que gobiernan el estimador, en concreto, de los que
    gobiernan la decisión sonoro/sordo.
  * Cualquier otra técnica que se le pueda ocurrir o encuentre en la literatura.

  Encontrará más información acerca de estas técnicas en las [Transparencias del Curso](https://atenea.upc.edu/pluginfile.php/2908770/mod_resource/content/3/2b_PS%20Techniques.pdf)
  y en [Spoken Language Processing](https://discovery.upc.edu/iii/encore/record/C__Rb1233593?lang=cat).
  También encontrará más información en los anexos del enunciado de esta práctica.

  Incluya, a continuación, una explicación de las técnicas incorporadas al estimador. Se valorará la
  inclusión de gráficas, tablas, código o cualquier otra cosa que ayude a comprender el trabajo realizado.

  También se valorará la realización de un estudio de los parámetros involucrados. Por ejemplo, si se opta
  por implementar el filtro de mediana, se valorará el análisis de los resultados obtenidos en función de
  la longitud del filtro.
   

Evaluación *ciega* del estimador
-------------------------------

Antes de realizar el *pull request* debe asegurarse de que su repositorio contiene los ficheros necesarios
para compilar los programas correctamente ejecutando `make release`.

Con los ejecutables construidos de esta manera, los profesores de la asignatura procederán a evaluar el
estimador con la parte de test de la base de datos (desconocida para los alumnos). Una parte importante de
la nota de la práctica recaerá en el resultado de esta evaluación.
