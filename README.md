## Projet de synthèse – M2 CHPS

### Objectif

Ce projet vise à étudier la propagation d’une maladie à l’aide de deux approches :

Un modèle épidémiologique SEIRS basé sur des équations différentielles (ODE)

Un modèle multi-agent (SMA)

L’objectif est de comparer les résultats, les performances et la consommation d’énergie selon les langages utilisés.

### Contenu du projet

Le projet est organisé en quatre parties principales :

🔹 Partie_1 : Modèle SEIRS (ODE)

Implémentation du modèle en C et en Python

Génération de fichiers CSV contenant les résultats

Analyse et visualisation avec un notebook Jupyter (analyse.ipynb)

🔹 Partie_2 : Modèle Multi-Agent

Implémentation du modèle en C, C++ et Python

Génération de fichiers CSV à partir de plusieurs réplications

Analyse statistique et visualisation avec un notebook Jupyter (analyse.ipynb)

🔹 Partie_Bonus : Performance et Énergie

Cette partie est consacrée à l’étude des performances et de la consommation énergétique.

Elle contient :

Mesure de performance

Scripts Bash pour lancer les simulations

Script Python analyse_performance.py

Analyse des fichiers CSV générés

Mesure d’énergie

Scripts Bash pour les mesures avec PowerJoular

Script Python energie_analyse.py

Analyse des résultats

Deux sous-dossiers sont présents :

performance_results : résultats de performance

energy_results : résultats de consommation d’énergie

Ces dossiers contiennent les fichiers CSV et TXT générés lors des mesures.

🔹 Rapport

Rapport final du projet

Présentation des méthodes, résultats et analyses

Discussion et conclusion

### Reproductibilité

Pour assurer la reproductibilité des résultats, un environnement virtuel Python a été créé pour chaque partie du projet.

Chaque dossier contient un fichier requirements.txt permettant d’installer automatiquement les dépendances.

Pour recréer un environnement :

python3 -m venv env
source env/bin/activate
pip install -r requirements.txt

### Auteur

Omer Boueke
Master 2 CHPS

### Encadrant

Benjamin Antunes

### Technologies utilisées

Langages : Python, C, C++

Analyse : NumPy, Pandas, Matplotlib, SciPy

Outils : Jupyter Notebook, Git, Bash

Mesure énergie : PowerJoular
