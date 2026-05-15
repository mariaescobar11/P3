#!/bin/bash

# Aquest script compara autocorrelació vs cepstrum
# per ajudar-te a veure quin dona millor resultat.

echo "Comparant autocorrelació vs cepstrum..."
echo "======================================="

echo ""
echo "--- Sense --cpeak (autocorrelació) ---"
./scripts/run_get_pitch.sh | grep "TOTAL"

echo ""
echo "--- Amb --cpeak (cepstrum) ---"
./scripts/run_get_pitch.sh --cpeak 0.5 | grep "TOTAL"

echo ""
echo "Comparació finalitzada!"
