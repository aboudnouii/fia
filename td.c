#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Global parameters that can be set by user
int MAX_GENERATIONS = 100;
int POPULATION_SIZE = 10;
int PIECE_COUNTS[4] = {0}; // Q, R, B, K
const double PC = 0.8;  // Crossover probability
const double PM = 0.1;  // Mutation probability

const int SIZE = 16;
const int ROWS = 4;
const int COLS = 4;

void printBoard(char board[ROWS][COLS]) {
    printf("\nBoard (4x4):\n");
    for (int r = 0; r < ROWS; r++) {
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

// Calculate penalty based on queen distribution across columns
int calculatePenalty(char chrom[]) {
    int penalty = 0;
    int queensInCol[COLS] = {0};
    
    // Count queens in each column
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'Q') {
            int col = i % COLS;
            queensInCol[col]++;
        }
    }
    
    // Penalize columns with more than 1 queen
    for (int c = 0; c < COLS; c++) {
        if (queensInCol[c] > 1) {
            penalty += queensInCol[c]; // Penalize each extra queen
        }
    }
    
    return penalty;
}

// Counts number of threatened pieces (not number of threats)
int countThreatenedPieces(char chrom[], int threatenedPieces[]) {
    int numThreatened = 0;
    
    // Initialize threatened array
    for (int i = 0; i < SIZE; i++) {
        threatenedPieces[i] = 0;
    }
    
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'E') continue;
        
        int r1 = i / COLS;
        int c1 = i % COLS;
        char p1 = chrom[i];
        
        for (int j = 0; j < SIZE; j++) {
            if (i == j || chrom[j] == 'E') continue;
            
            int r2 = j / COLS;
            int c2 = j % COLS;
            int isThreat = 0;
            
            if (p1 == 'Q') {
                if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                    isThreat = 1;
            } else if (p1 == 'R') {
                if (r1 == r2 || c1 == c2)
                    isThreat = 1;
            } else if (p1 == 'B') {
                if (abs(r1 - r2) == abs(c1 - c2))
                    isThreat = 1;
            } else if (p1 == 'K') {
                if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                    (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                    isThreat = 1;
            }
            
            if (isThreat) {
                threatenedPieces[j] = 1;  // Piece j is threatened
            }
        }
    }
    
    // Count number of threatened pieces
    for (int i = 0; i < SIZE; i++) {
        if (threatenedPieces[i] == 1) {
            numThreatened++;
        }
    }
    
    return numThreatened;
}

double fitness(char chrom[]) {
    int threatenedPieces[SIZE];
    int nb_conflicts = countThreatenedPieces(chrom, threatenedPieces);
    int penalty = calculatePenalty(chrom);
    
    return 1.0 / (1.0 + nb_conflicts + penalty);
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
        printf("Chromosome %d: ", i);
        printArray(population[i], SIZE);
        printf(" | Fitness: %.4f", fitnessScores[i]);
        
        // Show conflicts and penalty for debugging
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(population[i], threatenedPieces);
        int penalty = calculatePenalty(population[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
}

void copyArray(char dest[], char src[]) {
    for (int i = 0; i < SIZE; i++)
        dest[i] = src[i];
}

// Tournament selection with probability-based selection
void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[][SIZE], double selectedFitness[], 
                         int numSelected) 
{
    printf("\n=== TOURNAMENT SELECTION START ===\n");
    
    for (int s = 0; s < numSelected; s++) {
        // Select 2 random individuals
        int a = rand() % POPULATION_SIZE;
        int b = rand() % POPULATION_SIZE;
        
        // Make sure they're different
        while (b == a) {
            b = rand() % POPULATION_SIZE;
        }
        
        // Choose the better one (higher fitness)
        int winner = (fitnessScores[a] > fitnessScores[b]) ? a : b;
        
        copyArray(selected[s], population[winner]);
        selectedFitness[s] = fitnessScores[winner];
        
        printf("Selection %d: Chose individual %d (Fitness=%.4f)\n", 
               s, winner, fitnessScores[winner]);
    }
    
    printf("=== TOURNAMENT SELECTION END ===\n");
}

// Crossover with probability PC
void crossover(char population[][SIZE], double fitnessScores[],
               char offspring[][SIZE], double offspringFitness[]) 
{
    printf("\n=== CROSSOVER START (Pc=%.1f) ===\n", PC);
    
    int offspringCount = 0;
    
    // Create offspring through crossover
    for (int i = 0; i < POPULATION_SIZE; i += 2) {
        if (i + 1 >= POPULATION_SIZE) break;
        
        double randVal = (double)rand() / RAND_MAX;
        
        if (randVal < PC) {
            // Perform crossover
            int crossoverPoint = rand() % (SIZE - 1) + 1;
            
            // Parent 1
            for (int j = 0; j < crossoverPoint; j++) {
                offspring[offspringCount][j] = population[i][j];
            }
            for (int j = crossoverPoint; j < SIZE; j++) {
                offspring[offspringCount][j] = population[i + 1][j];
            }
            
            // Parent 2
            for (int j = 0; j < crossoverPoint; j++) {
                offspring[offspringCount + 1][j] = population[i + 1][j];
            }
            for (int j = crossoverPoint; j < SIZE; j++) {
                offspring[offspringCount + 1][j] = population[i][j];
            }
            
            printf("Crossover between %d and %d at point %d\n", 
                   i, i + 1, crossoverPoint);
            
            offspringCount += 2;
        } else {
            // No crossover, just copy parents
            copyArray(offspring[offspringCount], population[i]);
            copyArray(offspring[offspringCount + 1], population[i + 1]);
            
            printf("No crossover for %d and %d\n", i, i + 1);
            
            offspringCount += 2;
        }
    }
    
    // Calculate fitness for offspring
    for (int i = 0; i < offspringCount; i++) {
        offspringFitness[i] = fitness(offspring[i]);
    }
    
    printf("Created %d offspring\n", offspringCount);
    printf("=== CROSSOVER END ===\n");
}

// Mutation with probability PM
void mutation(char population[][SIZE], double fitnessScores[], int popSize) {
    printf("\n=== MUTATION START (Pm=%.1f) ===\n", PM);
    
    for (int i = 0; i < popSize; i++) {
        for (int j = 0; j < SIZE; j++) {
            double randVal = (double)rand() / RAND_MAX;
            
            if (randVal < PM) {
                // Mutate this position
                char current = population[i][j];
                char newChar;
                
                // Choose a random piece type
                do {
                    int pieceType = rand() % 5; // 0-4: Q, R, B, K, E
                    if (pieceType == 0) newChar = 'Q';
                    else if (pieceType == 1) newChar = 'R';
                    else if (pieceType == 2) newChar = 'B';
                    else if (pieceType == 3) newChar = 'K';
                    else newChar = 'E';
                } while (newChar == current);
                
                population[i][j] = newChar;
                printf("Mutated individual %d, position %d: %c -> %c\n", 
                       i, j, current, newChar);
            }
        }
        
        // Update fitness after mutation
        fitnessScores[i] = fitness(population[i]);
    }
    
    printf("=== MUTATION END ===\n");
}

// Apply piece count constraints
void applyPieceConstraints(char population[][SIZE], int popSize) {
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    
    for (int i = 0; i < popSize; i++) {
        // Count current pieces
        int counts[4] = {0};
        for (int j = 0; j < SIZE; j++) {
            if (population[i][j] == 'Q') counts[0]++;
            else if (population[i][j] == 'R') counts[1]++;
            else if (population[i][j] == 'B') counts[2]++;
            else if (population[i][j] == 'K') counts[3]++;
        }
        
        // Adjust counts to match target
        for (int p = 0; p < 4; p++) {
            while (counts[p] < PIECE_COUNTS[p]) {
                // Find an empty cell to place piece
                int pos = rand() % SIZE;
                if (population[i][pos] == 'E') {
                    population[i][pos] = pieces[p];
                    counts[p]++;
                }
            }
            
            while (counts[p] > PIECE_COUNTS[p]) {
                // Find this piece to remove
                int pos = rand() % SIZE;
                if (population[i][pos] == pieces[p]) {
                    population[i][pos] = 'E';
                    counts[p]--;
                }
            }
        }
    }
}

// Elitism: Keep best individuals
void elitism(char oldPop[][SIZE], double oldFit[], 
             char newPop[][SIZE], double newFit[],
             int eliteCount) 
{
    // Find indices of best individuals in old population
    int bestIndices[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) bestIndices[i] = i;
    
    // Sort indices by fitness (descending)
    for (int i = 0; i < POPULATION_SIZE - 1; i++) {
        for (int j = 0; j < POPULATION_SIZE - i - 1; j++) {
            if (oldFit[bestIndices[j]] < oldFit[bestIndices[j + 1]]) {
                int temp = bestIndices[j];
                bestIndices[j] = bestIndices[j + 1];
                bestIndices[j + 1] = temp;
            }
        }
    }
    
    // Copy elite individuals to new population
    for (int i = 0; i < eliteCount; i++) {
        copyArray(newPop[i], oldPop[bestIndices[i]]);
        newFit[i] = oldFit[bestIndices[i]];
    }
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], int generations) {
    printf("\n=== EVOLUTION LOOP START (%d generations) ===\n", generations);
    
    for (int gen = 1; gen <= generations; gen++) {
        printf("\n================ GENERATION %d ================\n", gen);
        
        // Print current population statistics
        double bestFit = 0;
        double avgFit = 0;
        for (int i = 0; i < POPULATION_SIZE; i++) {
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
        }
        avgFit /= POPULATION_SIZE;
        
        printf("Best Fitness: %.4f | Average Fitness: %.4f\n", bestFit, avgFit);
        
        // Check for perfect solution
        if (bestFit == 1.0) {
            printf("\n*** PERFECT SOLUTION FOUND! ***\n");
            for (int i = 0; i < POPULATION_SIZE; i++) {
                if (fitnessScores[i] == 1.0) {
                    printf("Perfect chromosome: ");
                    printArray(population[i], SIZE);
                    
                    int threatenedPieces[SIZE];
                    int conflicts = countThreatenedPieces(population[i], threatenedPieces);
                    int penalty = calculatePenalty(population[i]);
                    printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
                    break;
                }
            }
            break;
        }
        
        // Create offspring through selection, crossover, mutation
        char offspring[POPULATION_SIZE][SIZE];
        double offspringFitness[POPULATION_SIZE];
        
        // Selection (tournament selection)
        char selected[POPULATION_SIZE][SIZE];
        double selectedFitness[POPULATION_SIZE];
        tournamentSelection(population, fitnessScores, selected, selectedFitness, POPULATION_SIZE);
        
        // Crossover
        crossover(selected, selectedFitness, offspring, offspringFitness);
        
        // Mutation
        mutation(offspring, offspringFitness, POPULATION_SIZE);
        
        // Apply piece count constraints
        applyPieceConstraints(offspring, POPULATION_SIZE);
        
        // Calculate fitness after constraints
        for (int i = 0; i < POPULATION_SIZE; i++) {
            offspringFitness[i] = fitness(offspring[i]);
        }
        
        // Create new generation (elitism + offspring)
        char newPopulation[POPULATION_SIZE][SIZE];
        double newFitness[POPULATION_SIZE];
        
        // Keep 20% elite
        int eliteCount = POPULATION_SIZE * 0.2;
        elitism(population, fitnessScores, newPopulation, newFitness, eliteCount);
        
        // Fill rest with best offspring
        for (int i = eliteCount; i < POPULATION_SIZE; i++) {
            copyArray(newPopulation[i], offspring[i - eliteCount]);
            newFitness[i] = offspringFitness[i - eliteCount];
        }
        
        // Replace old population
        for (int i = 0; i < POPULATION_SIZE; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
        
        // Print generation statistics
        printf("Generation %d completed.\n", gen);
    }
    
    printf("\n=== EVOLUTION LOOP END ===\n");
}

int main() {
    srand(time(NULL));
    
    printf("=== CHESS PIECE PLACEMENT GENETIC ALGORITHM ===\n");
    
    // Get GA parameters from user
    printf("\n=== SET GA PARAMETERS ===\n");
    printf("Enter number of generations: ");
    scanf("%d", &MAX_GENERATIONS);
    
    printf("Enter population size: ");
    scanf("%d", &POPULATION_SIZE);
    
    printf("\n=== SET PIECE COUNTS ===\n");
    while (1) {
        printf("Enter number of Queens (0-16): ");
        scanf("%d", &PIECE_COUNTS[0]);
        printf("Enter number of Rooks (0-16): ");
        scanf("%d", &PIECE_COUNTS[1]);
        printf("Enter number of Bishops (0-16): ");
        scanf("%d", &PIECE_COUNTS[2]);
        printf("Enter number of Knights (0-16): ");
        scanf("%d", &PIECE_COUNTS[3]);
        
        int total = PIECE_COUNTS[0] + PIECE_COUNTS[1] + 
                    PIECE_COUNTS[2] + PIECE_COUNTS[3];
        
        if (total > 16) {
            printf("Total pieces cannot exceed 16. Currently: %d\n", total);
            continue;
        }
        
        printf("\nGA Parameters:\n");
        printf("  Generations: %d\n", MAX_GENERATIONS);
        printf("  Population: %d\n", POPULATION_SIZE);
        printf("  Pieces: Q=%d, R=%d, B=%d, K=%d\n", 
               PIECE_COUNTS[0], PIECE_COUNTS[1], PIECE_COUNTS[2], PIECE_COUNTS[3]);
        printf("  Pc=%.1f, Pm=%.1f\n", PC, PM);
        break;
    }
    
    // Initialize board
    char board[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    printBoard(board);
    
    // Let user place initial pieces (optional)
    char placeInitial;
    printf("\nPlace initial pieces? (y/n): ");
    scanf(" %c", &placeInitial);
    
    int nQ = PIECE_COUNTS[0];
    int nR = PIECE_COUNTS[1];
    int nB = PIECE_COUNTS[2];
    int nK = PIECE_COUNTS[3];
    
    if (placeInitial == 'y' || placeInitial == 'Y') {
        int pieceNum = 1;
        for (int i = 0; i < nQ; i++) readPosition('Q', pieceNum++, board);
        pieceNum = 1;
        for (int i = 0; i < nR; i++) readPosition('R', pieceNum++, board);
        pieceNum = 1;
        for (int i = 0; i < nB; i++) readPosition('B', pieceNum++, board);
        pieceNum = 1;
        for (int i = 0; i < nK; i++) readPosition('K', pieceNum++, board);
        
        printBoard(board);
    }
    
    // Create initial chromosome
    char chromosome[SIZE];
    int idx = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];
    
    printf("\nInitial chromosome from board: ");
    printArray(chromosome, SIZE);
    
    // Create initial population
    char population[POPULATION_SIZE][SIZE];
    double fitnessScores[POPULATION_SIZE];
    
    printf("\n\n=== CREATING INITIAL POPULATION ===\n");
    for (int i = 0; i < POPULATION_SIZE; i++) {
        copyArray(population[i], chromosome);
        shuffle(population[i]);
        
        // Apply piece count constraints
        applyPieceConstraints(population, POPULATION_SIZE);
        
        fitnessScores[i] = fitness(population[i]);
        
        printf("Chromosome %d: ", i);
        printArray(population[i], SIZE);
        
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(population[i], threatenedPieces);
        int penalty = calculatePenalty(population[i]);
        printf(" | Fitness: %.4f | Conflicts: %d | Penalty: %d\n", 
               fitnessScores[i], conflicts, penalty);
    }
    
    // Run evolution
    evolutionLoop(population, fitnessScores, MAX_GENERATIONS);
    
    // Display final results
    printf("\n\n=== FINAL RESULTS ===\n");
    
    // Find best solution
    double bestFit = 0;
    int bestIdx = 0;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        if (fitnessScores[i] > bestFit) {
            bestFit = fitnessScores[i];
            bestIdx = i;
        }
    }
    
    printf("\nBest Solution Found:\n");
    printf("Chromosome: ");
    printArray(population[bestIdx], SIZE);
    
    int threatenedPieces[SIZE];
    int conflicts = countThreatenedPieces(population[bestIdx], threatenedPieces);
    int penalty = calculatePenalty(population[bestIdx]);
    printf("\nFitness: %.4f | Conflicts: %d | Penalty: %d\n", 
           bestFit, conflicts, penalty);
    
    printf("\nBoard representation:\n");
    printf("    0 1 2 3\n");
    printf("    -------\n");
    for (int r = 0; r < ROWS; r++) {
        printf("%d | ", r);
        for (int c = 0; c < COLS; c++) {
            char piece = population[bestIdx][r * COLS + c];
            printf("%c ", (piece == 'E') ? '.' : piece);
        }
        printf("\n");
    }
    
    // Show threatened pieces
    printf("\nThreatened pieces (marked with *):\n");
    printf("    0 1 2 3\n");
    printf("    -------\n");
    for (int r = 0; r < ROWS; r++) {
        printf("%d | ", r);
        for (int c = 0; c < COLS; c++) {
            int idx = r * COLS + c;
            char piece = population[bestIdx][idx];
            if (piece == 'E') {
                printf(". ");
            } else if (threatenedPieces[idx]) {
                printf("%c*", piece);
            } else {
                printf("%c ", piece);
            }
        }
        printf("\n");
    }
    
    return 0;
}