#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

ts="$(date +%s)"
opts="-Ofast"

run_with_logging() {
    printf "\n> %s\n" "$1" | tee -a "$2"
    set +e
    eval "$1" 2>&1 | tee -a "$2"
    retval=$?
    set -e
}

for lib in "$CHARM_HOME"/lib/libthreads*.a; do
    lib="${lib##*/}"
    lib="${lib##*libthreads-}"
    lib="${lib%%.a}"

    echo "running with: $lib"

    fname="${HOSTNAME%.*}_${lib}_$ts.txt"
    counts=(1024 2048 4096 8192 16384)

    make clean
    cmd="OPTS=\"$opts -thread $lib\" make"
    run_with_logging "$cmd" "$fname"

    pgm="./tester"
    if command -v srun &> /dev/null; then
        runner="srun --mpi=pmi2 -n 1"
    else
        runner="./charmrun ++local +p1"
    fi

    for count in "${counts[@]}"; do
        cmd="$runner $pgm $count"
        run_with_logging "$cmd" "$fname"
        # early termination if we fail
        if [ "$retval" != "0" ]; then
            break
        fi
    done
done
