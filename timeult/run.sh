#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

fname="${HOSTNAME%.*}_$1_$(date +%s).txt"
allCounts=(1024 2048 4096 8192 16384)

echo "redirecting stdout to $fname"

pgm="./tester"
if command -v srun &> /dev/null; then
    runner="srun --mpi=pmi2 -n 1"
else
    runner="./charmrun ++local +p1"
fi

for count in ${allCounts[@]}; do
    cmd="$runner $pgm $count"
    printf "\n> $cmd\n" | tee -a $fname
    eval $cmd | tee -a $fname
done
