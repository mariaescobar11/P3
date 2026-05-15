#!/bin/bash

best_score=0
best_params=""

echo "Iniciant la cerca FINA de paràmetres (Fine Grid Search)..."
echo "Això pot trigar una mica. Si us plau, espera."
echo "=========================================================="

# Llistes de valors molt acotats al voltant dels millors resultats (91.26%)
Z_VALS="0.08 0.10 0.12"
P_VALS="-51 -52 -53"
R1_VALS="0.23 0.25 0.27"
RM_VALS="0.23 0.25 0.27"

# Assegurem-nos que l'executable està compilat i al dia
make > /dev/null

for z in $Z_VALS; do
  for p in $P_VALS; do
    for r1 in $R1_VALS; do
      for rm in $RM_VALS; do
        
        # Executem el test amagant la sortida general
        output=$(./scripts/run_get_pitch.sh -c -z $z --pot=$p -1 $r1 -M $rm 2>&1)
        
        # Extraiem el número de la línia "===>    TOTAL:  XX.XX %"
        score=$(echo "$output" | grep "TOTAL:" | awk '{print $3}')
        
        # Si hi ha hagut algun error i no troba l'score, el posem a 0
        if [ -z "$score" ]; then
            score=0
        fi

        # Mostrem el progrés per pantalla
        echo "Provant: -z $z --pot=$p -1 $r1 -M $rm -> Score: $score%"
        
        # Comparem per si és el nou millor usant awk (per suportar decimals)
        is_better=$(echo "$score $best_score" | awk '{if ($1 > $2) print 1; else print 0}')
        if [ "$is_better" -eq 1 ]; then
          best_score=$score
          best_params="-z $z --pot=$p -1 $r1 -M $rm"
        fi
        
      done
    done
  done
done

echo ""
echo "================================================="
echo "LA MILLOR COMBINACIÓ TROBADA ÉS:"
echo "$best_params amb un encert de $best_score%"
echo "================================================="
