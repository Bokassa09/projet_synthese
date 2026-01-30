import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob
import os


def calculate_total_energy(filename):
    try:
        df = pd.read_csv(filename)
        
        # Vérification des colonnes de puissance (PowerJoular)
        if 'CPU Power' in df.columns:
            power_column = 'CPU Power'
        elif 'Total Power' in df.columns:
            power_column = 'Total Power'
        else:
            print(f"Colonne de puissance non trouvée dans {filename}")
            return None, None, None
        
        # Puissance moyenne en Watts
        avg_power_w = df[power_column].mean()
        
        # PowerJoular mesure généralement toutes les 500ms (0.5s)
        duration_s = len(df) * 0.5
        
        # Énergie = Puissance (W) × Temps (s) = Joules
        total_energy_j = avg_power_w * duration_s
        
        return total_energy_j, avg_power_w, duration_s
        
    except Exception as e:
        print(f"Erreur pour {filename}: {e}")
        return None, None, None


data = []
languages = ['c', 'cpp', 'python']  # Liste pour les dossiers
lang_display_names = {'c': 'C', 'cpp': 'C++', 'python': 'Python'}

print("Chargement des données énergétiques...")

for lang in languages:
    # Cherche les fichiers dans les sous-dossiers respectifs
    files = sorted(glob.glob(f'energy_results/{lang}/energy_run_*.csv'))
    
    if not files:
        print(f"Attention: Aucun fichier trouvé pour {lang}")
        continue

    for i, file in enumerate(files, 1):
        energy_j, avg_power_w, duration_s = calculate_total_energy(file)
        if energy_j is not None:
            data.append({
                'Language': lang_display_names[lang],
                'Run': i,
                'Energy_J': energy_j,
                'Avg_Power_W': avg_power_w,
                'Duration_s': duration_s
            })

df = pd.DataFrame(data)

# Sauvegarde des données propres
os.makedirs('energy_results', exist_ok=True)
df.to_csv('energy_results/energy_data_clean.csv', index=False)


print("\n" + "="*60)
print("RÉSUMÉ CONSOMMATION ÉNERGÉTIQUE (Moyenne ± STD)")
print("="*60)

# Ordre d'affichage souhaité
ordre_final = ['C', 'C++', 'Python']
couleurs = ['#555555', '#00599C', '#3776ab'] # Gris, Bleu C++, Bleu Python

# On prépare les stats en forçant l'ordre
stats_energy = df.groupby('Language').agg({
    'Energy_J': ['mean', 'std'],
    'Avg_Power_W': ['mean', 'std']
}).reindex(ordre_final)

for lang in ordre_final:
    row = stats_energy.loc[lang]
    print(f"{lang}:")
    print(f"  Énergie : {row[('Energy_J', 'mean')]:.2f} ± {row[('Energy_J', 'std')]:.2f} Joules")
    print(f"  Puissance moy : {row[('Avg_Power_W', 'mean')]:.2f} ± {row[('Avg_Power_W', 'std')]:.2f} Watts")
    print()


fig, axes = plt.subplots(1, 2, figsize=(16, 7))

# Graphique 1: Énergie Totale (J)
bars_e = axes[0].bar(ordre_final, stats_energy[('Energy_J', 'mean')], 
                     yerr=stats_energy[('Energy_J', 'std')],
                     capsize=6, alpha=0.85, color=couleurs)
axes[0].set_ylabel('Énergie consommée (Joules)', fontweight='bold')
axes[0].set_title('Énergie Totale consommée (Moyenne)', fontsize=13)
axes[0].grid(True, alpha=0.3, axis='y', linestyle='--')

# Labels au-dessus des barres (Énergie)
for bar in bars_e:
    height = bar.get_height()
    axes[0].text(bar.get_x() + bar.get_width()/2., height + 10,
                f'{height:.1f} J', ha='center', va='bottom', fontweight='bold')

# Graphique 2: Puissance Moyenne (W)
bars_p = axes[1].bar(ordre_final, stats_energy[('Avg_Power_W', 'mean')], 
                     yerr=stats_energy[('Avg_Power_W', 'std')],
                     capsize=6, alpha=0.85, color=couleurs)
axes[1].set_ylabel('Puissance moyenne (Watts)', fontweight='bold')
axes[1].set_title('Puissance moyenne du CPU pendant l\'exécution', fontsize=13)
axes[1].grid(True, alpha=0.3, axis='y', linestyle='--')
axes[1].set_ylim(0, max(stats_energy[('Avg_Power_W', 'mean')]) * 1.3)

# Labels au-dessus des barres (Puissance)
for bar in bars_p:
    height = bar.get_height()
    axes[1].text(bar.get_x() + bar.get_width()/2., height + 0.5,
                f'{height:.2f} W', ha='center', va='bottom', fontweight='bold')

plt.tight_layout()
plt.savefig('energy_results/energy_comparison_fixed.png', dpi=300)
plt.show()

print("="*60)
print("Graphique sauvegardé avec l'ordre correct : energy_results/energy_comparison.png")
print("="*60)