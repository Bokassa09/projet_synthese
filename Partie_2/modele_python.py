import random
import math
import csv
import os

# Constantes
N_AGENTS = 20000
GRID_SIZE = 300
N_DAYS = 730
#N_REPLICATIONS = 30
N_REPLICATIONS = 4 # Comme je voulais faire la mesure de performance 

# Statuts possibles
S, E, I, R = 0, 1, 2, 3

class Agent:
    def __init__(self, status, dE, dI, dR, x, y):
        self.status = status
        self.time_in_status = 0
        self.dE = dE
        self.dI = dI
        self.dR = dR
        self.x = x
        self.y = y

def neg_exp(mean):
    # Distribution exponentielle negative
    return -mean * math.log(1.0 - random.random())

def initialize_agents():
    # Cree 20 agents I et 19980 agents S
    agents = []
    
    for i in range(N_AGENTS):
        status = I if i < 20 else S
        
        dE = neg_exp(3.0)
        dI = neg_exp(7.0)
        dR = neg_exp(365.0)
        
        x = random.randint(0, GRID_SIZE - 1)
        y = random.randint(0, GRID_SIZE - 1)
        
        agents.append(Agent(status, dE, dI, dR, x, y))
    
    return agents

def build_grid(agents):
    # Construit une grille pour accelerer la recherche de voisins
    grid = {}
    for idx, agent in enumerate(agents):
        key = (agent.x, agent.y)
        if key not in grid:
            grid[key] = []
        grid[key].append(idx)
    return grid

def count_infected_neighbors(agents, agent_idx, grid):
    # Compte les agents I dans le voisinage de Moore
    agent = agents[agent_idx]
    count = 0
    
    # Parcourir seulement le voisinage de Moore
    for dx in [-1, 0, 1]:
        for dy in [-1, 0, 1]:
            nx = (agent.x + dx) % GRID_SIZE
            ny = (agent.y + dy) % GRID_SIZE
            
            key = (nx, ny)
            if key in grid:
                for other_idx in grid[key]:
                    if other_idx != agent_idx and agents[other_idx].status == I:
                        count += 1
    
    return count

def move_agent(agent):
    # Deplacement aleatoire sur la grille
    agent.x = random.randint(0, GRID_SIZE - 1)
    agent.y = random.randint(0, GRID_SIZE - 1)

def update_agent(agents, agent_idx, grid):
    # Met a jour le statut d'un agent
    agent = agents[agent_idx]
    agent.time_in_status += 1
    
    if agent.status == S:
        Ni = count_infected_neighbors(agents, agent_idx, grid)
        if Ni > 0:
            p = 1.0 - math.exp(-0.5 * Ni)
            if random.random() < p:
                agent.status = E
                agent.time_in_status = 0
    
    elif agent.status == E:
        if agent.time_in_status >= agent.dE:
            agent.status = I
            agent.time_in_status = 0
    
    elif agent.status == I:
        if agent.time_in_status >= agent.dI:
            agent.status = R
            agent.time_in_status = 0
    
    elif agent.status == R:
        if agent.time_in_status >= agent.dR:
            agent.status = S
            agent.time_in_status = 0

def count_status(agents):
    # Compte le nombre d'agents dans chaque statut
    counts = [0, 0, 0, 0]
    for agent in agents:
        counts[agent.status] += 1
    return counts

def run_simulation(seed):
    # Execute une simulation complete
    random.seed(seed)
    
    agents = initialize_agents()
    results = []
    
    print(f"Simulation avec seed={seed}")
    
    for day in range(N_DAYS):
        # Deplacement de tous les agents
        for agent in agents:
            move_agent(agent)
        
        # Construire la grille apres deplacement
        grid = build_grid(agents)
        
        # Mise a jour dans un ordre aleatoire
        indices = list(range(N_AGENTS))
        random.shuffle(indices)
        
        for idx in indices:
            update_agent(agents, idx, grid)
        
        # Enregistrement des comptages
        counts = count_status(agents)
        results.append([day, counts[S], counts[E], counts[I], counts[R]])
        
        if (day + 1) % 100 == 0:
            print(f"  Jour {day + 1}: S={counts[S]}, E={counts[E]}, I={counts[I]}, R={counts[R]}")
    
    return results

def save_results(results, filename):
    # Sauvegarde les resultats dans un fichier CSV
    os.makedirs('results_python', exist_ok=True)
    filepath = os.path.join('results_python', filename)
    
    with open(filepath, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['Day', 'S', 'E', 'I', 'R'])
        writer.writerows(results)
    
    print(f"Resultats sauvegardes dans {filepath}")

def main():
    # Execute les 30 replications
    for rep in range(N_REPLICATIONS):
        print(f"\n=== Replication {rep + 1}/{N_REPLICATIONS} ===")
        
        seed = 1000 + rep
        results = run_simulation(seed)
        
        filename = f"replication_{rep + 1}.csv"
        save_results(results, filename)
    
    print("\n=== Toutes les replications terminees ===")

if __name__ == "__main__":
    main()