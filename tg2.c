#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Constraints and Parameters
#define SIZE 16
#define ROWS 4
#define COLS 4
#define MAX_POP 100 // Maximum population size limit

// GA Parameters from the image
const double Pc = 0.8; // Crossover Probability
const double Pm = 0.1; // Mutation Probability

void printBoard(char board[ROWS][COLS]) {
    printf("\nBoard (4x4):\n");
    printf("  0 1 2 3\n");
    for (int r = 0; r < ROWS; r++) {
        printf("%d ", r);
        for (int c = 0; c < COLS; c++) {
            printf("%c ", board[r][c]);
        }
        printf("\n");
    }
}

void readPosition(char piece, int number, char board[ROWS][COLS]) {
    int r, c;
    while (1) {
        printf("Enter row and col for %c%d (0-3 0-3): ", piece, number);
        scanf("%d %d", &r, &c);
        if (r < 0 || r > 3 || c < 0 || c > 3) {
            printf("Invalid position.\n");
            continue;
        }
        if (board[r][c] != 'E') {
            printf("Cell already used.\n");
            continue;
        }
        board[r][c] = piece;
        break;
    }
}

// Check if piece at index i attacks index j
int isAttacking(int i, char p1, int j) {
    int r1 = i / COLS;
    int c1 = i % COLS;
    int r2 = j / COLS;
    int c2 = j % COLS;

    if (p1 == 'Q') {
        if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2)) return 1;
    } else if (p1 == 'R') {
        if (r1 == r2 || c1 == c2) return 1;
    } else if (p1 == 'B') {
        if (abs(r1 - r2) == abs(c1 - c2)) return 1;
    } else if (p1 == 'K') {
        if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
            (abs(r1 - r2) == 1 && abs(c1 - c2) == 2)) return 1;
    }
    return 0;
}

// MODIFIED: Calculates Fitness based on Image (Eq 2) and User Request
// 1. Counts "Threatened Pieces" instead of just conflict pairs.
// 2. Adds Penalty for columns with > 1 Queen.
double fitness(char chrom[]) {
    int is_threatened[SIZE] = {0}; // Track which pieces are under attack
    int penalty = 0;

    // 1. Calculate Threatened Pieces
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'E') continue;
        
        for (int j = 0; j < SIZE; j++) {
            if (i == j || chrom[j] == 'E') continue;
            
            // Check if piece at i attacks piece at j
            if (isAttacking(i, chrom[i], j)) {
                is_threatened[j] = 1; // Mark j as threatened
            }
        }
    }

    int nb_threatened_pieces = 0;
    for (int i = 0; i < SIZE; i++) {
        nb_threatened_pieces += is_threatened[i];
    }

    // 2. Calculate Penalty (Eq 2 in Image)
    // Penalty = number of columns which contains more than 1 queen
    for (int c = 0; c < COLS; c++) {
        int queen_count_in_col = 0;
        for (int r = 0; r < ROWS; r++) {
            int idx = r * COLS + c;
            if (chrom[idx] == 'Q') {
                queen_count_in_col++;
            }
        }
        if (queen_count_in_col > 1) {
            penalty++;
        }
    }

    // Fitness Formula: F = 1 / (1 + nb_conflicts + penalty)
    // Here nb_conflicts is replaced by nb_threatened_pieces as requested
    return 1.0 / (1.0 + nb_threatened_pieces + penalty);
}

void shuffle(char chrom[]) {
    for (int i = SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temp = chrom[i];
        chrom[i] = chrom[j];
        chrom[j] = temp;
    }
}

void printArray(char arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%c", arr[i]);
        if (i != size - 1) printf(", ");
    }
    printf("]");
}

void printPopulation(char population[][SIZE], double fitnessScores[], int count, char* label) {
    printf("\n=== %s ===\n", label);
    for (int i = 0; i < count; i++) {
        printf("Chrom %d: ", i);
        printArray(population[i], SIZE);
        printf(" | Fit: %.4f\n", fitnessScores[i]);
    }
}

void copyArray(char dest[], char src[]) {
    for (int i = 0; i < SIZE; i++)
        dest[i] = src[i];
}

void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[][SIZE], double selectedFitness[], int popSize) 
{
    // Initialize temporary fitness array to mark used candidates (optional, but good for tracking)
    double tempFitnessScores[MAX_POP];
    for (int k = 0; k < popSize; k++) {
        tempFitnessScores[k] = fitnessScores[k];
    }
    
    // Select popSize/2 pairs (roughly) or just select enough parents for crossover
    // Here we select 'popSize' parents to fill the mating pool
    for (int s = 0; s < popSize; s++) {
        int a = rand() % popSize;
        int b = rand() % popSize;

        // Simple tournament
        int winner = (fitnessScores[a] > fitnessScores[b]) ? a : b;

        copyArray(selected[s], population[winner]);
        selectedFitness[s] = fitnessScores[winner];
    }
}

void crossover(char selected[][SIZE], double selectedFitness[],
               char offspring[][SIZE], double offspringFitness[], int popSize) 
{
    // Initialize offspring with parents
    for (int i = 0; i < popSize; i++) {
        copyArray(offspring[i], selected[i]);
        offspringFitness[i] = selectedFitness[i];
    }

    // Perform crossover in pairs
    for (int i = 0; i < popSize - 1; i += 2) {
        
        // MODIFIED: Added Crossover Probability check (Pc = 0.8)
        double r = (double)rand() / RAND_MAX;
        
        if (r < Pc) {
            // Perform Single Point Crossover (Split at 8)
            char child1[SIZE], child2[SIZE];
            
            for (int k = 0; k < 8; k++) {
                child1[k] = selected[i][k];
                child2[k] = selected[i+1][k];
            }
            for (int k = 8; k < SIZE; k++) {
                child1[k] = selected[i+1][k];
                child2[k] = selected[i][k];
            }
            
            copyArray(offspring[i], child1);
            offspringFitness[i] = fitness(child1);
            
            copyArray(offspring[i+1], child2);
            offspringFitness[i+1] = fitness(child2);
        }
        // If r > Pc, parents are kept as is (cloned to offspring)
    }
}

void mutation(char population[][SIZE], double fitnessScores[],
              int nQ, int nR, int nB, int nK, int popSize)
{
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};
    
    for (int c = 0; c < popSize; c++) {
        
        // MODIFIED: Added Mutation Probability check (Pm = 0.1)
        // Note: We perform random swaps if Pm is met. 
        // Then we ALWAYS perform the "repair" logic to ensure piece counts are valid.
        
        double r = (double)rand() / RAND_MAX;
        
        if (r < Pm) {
            // Perform a random swap (Mutation)
            int p1 = rand() % SIZE;
            int p2 = rand() % SIZE;
            char temp = population[c][p1];
            population[c][p1] = population[c][p2];
            population[c][p2] = temp;
        }

        // --- REPAIR / CONSTRAINT HANDLING ---
        // This part ensures the number of pieces remains correct (Q, R, B, K counts)
        // This is necessary because crossover might destroy the counts.
        
        for (int p = 0; p < 4; p++) {
            int count = 0;
            for (int i = 0; i < SIZE; i++)
                if (population[c][i] == pieces[p]) count++;

            // Add missing pieces
            while (count < targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == 'E') {
                    population[c][pos] = pieces[p];
                    count++;
                }
            }
            
            // Remove excess pieces
            while (count > targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == pieces[p]) {
                    population[c][pos] = 'E';
                    count--;
                }
            }
        }
        
        // Update fitness after mutation/repair
        fitnessScores[c] = fitness(population[c]);
    }
}

void replacement(char oldPopulation[][SIZE], double oldFitness[],
                 char newPopulation[][SIZE], double newFitness[],
                 char resultPopulation[][SIZE], double resultFitness[], int popSize) 
{
    // Elitism + Selection: Combine both populations and pick the best
    struct Individual {
        char chrom[SIZE];
        double fit;
    } all[MAX_POP * 2];
    
    int total = 0;
    for (int i = 0; i < popSize; i++) {
        copyArray(all[total].chrom, oldPopulation[i]);
        all[total].fit = oldFitness[i];
        total++;
    }
    for (int i = 0; i < popSize; i++) {
        copyArray(all[total].chrom, newPopulation[i]);
        all[total].fit = newFitness[i];
        total++;
    }
    
    // Sort descending (Bubble sort for simplicity)
    for (int i = 0; i < total - 1; i++) {
        for (int j = 0; j < total - i - 1; j++) {
            if (all[j].fit < all[j + 1].fit) {
                struct Individual temp = all[j];
                all[j] = all[j + 1];
                all[j + 1] = temp;
            }
        }
    }
    
    // Select top 'popSize'
    for (int i = 0; i < popSize; i++) {
        copyArray(resultPopulation[i], all[i].chrom);
        resultFitness[i] = all[i].fit;
    }
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], 
                   int nQ, int nR, int nB, int nK, int generations, int popSize) 
{
    printf("\n=== EVOLUTION START (Max Gen: %d, Pop: %d) ===\n", generations, popSize);
    printf("Probabilities: Pc = %.2f, Pm = %.2f\n", Pc, Pm);
    
    char selected[MAX_POP][SIZE];
    double selectedFitness[MAX_POP];
    char offspring[MAX_POP][SIZE];
    double offspringFitness[MAX_POP];
    char newPopulation[MAX_POP][SIZE];
    double newFitness[MAX_POP];
    
    for (int gen = 1; gen <= generations; gen++) {
        
        // 1. Selection
        tournamentSelection(population, fitnessScores, selected, selectedFitness, popSize);
        
        // 2. Crossover (With Pc check)
        crossover(selected, selectedFitness, offspring, offspringFitness, popSize);
        
        // 3. Mutation (With Pm check) & Repair
        mutation(offspring, offspringFitness, nQ, nR, nB, nK, popSize);
        
        // 4. Replacement (Elitism)
        replacement(population, fitnessScores, offspring, offspringFitness, 
                    newPopulation, newFitness, popSize);
        
        // Update main population
        double bestFit = 0.0;
        double avgFit = 0.0;
        
        for (int i = 0; i < popSize; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
            
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
        }
        avgFit /= popSize;
        
        printf("Generation %d: Best Fit = %.4f, Avg Fit = %.4f\n", gen, bestFit, avgFit);
        
        if (bestFit >= 0.9999) { // 1.0 roughly
            printf("\n*** OPTIMAL SOLUTION FOUND AT GEN %d ***\n", gen);
            break;
        }
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== CHESS PIECE PLACEMENT GENETIC ALGORITHM ===\n");
    
    char board[ROWS][COLS];
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    // Initial Board Setup
    printBoard(board);
    
    int nQ, nR, nB, nK;
    while (1) {
        printf("\nEnter number of Queens (max 4): "); scanf("%d", &nQ);
        printf("Enter number of Rooks (max 4): "); scanf("%d", &nR);
        printf("Enter number of Bishops (max 4): "); scanf("%d", &nB);
        printf("Enter number of Knights (max 4): "); scanf("%d", &nK);
        int total = nQ + nR + nB + nK;
        if (nQ < 0 || nQ > 4 || nR < 0 || nR > 4 || nB < 0 || nB > 4 || nK < 0 || nK > 4) {
            printf("Invalid numbers.\n"); continue;
        }
        if (total > 16) {
            printf("Total pieces cannot exceed 16.\n"); continue;
        }
        break;
    }
    
    // Input parameters per Image requirement
    int numberofgen, popSize;
    printf("Enter Number of Generations: ");
    scanf("%d", &numberofgen);
    
    printf("Enter Population Size (e.g. 10, max %d): ", MAX_POP);
    scanf("%d", &popSize);
    if(popSize > MAX_POP) popSize = MAX_POP;
    if(popSize < 2) popSize = 2; // Minimum for crossover

    // Place initial pieces
    int pieceNum = 1;
    for (int i = 0; i < nQ; i++) readPosition('Q', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nR; i++) readPosition('R', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nB; i++) readPosition('B', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nK; i++) readPosition('K', pieceNum++, board);
    
    // Create base chromosome
    char baseChromosome[SIZE];
    int idx = 0;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            baseChromosome[idx++] = board[r][c];
    
    // Initialize Population
    char population[MAX_POP][SIZE];
    double fitnessScores[MAX_POP];
    
    printf("\nInitializing Population...\n");
    for (int i = 0; i < popSize; i++) {
        copyArray(population[i], baseChromosome);
        if (i > 0) shuffle(population[i]); // Keep 0 as user input, shuffle others
        fitnessScores[i] = fitness(population[i]);
    }
    
    printPopulation(population, fitnessScores, (popSize > 5 ? 5 : popSize), "Initial Population (Top 5)");

    // Run GA
    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, numberofgen, popSize);
    
    // Final Result
    int bestIdx = 0;
    for (int i = 1; i < popSize; i++) {
        if (fitnessScores[i] > fitnessScores[bestIdx]) bestIdx = i;
    }
    
    printf("\n=== BEST SOLUTION ===\n");
    printf("Fitness: %.4f\n", fitnessScores[bestIdx]);
    printf("Board:\n");
    printf("  0 1 2 3\n");
    for (r = 0; r < ROWS; r++) {
        printf("%d ", r);
        for (c = 0; c < COLS; c++) {
            printf("%c ", population[bestIdx][r * COLS + c]);
        }
        printf("\n");
    }
    
    return 0;
}