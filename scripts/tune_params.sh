#!/bin/bash

# Barrido parametric intel·ligent per trobar els millors llindars.
# Cerca per coordenades: optimitza un parametre cada vegada.

set -o pipefail

GETF0="get_pitch"
EVAL="pitch_evaluate"

evaluate() {
    local pot=$1 r1=$2 rm=$3 zcr=$4
    rm -f /tmp/__f0_*.f0 2>/dev/null
    for fwav in pitch_db/train/*.wav; do
        ff0="/tmp/__f0_$(basename ${fwav/.wav/.f0})"
        $GETF0 -p $pot -1 $r1 -M $rm -z $zcr $fwav $ff0 > /dev/null 2>&1
    done
    $EVAL /tmp/__f0_*.f0 2>/dev/null | grep "TOTAL" | awk '{print $2}' | sed 's/%//'
}

best_pot=-49
best_r1=0.38
best_rm=0.38
best_zcr=0.25
best_total=$(evaluate $best_pot $best_r1 $best_rm $best_zcr)
echo "INICI: TOTAL=$best_total %  (pot=$best_pot r1=$best_r1 rM=$best_rm zcr=$best_zcr)"

for round in 1 2; do
    echo ""
    echo "=== RONDA $round ==="

    # 1. Optimitzar pot
    echo "--- Optimitzant pot ---"
    for pot in -60 -57 -54 -51 -48 -45 -42 -39 -36 -33 -30 -27 -24 -21 -18; do
        total=$(evaluate $pot $best_r1 $best_rm $best_zcr)
        echo "  pot=$pot -> TOTAL=$total %"
        if (( $(echo "$total > $best_total" | bc -l) )); then
            best_total=$total; best_pot=$pot
            echo "    *** NOU MILLOR ***"
        fi
    done
    echo "  Millor pot=$best_pot (TOTAL=$best_total %)"

    # 2. Optimitzar r1norm
    echo "--- Optimitzant r1norm ---"
    for r1 in 0.20 0.24 0.28 0.32 0.36 0.40 0.44 0.48 0.52 0.56 0.60; do
        total=$(evaluate $best_pot $r1 $best_rm $best_zcr)
        echo "  r1=$r1 -> TOTAL=$total %"
        if (( $(echo "$total > $best_total" | bc -l) )); then
            best_total=$total; best_r1=$r1
            echo "    *** NOU MILLOR ***"
        fi
    done
    echo "  Millor r1=$best_r1 (TOTAL=$best_total %)"

    # 3. Optimitzar rmaxnorm
    echo "--- Optimitzant rmaxnorm ---"
    for rm in 0.20 0.24 0.28 0.32 0.36 0.40 0.44 0.48 0.52 0.56 0.60; do
        total=$(evaluate $best_pot $best_r1 $rm $best_zcr)
        echo "  rM=$rm -> TOTAL=$total %"
        if (( $(echo "$total > $best_total" | bc -l) )); then
            best_total=$total; best_rm=$rm
            echo "    *** NOU MILLOR ***"
        fi
    done
    echo "  Millor rM=$best_rm (TOTAL=$best_total %)"

    # 4. Optimitzar zcr
    echo "--- Optimitzant ZCR ---"
    for zcr in 0.05 0.10 0.15 0.20 0.25 0.30 0.35 0.40 0.50 0.60 0.80 1.0; do
        total=$(evaluate $best_pot $best_r1 $best_rm $zcr)
        echo "  zcr=$zcr -> TOTAL=$total %"
        if (( $(echo "$total > $best_total" | bc -l) )); then
            best_total=$total; best_zcr=$zcr
            echo "    *** NOU MILLOR ***"
        fi
    done
    echo "  Millor zcr=$best_zcr (TOTAL=$best_total %)"

done

echo ""
echo "========================================"
echo "  RESULTAT FINAL"
echo "  TOTAL: $best_total %"
echo "  Parametres: -p $best_pot -1 $best_r1 -M $best_rm -z $best_zcr"
echo "========================================"
echo ""
echo "Per provar:"
echo "  ./scripts/run_get_pitch.sh -p $best_pot -1 $best_r1 -M $best_rm -z $best_zcr"
echo "  ./scripts/run_get_pitch.sh --ceps -p $best_pot -1 $best_r1 -M $best_rm -z $best_zcr"
