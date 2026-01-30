#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

// Constantes
#define N_AGENTS 20000
#define GRID_SIZE 300
#define N_DAYS 730
//#define N_REPLICATIONS 30

//Comme je voulais faire la mesure de performance 
#define N_REPLICATIONS 4

// Statuts
#define S 0
#define E 1
#define I 2
#define R 3

// Structure Agent
typedef struct {
    int status;
    int time_in_status;
    double dE;
    double dI;
    double dR;
    int x;
    int y;
} Agent;

// Structure pour la grille
typedef struct {
    int* indices;
    int count;
    int capacity;
} GridCell;

// Variables globales pour MT19937
#define MT_N 624
#define MT_M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

static unsigned long mt[MT_N];
static int mti = MT_N + 1;

// Initialisation MT19937
void init_genrand(unsigned long s) {
    mt[0] = s & 0xffffffffUL;
    for (mti = 1; mti < MT_N; mti++) {
        mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        mt[mti] &= 0xffffffffUL;
    }
}

// Generation nombre aleatoire MT19937
unsigned long genrand_int32(void) {
    unsigned long y;
    static unsigned long mag01[2] = {0x0UL, MATRIX_A};
    
    if (mti >= MT_N) {
        int kk;
        for (kk = 0; kk < MT_N - MT_M; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < MT_N - 1; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+(MT_M-MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[MT_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[MT_N-1] = mt[MT_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mti = 0;
    }
    
    y = mt[mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    
    return y;
}

// Genere un double entre 0 et 1
double genrand_real(void) {
    return genrand_int32() * (1.0 / 4294967296.0);
}

// Distribution exponentielle negative
double neg_exp(double mean) {
    return -mean * log(1.0 - genrand_real());
}

// Initialisation des agents
void initialize_agents(Agent* agents) {
    for (int i = 0; i < N_AGENTS; i++) {
        if (i < 20) {
            agents[i].status = I;
        } else {
            agents[i].status = S;
        }
        
        agents[i].time_in_status = 0;
        agents[i].dE = neg_exp(3.0);
        agents[i].dI = neg_exp(7.0);
        agents[i].dR = neg_exp(365.0);
        agents[i].x = genrand_int32() % GRID_SIZE;
        agents[i].y = genrand_int32() % GRID_SIZE;
    }
}

// Initialisation de la grille
GridCell* create_grid(void) {
    GridCell* grid = (GridCell*)malloc(GRID_SIZE * GRID_SIZE * sizeof(GridCell));
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i].indices = NULL;
        grid[i].count = 0;
        grid[i].capacity = 0;
    }
    return grid;
}

// Liberer la grille
void free_grid(GridCell* grid) {
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        if (grid[i].indices != NULL) {
            free(grid[i].indices);
        }
    }
    free(grid);
}

// Construire la grille
void build_grid(Agent* agents, GridCell* grid) {
    // Reinitialiser les compteurs
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i].count = 0;
    }
    
    // Ajouter les agents
    for (int idx = 0; idx < N_AGENTS; idx++) {
        int cell_idx = agents[idx].y * GRID_SIZE + agents[idx].x;
        
        if (grid[cell_idx].count >= grid[cell_idx].capacity) {
            grid[cell_idx].capacity = grid[cell_idx].capacity == 0 ? 10 : grid[cell_idx].capacity * 2;
            grid[cell_idx].indices = (int*)realloc(grid[cell_idx].indices, 
                                                   grid[cell_idx].capacity * sizeof(int));
        }
        
        grid[cell_idx].indices[grid[cell_idx].count++] = idx;
    }
}

// Compte les voisins infectes
int count_infected_neighbors(Agent* agents, int agent_idx, GridCell* grid) {
    Agent* agent = &agents[agent_idx];
    int count = 0;
    
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int nx = (agent->x + dx + GRID_SIZE) % GRID_SIZE;
            int ny = (agent->y + dy + GRID_SIZE) % GRID_SIZE;
            int cell_idx = ny * GRID_SIZE + nx;
            
            for (int i = 0; i < grid[cell_idx].count; i++) {
                int other_idx = grid[cell_idx].indices[i];
                if (other_idx != agent_idx && agents[other_idx].status == I) {
                    count++;
                }
            }
        }
    }
    
    return count;
}

// Deplace un agent
void move_agent(Agent* agent) {
    agent->x = genrand_int32() % GRID_SIZE;
    agent->y = genrand_int32() % GRID_SIZE;
}

// Met a jour un agent
void update_agent(Agent* agents, int agent_idx, GridCell* grid) {
    Agent* agent = &agents[agent_idx];
    agent->time_in_status++;
    
    if (agent->status == S) {
        int Ni = count_infected_neighbors(agents, agent_idx, grid);
        if (Ni > 0) {
            double p = 1.0 - exp(-0.5 * Ni);
            if (genrand_real() < p) {
                agent->status = E;
                agent->time_in_status = 0;
            }
        }
    } else if (agent->status == E) {
        if (agent->time_in_status >= agent->dE) {
            agent->status = I;
            agent->time_in_status = 0;
        }
    } else if (agent->status == I) {
        if (agent->time_in_status >= agent->dI) {
            agent->status = R;
            agent->time_in_status = 0;
        }
    } else if (agent->status == R) {
        if (agent->time_in_status >= agent->dR) {
            agent->status = S;
            agent->time_in_status = 0;
        }
    }
}

// Compte les statuts
void count_status(Agent* agents, int* counts) {
    counts[0] = counts[1] = counts[2] = counts[3] = 0;
    for (int i = 0; i < N_AGENTS; i++) {
        counts[agents[i].status]++;
    }
}

// Melange un tableau 
void shuffle(int* array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = genrand_int32() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// Execute une simulation
void run_simulation(int seed, int rep_num) {
    init_genrand(seed);
    
    Agent* agents = (Agent*)malloc(N_AGENTS * sizeof(Agent));
    GridCell* grid = create_grid();
    int* indices = (int*)malloc(N_AGENTS * sizeof(int));
    
    initialize_agents(agents);
    
    printf("Simulation avec seed=%d\n", seed);
    
    // Creer le dossier results_c
    #ifdef _WIN32
        mkdir("results_c");
    #else
        mkdir("results_c", 0777);
    #endif
    
    // Ouvrir le fichier CSV
    char filename[256];
    sprintf(filename, "results_c/replication_%d.csv", rep_num);
    FILE* file = fopen(filename, "w");
    fprintf(file, "Day,S,E,I,R\n");
    
    for (int day = 0; day < N_DAYS; day++) {
        // Deplacement
        for (int i = 0; i < N_AGENTS; i++) {
            move_agent(&agents[i]);
        }
        
        // Construire la grille
        build_grid(agents, grid);
        
        // Ordre aleatoire
        for (int i = 0; i < N_AGENTS; i++) {
            indices[i] = i;
        }
        shuffle(indices, N_AGENTS);
        
        // Mise a jour
        for (int i = 0; i < N_AGENTS; i++) {
            update_agent(agents, indices[i], grid);
        }
        
        // Comptage
        int counts[4];
        count_status(agents, counts);
        fprintf(file, "%d,%d,%d,%d,%d\n", day, counts[S], counts[E], counts[I], counts[R]);
        
        if ((day + 1) % 100 == 0) {
            printf("  Jour %d: S=%d, E=%d, I=%d, R=%d\n", day + 1, counts[S], counts[E], counts[I], counts[R]);
        }
    }
    
    fclose(file);
    printf("Resultats sauvegardes dans %s\n", filename);
    
    free(agents);
    free_grid(grid);
    free(indices);
}

int main(void) {
    for (int rep = 0; rep < N_REPLICATIONS; rep++) {
        printf("\n=== Replication %d/%d ===\n", rep + 1, N_REPLICATIONS);
        run_simulation(1000 + rep, rep + 1);
    }
    
    printf("\n=== Toutes les replications terminees ===\n");
    return 0;
}