#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <sys/stat.h>

// Constantes
const int N_AGENTS = 20000;
const int GRID_SIZE = 300;
const int N_DAYS = 730;
//const int N_REPLICATIONS = 30;

//Comme je voulais faire la mesure de performance 
const int N_REPLICATIONS = 4;

// Statuts
enum Status { S = 0, E = 1, I = 2, R = 3 };

// Structure Agent
struct Agent {
    Status status;
    int time_in_status;
    double dE;
    double dI;
    double dR;
    int x;
    int y;
    
    Agent(Status s, double de, double di, double dr, int px, int py)
        : status(s), time_in_status(0), dE(de), dI(di), dR(dr), x(px), y(py) {}
};

// Classe pour le modele SEIRS
class SEIRSModel {
private:
    std::vector<Agent> agents;
    std::mt19937 rng;
    std::uniform_real_distribution<double> uniform_dist;
    std::uniform_int_distribution<int> grid_dist;
    
    // Distribution exponentielle negative
    double neg_exp(double mean) {
        return -mean * std::log(1.0 - uniform_dist(rng));
    }
    
    // Initialisation des agents
    void initialize_agents() {
        agents.clear();
        
        for (int i = 0; i < N_AGENTS; i++) {
            Status status = (i < 20) ? I : S;
            double dE = neg_exp(3.0);
            double dI = neg_exp(7.0);
            double dR = neg_exp(365.0);
            int x = grid_dist(rng);
            int y = grid_dist(rng);
            
            agents.emplace_back(status, dE, dI, dR, x, y);
        }
    }
    
    // Construit la grille spatiale
    std::vector<std::vector<int>> build_grid() {
        std::vector<std::vector<int>> grid(GRID_SIZE * GRID_SIZE);
        
        for (int idx = 0; idx < N_AGENTS; idx++) {
            int cell_idx = agents[idx].y * GRID_SIZE + agents[idx].x;
            grid[cell_idx].push_back(idx);
        }
        
        return grid;
    }
    
    // Compte les voisins infectes
    int count_infected_neighbors(int agent_idx, const std::vector<std::vector<int>>& grid) {
        const Agent& agent = agents[agent_idx];
        int count = 0;
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = (agent.x + dx + GRID_SIZE) % GRID_SIZE;
                int ny = (agent.y + dy + GRID_SIZE) % GRID_SIZE;
                int cell_idx = ny * GRID_SIZE + nx;
                
                for (int other_idx : grid[cell_idx]) {
                    if (other_idx != agent_idx && agents[other_idx].status == I) {
                        count++;
                    }
                }
            }
        }
        
        return count;
    }
    
    // Deplace un agent
    void move_agent(Agent& agent) {
        agent.x = grid_dist(rng);
        agent.y = grid_dist(rng);
    }
    
    // Met a jour un agent
    void update_agent(int agent_idx, const std::vector<std::vector<int>>& grid) {
        Agent& agent = agents[agent_idx];
        agent.time_in_status++;
        
        if (agent.status == S) {
            int Ni = count_infected_neighbors(agent_idx, grid);
            if (Ni > 0) {
                double p = 1.0 - std::exp(-0.5 * Ni);
                if (uniform_dist(rng) < p) {
                    agent.status = E;
                    agent.time_in_status = 0;
                }
            }
        } else if (agent.status == E) {
            if (agent.time_in_status >= agent.dE) {
                agent.status = I;
                agent.time_in_status = 0;
            }
        } else if (agent.status == I) {
            if (agent.time_in_status >= agent.dI) {
                agent.status = R;
                agent.time_in_status = 0;
            }
        } else if (agent.status == R) {
            if (agent.time_in_status >= agent.dR) {
                agent.status = S;
                agent.time_in_status = 0;
            }
        }
    }
    
    // Compte les statuts
    std::vector<int> count_status() {
        std::vector<int> counts(4, 0);
        for (const auto& agent : agents) {
            counts[agent.status]++;
        }
        return counts;
    }
    
public:
    SEIRSModel(int seed) 
        : rng(seed), uniform_dist(0.0, 1.0), grid_dist(0, GRID_SIZE - 1) {}
    
    // Execute une simulation
    void run_simulation(int rep_num) {
        initialize_agents();
        
        std::cout << "Simulation avec seed=" << rng() << std::endl;
        
        // Creer le dossier results_cpp
        #ifdef _WIN32
            mkdir("results_cpp");
        #else
            mkdir("results_cpp", 0777);
        #endif
        
        // Ouvrir le fichier CSV
        std::string filename = "results_cpp/replication_" + std::to_string(rep_num) + ".csv";
        std::ofstream file(filename);
        file << "Day,S,E,I,R\n";
        
        for (int day = 0; day < N_DAYS; day++) {
            // Deplacement
            for (auto& agent : agents) {
                move_agent(agent);
            }
            
            // Construire la grille
            auto grid = build_grid();
            
            // Ordre aleatoire
            std::vector<int> indices(N_AGENTS);
            for (int i = 0; i < N_AGENTS; i++) {
                indices[i] = i;
            }
            std::shuffle(indices.begin(), indices.end(), rng);
            
            // Mise a jour
            for (int idx : indices) {
                update_agent(idx, grid);
            }
            
            // Comptage
            auto counts = count_status();
            file << day << "," << counts[S] << "," << counts[E] << "," 
                 << counts[I] << "," << counts[R] << "\n";
            
            if ((day + 1) % 100 == 0) {
                std::cout << "  Jour " << day + 1 << ": S=" << counts[S] 
                          << ", E=" << counts[E] << ", I=" << counts[I] 
                          << ", R=" << counts[R] << std::endl;
            }
        }
        
        file.close();
        std::cout << "Resultats sauvegardes dans " << filename << std::endl;
    }
};

int main() {
    for (int rep = 0; rep < N_REPLICATIONS; rep++) {
        std::cout << "\n=== Replication " << rep + 1 << "/" << N_REPLICATIONS << " ===" << std::endl;
        
        SEIRSModel model(1000 + rep);
        model.run_simulation(rep + 1);
    }
    
    std::cout << "\n=== Toutes les replications terminees ===" << std::endl;
    return 0;
}