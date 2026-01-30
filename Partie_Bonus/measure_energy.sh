#!/bin/bash
# Script de mesure de consommation énergétique avec PowerJoular
# Pour les 3 langages : Python, C, C++

echo "=========================================="
echo "MESURE CONSOMMATION ÉNERGÉTIQUE"
echo "PowerJoular - 5 essais par langage"
echo "=========================================="
echo ""

# Configuration
NUM_RUNS=5
RESULTS_DIR="energy_results"

# Créer les dossiers
mkdir -p $RESULTS_DIR/python
mkdir -p $RESULTS_DIR/c
mkdir -p $RESULTS_DIR/cpp

# Demander sudo une seule fois
sudo -v

# Garder sudo actif
(while true; do sudo -n true; sleep 50; done) 2>/dev/null &
SUDO_KEEPALIVE_PID=$!

# ==========================================
# PYTHON
# ==========================================
echo ">>> Mesure Python (5 essais)..."

for run in $(seq 1 $NUM_RUNS); do
    echo "  Essai $run/$NUM_RUNS"
    
    ENERGY_FILE="$RESULTS_DIR/python/energy_run_${run}.csv"
    
    # Lancer Python en arrière-plan
    python3 modele_python.py > /dev/null 2>&1 &
    PID=$!
    
    # Attendre le démarrage
    sleep 1
    
    # Lancer PowerJoular
    if kill -0 $PID 2>/dev/null; then
        sudo powerjoular -p $PID -f $ENERGY_FILE -t 500 &>/dev/null &
        JOULAR_PID=$!
        
        # Attendre la fin du programme
        wait $PID 2>/dev/null
        
        # Arrêter PowerJoular
        sudo kill -SIGINT $JOULAR_PID &>/dev/null
        wait $JOULAR_PID &>/dev/null
    fi
    
    sleep 2
done

echo "Python terminé !"
echo ""

# ==========================================
# C
# ==========================================
echo ">>> Mesure C (5 essais)..."

for run in $(seq 1 $NUM_RUNS); do
    echo "  Essai $run/$NUM_RUNS"
    
    ENERGY_FILE="$RESULTS_DIR/c/energy_run_${run}.csv"
    
    # Lancer C en arrière-plan
    ./model_c_perf > /dev/null 2>&1 &
    PID=$!
    
    # Attendre le démarrage
    sleep 1
    
    # Lancer PowerJoular
    if kill -0 $PID 2>/dev/null; then
        sudo powerjoular -p $PID -f $ENERGY_FILE -t 500 &>/dev/null &
        JOULAR_PID=$!
        
        # Attendre la fin du programme
        wait $PID 2>/dev/null
        
        # Arrêter PowerJoular
        sudo kill -SIGINT $JOULAR_PID &>/dev/null
        wait $JOULAR_PID &>/dev/null
    fi
    
    sleep 2
done

echo "C terminé !"
echo ""

# ==========================================
# C++
# ==========================================
echo ">>> Mesure C++ (5 essais)..."

for run in $(seq 1 $NUM_RUNS); do
    echo "  Essai $run/$NUM_RUNS"
    
    ENERGY_FILE="$RESULTS_DIR/cpp/energy_run_${run}.csv"
    
    # Lancer C++ en arrière-plan
    ./model_cpp_perf > /dev/null 2>&1 &
    PID=$!
    
    # Attendre le démarrage
    sleep 1
    
    # Lancer PowerJoular
    if kill -0 $PID 2>/dev/null; then
        sudo powerjoular -p $PID -f $ENERGY_FILE -t 500 &>/dev/null &
        JOULAR_PID=$!
        
        # Attendre la fin du programme
        wait $PID 2>/dev/null
        
        # Arrêter PowerJoular
        sudo kill -SIGINT $JOULAR_PID &>/dev/null
        wait $JOULAR_PID &>/dev/null
    fi
    
    sleep 2
done

echo "C++ terminé !"
echo ""

# Arrêter le keepalive sudo
kill $SUDO_KEEPALIVE_PID 2>/dev/null

echo "=========================================="
echo "Mesures terminées !"
echo "Fichiers dans energy_results/"
echo "=========================================="