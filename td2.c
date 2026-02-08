#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Global parameters that can be set by user
int MAX_GENERATIONS = 100;
int POPULATION_SIZE = 10;
int nQ = 0, nR = 0, nB = 0, nK = 0; // Piece counts
#define  PC  0.8  // Crossover probability
#define  PM  0.1  // Mutation probability

#define  SIZE  16
#define  ROWS  4
#define  COLS  4

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
            penalty++; // Penalize each column with more than 1 queen
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
    
    // Check threats for each piece
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'E') continue;
        
        int r1 = i / COLS;
        int c1 = i % COLS;
        char p1 = chrom[i];
        
        for (int j = i + 1; j < SIZE; j++) {
            if (chrom[j] == 'E') continue;
            
            int r2 = j / COLS;
            int c2 = j % COLS;
            int threatFromItoJ = 0;
            int threatFromJtoI = 0;
            
            // Check if piece i threatens piece j
            if (p1 == 'Q') {
                if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                    threatFromItoJ = 1;
            } else if (p1 == 'R') {
                if (r1 == r2 || c1 == c2)
                    threatFromItoJ = 1;
            } else if (p1 == 'B') {
                if (abs(r1 - r2) == abs(c1 - c2))
                    threatFromItoJ = 1;
            } else if (p1 == 'K') {
                if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                    (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                    threatFromItoJ = 1;
            }
            
            // Check if piece j threatens piece i
            char p2 = chrom[j];
            if (p2 == 'Q') {
                if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                    threatFromJtoI = 1;
            } else if (p2 == 'R') {
                if (r1 == r2 || c1 == c2)
                    threatFromJtoI = 1;
            } else if (p2 == 'B') {
                if (abs(r1 - r2) == abs(c1 - c2))
                    threatFromJtoI = 1;
            } else if (p2 == 'K') {
                if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                    (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                    threatFromJtoI = 1;
            }
            
            // Mark threatened pieces
            if (threatFromItoJ && !threatenedPieces[j]) {
                threatenedPieces[j] = 1;
                numThreatened++;
            }
            if (threatFromJtoI && !threatenedPieces[i]) {
                threatenedPieces[i] = 1;
                numThreatened++;
            }
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

// Tournament selection
void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[][SIZE], int numSelected) 
{
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
    }
}

// Crossover with probability PC
void crossover(char selected[][SIZE], char offspring[][SIZE], int numSelected) 
{
    int offspringCount = 0;
    
    // Create offspring through crossover
    for (int i = 0; i < numSelected; i += 2) {
        if (i + 1 >= numSelected) {
            // If odd number, just copy the last one
            copyArray(offspring[offspringCount++], selected[i]);
            break;
        }
        
        double randVal = (double)rand() / RAND_MAX;
        
        if (randVal < PC) {
            // Perform crossover
            int crossoverPoint = rand() % (SIZE - 1) + 1;
            
            // Child 1: first part from parent1, second from parent2
            for (int j = 0; j < crossoverPoint; j++) {
                offspring[offspringCount][j] = selected[i][j];
            }
            for (int j = crossoverPoint; j < SIZE; j++) {
                offspring[offspringCount][j] = selected[i + 1][j];
            }
            
            // Child 2: first part from parent2, second from parent1
            for (int j = 0; j < crossoverPoint; j++) {
                offspring[offspringCount + 1][j] = selected[i + 1][j];
            }
            for (int j = crossoverPoint; j < SIZE; j++) {
                offspring[offspringCount + 1][j] = selected[i][j];
            }
            
            offspringCount += 2;
        } else {
            // No crossover, just copy parents
            copyArray(offspring[offspringCount], selected[i]);
            copyArray(offspring[offspringCount + 1], selected[i + 1]);
            offspringCount += 2;
        }
    }
}

// Mutation with probability PM
void mutation(char population[][SIZE], int popSize) {
    for (int i = 0; i < popSize; i++) {
        for (int j = 0; j < SIZE; j++) {
            double randVal = (double)rand() / RAND_MAX;
            
            if (randVal < PM) {
                // Swap with random position
                int swapPos = rand() % SIZE;
                char temp = population[i][j];
                population[i][j] = population[i][swapPos];
                population[i][swapPos] = temp;
            }
        }
    }
}

// Apply piece count constraints
void applyPieceConstraints(char population[][SIZE], int popSize) {
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};
    
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
            while (counts[p] < targets[p]) {
                // Find an empty cell to place piece
                for (int attempt = 0; attempt < 100; attempt++) {
                    int pos = rand() % SIZE;
                    if (population[i][pos] == 'E') {
                        population[i][pos] = pieces[p];
                        counts[p]++;
                        break;
                    }
                }
            }
            
            while (counts[p] > targets[p]) {
                // Find this piece to remove
                for (int attempt = 0; attempt < 100; attempt++) {
                    int pos = rand() % SIZE;
                    if (population[i][pos] == pieces[p]) {
                        population[i][pos] = 'E';
                        counts[p]--;
                        break;
                    }
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
    // Create array of indices
    int indices[POPULATION_SIZE];
    for (int i = 0; i < POPULATION_SIZE; i++) indices[i] = i;
    
    // Sort indices by fitness (descending) using bubble sort
    for (int i = 0; i < POPULATION_SIZE - 1; i++) {
        for (int j = 0; j < POPULATION_SIZE - i - 1; j++) {
            if (oldFit[indices[j]] < oldFit[indices[j + 1]]) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Copy elite individuals to new population
    for (int i = 0; i < eliteCount; i++) {
        copyArray(newPop[i], oldPop[indices[i]]);
        newFit[i] = oldFit[indices[i]];
    }
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], int generations) {
    printf("\n=== EVOLUTION LOOP START (%d generations) ===\n", generations);
    
    for (int gen = 1; gen <= generations; gen++) {
        if (gen % 10 == 0 || gen == 1 || gen == generations) {
            printf("\n================ GENERATION %d ================\n", gen);
        }
        
        // Calculate statistics
        double bestFit = fitnessScores[0];
        double avgFit = 0;
        int bestIdx = 0;
        
        for (int i = 0; i < POPULATION_SIZE; i++) {
            if (fitnessScores[i] > bestFit) {
                bestFit = fitnessScores[i];
                bestIdx = i;
            }
            avgFit += fitnessScores[i];
        }
        avgFit /= POPULATION_SIZE;
        
        if (gen % 10 == 0 || gen == 1 || gen == generations) {
            printf("Best Fitness: %.4f | Average Fitness: %.4f\n", bestFit, avgFit);
        }
        
        // Check for perfect solution
        if (bestFit == 1.0) {
            printf("\n*** PERFECT SOLUTION FOUND! ***\n");
            printf("Perfect chromosome: ");
            printArray(population[bestIdx], SIZE);
            
            int threatenedPieces[SIZE];
            int conflicts = countThreatenedPieces(population[bestIdx], threatenedPieces);
            int penalty = calculatePenalty(population[bestIdx]);
            printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
            break;
        }
        
        // Tournament selection
        char selected[POPULATION_SIZE][SIZE];
        tournamentSelection(population, fitnessScores, selected, POPULATION_SIZE);
        
        // Crossover
        char offspring[POPULATION_SIZE][SIZE];
        crossover(selected, offspring, POPULATION_SIZE);
        
        // Mutation
        mutation(offspring, POPULATION_SIZE);
        
        // Apply piece count constraints
        applyPieceConstraints(offspring, POPULATION_SIZE);
        
        // Calculate fitness for offspring
        double offspringFitness[POPULATION_SIZE];
        for (int i = 0; i < POPULATION_SIZE; i++) {
            offspringFitness[i] = fitness(offspring[i]);
        }
        
        // Create new generation (elitism + offspring)
        char newPopulation[POPULATION_SIZE][SIZE];
        double newFitness[POPULATION_SIZE];
        
        // Keep 20% elite
        int eliteCount = POPULATION_SIZE * 0.2;
        if (eliteCount < 1) eliteCount = 1;
        
        elitism(population, fitnessScores, newPopulation, newFitness, eliteCount);
        
        // Fill rest with best offspring
        // First, sort offspring by fitness
        int offspringIndices[POPULATION_SIZE];
        for (int i = 0; i < POPULATION_SIZE; i++) offspringIndices[i] = i;
        
        for (int i = 0; i < POPULATION_SIZE - 1; i++) {
            for (int j = 0; j < POPULATION_SIZE - i - 1; j++) {
                if (offspringFitness[offspringIndices[j]] < offspringFitness[offspringIndices[j + 1]]) {
                    int temp = offspringIndices[j];
                    offspringIndices[j] = offspringIndices[j + 1];
                    offspringIndices[j + 1] = temp;
                }
            }
        }
        
        // Select best offspring to fill the population
        for (int i = eliteCount; i < POPULATION_SIZE; i++) {
            copyArray(newPopulation[i], offspring[offspringIndices[i - eliteCount]]);
            newFitness[i] = offspringFitness[offspringIndices[i - eliteCount]];
        }
        
        // Replace old population
        for (int i = 0; i < POPULATION_SIZE; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
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
        scanf("%d", &nQ);
        printf("Enter number of Rooks (0-16): ");
        scanf("%d", &nR);
        printf("Enter number of Bishops (0-16): ");
        scanf("%d", &nB);
        printf("Enter number of Knights (0-16): ");
        scanf("%d", &nK);
        
        int total = nQ + nR + nB + nK;
        
        if (total > 16) {
            printf("Total pieces cannot exceed 16. Currently: %d\n", total);
            continue;
        }
        
        if (nQ < 0 || nR < 0 || nB < 0 || nK < 0) {
            printf("Piece counts cannot be negative.\n");
            continue;
        }
        
        printf("\nGA Parameters:\n");
        printf("  Generations: %d\n", MAX_GENERATIONS);
        printf("  Population: %d\n", POPULATION_SIZE);
        printf("  Pieces: Q=%d, R=%d, B=%d, K=%d\n", nQ, nR, nB, nK);
        printf("  Total pieces: %d\n", total);
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
    printf("\n");
    
    // Create initial population
    char population[POPULATION_SIZE][SIZE];
    double fitnessScores[POPULATION_SIZE];
    
    printf("\n=== CREATING INITIAL POPULATION ===\n");
    for (int i = 0; i < POPULATION_SIZE; i++) {
        // Start with empty board
        for (int j = 0; j < SIZE; j++) {
            population[i][j] = 'E';
        }
        
        // Place pieces randomly
        int placed = 0;
        int targetQ = nQ, targetR = nR, targetB = nB, targetK = nK;
        
        while (placed < (nQ + nR + nB + nK)) {
            int pos = rand() % SIZE;
            if (population[i][pos] == 'E') {
                if (targetQ > 0) {
                    population[i][pos] = 'Q';
                    targetQ--;
                    placed++;
                } else if (targetR > 0) {
                    population[i][pos] = 'R';
                    targetR--;
                    placed++;
                } else if (targetB > 0) {
                    population[i][pos] = 'B';
                    targetB--;
                    placed++;
                } else if (targetK > 0) {
                    population[i][pos] = 'K';
                    targetK--;
                    placed++;
                }
            }
        }
        
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
    double bestFit = fitnessScores[0];
    int bestIdx = 0;
    for (int i = 1; i < POPULATION_SIZE; i++) {
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
    
    // Count piece types
    int qCount = 0, rCount = 0, bCount = 0, kCount = 0;
    for (int i = 0; i < SIZE; i++) {
        if (population[bestIdx][i] == 'Q') qCount++;
        else if (population[bestIdx][i] == 'R') rCount++;
        else if (population[bestIdx][i] == 'B') bCount++;
        else if (population[bestIdx][i] == 'K') kCount++;
    }
    
    printf("\nPiece counts in best solution: Q=%d, R=%d, B=%d, K=%d\n", 
           qCount, rCount, bCount, kCount);
    
    return 0;
}