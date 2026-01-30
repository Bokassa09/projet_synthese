#!/bin/bash
# Script simple de mesure de performance

echo "=========================================="
echo "MESURE DE PERFORMANCE - 5 essais"
echo "=========================================="

mkdir -p performance_results

N_RUNS=5

# Python
echo ">>> Python..."
for i in $(seq 1 $N_RUNS); do
    echo "  Essai $i/$N_RUNS"
    /usr/bin/time -v python3 modele_python.py 2> performance_results/python_$i.txt > /dev/null
done

# C
echo ">>> C..."
for i in $(seq 1 $N_RUNS); do
    echo "  Essai $i/$N_RUNS"
    /usr/bin/time -v ./model_c_perf 2> performance_results/c_$i.txt > /dev/null
done

# C++
echo ">>> C++..."
for i in $(seq 1 $N_RUNS); do
    echo "  Essai $i/$N_RUNS"
    /usr/bin/time -v ./model_cpp_perf 2> performance_results/cpp_$i.txt > /dev/null
done

echo ""
echo "Mesures terminées ! Fichiers dans performance_results/"