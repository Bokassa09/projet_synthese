import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os


def extract_perf(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    
    time_match = re.search(r'Elapsed \(wall clock\) time.*?: (?:(\d+):)?(\d+):(\d+\.\d+)', content)
    
    if time_match:
        
        h = int(time_match.group(1)) if time_match.group(1) else 0
        m = int(time_match.group(2))
        s = float(time_match.group(3)) # float() ici pour accepter les décimales
        total_seconds = h * 3600 + m * 60 + s
    else:
        
        total_seconds = None

    # Extraction de la mémoire
    mem_match = re.search(r'Maximum resident set size.*?: (\d+)', content)
    memory_kb = int(mem_match.group(1)) if mem_match else None
    
    return total_seconds, memory_kb


data = []
languages = ['python', 'c', 'cpp']
lang_names = {'python': 'Python', 'c': 'C', 'cpp': 'C++'}

for lang in languages:
    for i in range(1, 6):
        filename = f'performance_results/{lang}_{i}.txt'
        if os.path.exists(filename):
            try:
                time_s, mem_kb = extract_perf(filename)
                if time_s is not None:
                    data.append({
                        'Language': lang_names[lang],
                        'Run': i,
                        'Time_seconds': time_s,
                        'Memory_MB': mem_kb / 1024 if mem_kb else None
                    })
            except Exception as e:
                print(f"Erreur sur {filename}: {e}")

if not data:
    print("ERREUR : Aucune donnée n'a pu être extraite. Vérifiez le format de vos fichiers .txt")
else:
    df = pd.DataFrame(data)

    
    print("="*60)
    print("RÉSUMÉ DES PERFORMANCES (Moyenne ± Écart-type)")
    print("="*60)

    ordre_langages = ['C', 'C++', 'Python']
    for lang in ordre_langages:
        lang_data = df[df['Language'] == lang]
        if not lang_data.empty:
            print(f"{lang}:")
            print(f"  Temps   : {lang_data['Time_seconds'].mean():.2f} ± {lang_data['Time_seconds'].std():.2f} s")
            print(f"  Mémoire : {lang_data['Memory_MB'].mean():.2f} ± {lang_data['Memory_MB'].std():.2f} MB")
            print()

   
    stats_df = df.groupby('Language').agg({
        'Time_seconds': ['mean', 'std'],
        'Memory_MB': ['mean', 'std']
    }).reindex(ordre_langages)

    fig, axes = plt.subplots(1, 2, figsize=(15, 6))
    couleurs = ['#555555', '#00599C', '#3776ab']

    # Temps d'exécution
    axes[0].bar(ordre_langages, stats_df['Time_seconds']['mean'], 
                yerr=stats_df['Time_seconds']['std'], 
                capsize=6, alpha=0.8, color=couleurs)
    axes[0].set_ylabel('Temps (secondes)')
    axes[0].set_title('Temps d\'exécution total')

    # Mémoire
    axes[1].bar(ordre_langages, stats_df['Memory_MB']['mean'], 
                yerr=stats_df['Memory_MB']['std'],
                capsize=6, alpha=0.8, color=couleurs)
    axes[1].set_ylabel('Mémoire (MB)')
    axes[1].set_title('Consommation Mémoire Maximale')

    plt.tight_layout()
    plt.savefig('performance_results/performance_comparison.png', dpi=300)
    plt.show()