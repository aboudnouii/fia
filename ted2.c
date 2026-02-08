#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int SIZE = 16;
const int ROWS = 4;
const int COLS = 4;
const int POPULATION = 10;
const double PC = 0.8;
const double PM = 0.1;

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

int calculatePenalty(char chrom[]) {
    int penalty = 0;
    int queensInCol[COLS];
    
    for (int i = 0; i < COLS; i++) {
        queensInCol[i] = 0;
    }
    
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'Q') {
            int col = i % COLS;
            queensInCol[col]++;
        }
    }
    
    for (int c = 0; c < COLS; c++) {
        if (queensInCol[c] > 1) {
            penalty++; 
        }
    }
    
    return penalty;
}

int countThreatenedPieces(char chrom[], int threatenedPieces[]) {
    int numThreatened = 0;
    
    for (int i = 0; i < SIZE; i++) {
        threatenedPieces[i] = 0;
    }
    
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

void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[6][SIZE], double selectedFitness[]) 
{
    printf("\n=== TOURNAMENT SELECTION START ===\n");
    
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < SIZE; j++)
            selected[i][j] = 'E';

    double tempFitnessScores[POPULATION];
    for (int k = 0; k < POPULATION; k++) {
        tempFitnessScores[k] = fitnessScores[k];
    }
    
    printf("Initial population for selection:\n");
    for (int k = 0; k < POPULATION; k++) {
        printf("  %d: ", k);
        printArray(population[k], SIZE);
        printf(" | Fitness = %.4f", tempFitnessScores[k]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(population[k], threatenedPieces);
        int penalty = calculatePenalty(population[k]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    
    for (int s = 0; s < 6; s++) {
        int a, b;
        do {
            a = rand() % POPULATION;
        } while (tempFitnessScores[a] == -1.0);
        do {
            b = rand() % POPULATION;
        } while (b == a || tempFitnessScores[b] == -1.0);

        printf("\nTournament %d:", s+1);
        printf("\n  Candidate %d: ", a);
        printArray(population[a], SIZE);
        printf(" | Fitness=%.4f", tempFitnessScores[a]);
        int threatenedPiecesA[SIZE];
        int conflictsA = countThreatenedPieces(population[a], threatenedPiecesA);
        int penaltyA = calculatePenalty(population[a]);
        printf(" | Conflicts: %d | Penalty: %d", conflictsA, penaltyA);
        
        printf("\n  Candidate %d: ", b);
        printArray(population[b], SIZE);
        printf(" | Fitness=%.4f", tempFitnessScores[b]);
        int threatenedPiecesB[SIZE];
        int conflictsB = countThreatenedPieces(population[b], threatenedPiecesB);
        int penaltyB = calculatePenalty(population[b]);
        printf(" | Conflicts: %d | Penalty: %d", conflictsB, penaltyB);
        
        int winner;
        if (tempFitnessScores[a] > tempFitnessScores[b]) {
            winner = a;
            printf("\n  Winner: Candidate %d (Fitness=%.4f)\n", a, tempFitnessScores[a]);
        } else {
            winner = b;
            printf("\n  Winner: Candidate %d (Fitness=%.4f)\n", b, tempFitnessScores[b]);
        }

        copyArray(selected[s], population[winner]);
        selectedFitness[s] = tempFitnessScores[winner];
        tempFitnessScores[winner] = -1.0;
        
        printf("  Selected chromosome: ");
        printArray(selected[s], SIZE);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(selected[s], threatenedPieces);
        int penalty = calculatePenalty(selected[s]);
        printf(" | Fitness: %.4f | Conflicts: %d | Penalty: %d\n", selectedFitness[s], conflicts, penalty);
    }
    
    printf("\n=== TOURNAMENT SELECTION END - Selected chromosomes ===\n");
    for (int i = 0; i < 6; i++) {
        printf("Selected[%d]: ", i);
        printArray(selected[i], SIZE);
        printf(" | Fitness: %.4f", selectedFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(selected[i], threatenedPieces);
        int penalty = calculatePenalty(selected[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
}

void crossover(char selected[][SIZE], double selectedFitness[],
               char offspring[][SIZE], double offspringFitness[], int popSize) 
{
    for (int i = 0; i < popSize; i++) {
        copyArray(offspring[i], selected[i]);
        offspringFitness[i] = selectedFitness[i];
    }

    for (int i = 0; i < popSize - 1; i += 2) {
        double r = (double)rand() / RAND_MAX;
        
        if (r < PC) {
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
    }
}

void mutation(char population[][SIZE], double fitnessScores[],
              int nQ, int nR, int nB, int nK, int popSize)
{
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};
    
    for (int c = 0; c < popSize; c++) {
        double r = (double)rand() / RAND_MAX;
        
        if (r < PM) {
            int p1 = rand() % SIZE;
            int p2 = rand() % SIZE;
            char temp = population[c][p1];
            population[c][p1] = population[c][p2];
            population[c][p2] = temp;
        }

        for (int p = 0; p < 4; p++) {
            int count = 0;
            for (int i = 0; i < SIZE; i++)
                if (population[c][i] == pieces[p]) count++;

            while (count < targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == 'E') {
                    population[c][pos] = pieces[p];
                    count++;
                }
            }
            
            while (count > targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == pieces[p]) {
                    population[c][pos] = 'E';
                    count--;
                }
            }
        }
        
        fitnessScores[c] = fitness(population[c]);
    }
}

void replacement(char oldPopulation[][SIZE], double oldFitness[],
                 char newPopulation[][SIZE], double newFitness[],
                 char resultPopulation[][SIZE], double resultFitness[]) 
{
    printf("\n=== REPLACEMENT START ===\n");
    printf("Old population (size=%d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  Old[%d]: ", i);
        printArray(oldPopulation[i], SIZE);
        printf(" | Fitness=%.4f", oldFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(oldPopulation[i], threatenedPieces);
        int penalty = calculatePenalty(oldPopulation[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    
    printf("New population (size=%d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  New[%d]: ", i);
        printArray(newPopulation[i], SIZE);
        printf(" | Fitness=%.4f", newFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(newPopulation[i], threatenedPieces);
        int penalty = calculatePenalty(newPopulation[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    
    char combined[20][SIZE];
    double combinedFitness[20];
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(combined[i], oldPopulation[i]);
        combinedFitness[i] = oldFitness[i];
        copyArray(combined[i + POPULATION], newPopulation[i]);
        combinedFitness[i + POPULATION] = newFitness[i];
    }
    
    printf("\nCombined population (size=20) before sorting:\n");
    for (int i = 0; i < 20; i++) {
        printf("  Combined[%d]: ", i);
        printArray(combined[i], SIZE);
        printf(" | Fitness: %.4f", combinedFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(combined[i], threatenedPieces);
        int penalty = calculatePenalty(combined[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19 - i; j++) {
            if (combinedFitness[j] < combinedFitness[j + 1]) {
                double tempFit = combinedFitness[j];
                combinedFitness[j] = combinedFitness[j + 1];
                combinedFitness[j + 1] = tempFit;
                
                char tempChrom[SIZE];
                copyArray(tempChrom, combined[j]);
                copyArray(combined[j], combined[j + 1]);
                copyArray(combined[j + 1], tempChrom);
            }
        }
    }
    
    printf("\nCombined population after sorting (descending fitness):\n");
    for (int i = 0; i < 20; i++) {
        printf("  Combined[%d]: ", i);
        printArray(combined[i], SIZE);
        printf(" | Fitness: %.4f", combinedFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(combined[i], threatenedPieces);
        int penalty = calculatePenalty(combined[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(resultPopulation[i], combined[i]);
        resultFitness[i] = combinedFitness[i];
    }
    
    printf("\nFinal result population (top %d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  Result[%d]: ", i);
        printArray(resultPopulation[i], SIZE);
        printf(" | Fitness: %.4f", resultFitness[i]);
        int threatenedPieces[SIZE];
        int conflicts = countThreatenedPieces(resultPopulation[i], threatenedPieces);
        int penalty = calculatePenalty(resultPopulation[i]);
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    }
    printf("\n=== REPLACEMENT END ===\n");
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], 
                   int nQ, int nR, int nB, int nK, int generations) 
{
    printf("\n=== EVOLUTION LOOP START ===\n");
    
    for (int gen = 1; gen <= generations; gen++) {
        printf("\n\n================ GENERATION %d ================\n", gen);
        
        printPopulation(population, fitnessScores, POPULATION, "Current Population");
        
        double currentFitness[POPULATION];
        for (int i = 0; i < POPULATION; i++) {
            currentFitness[i] = fitnessScores[i];
        }

        char selected[6][SIZE];
        double selectedFitness[6];
        char offspring[POPULATION][SIZE];
        double offspringFitness[POPULATION];
        char newPopulation[POPULATION][SIZE];
        double newFitness[POPULATION];
        
        tournamentSelection(population, currentFitness, selected, selectedFitness);
        
        for (int i = 0; i < POPULATION; i++) {
            fitnessScores[i] = fitness(population[i]);
        }

        crossover(selected, selectedFitness, offspring, offspringFitness, POPULATION);
        
        mutation(offspring, offspringFitness, nQ, nR, nB, nK, POPULATION);
        
        replacement(population, fitnessScores, offspring, offspringFitness, 
                    newPopulation, newFitness);
        
        for (int i = 0; i < POPULATION; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
        
        double bestFit = fitnessScores[0];
        double avgFit = 0;
        int totalConflicts = 0;
        int totalPenalty = 0;
        
        for (int i = 0; i < POPULATION; i++) {
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
            
            int threatenedPieces[SIZE];
            totalConflicts += countThreatenedPieces(population[i], threatenedPieces);
            totalPenalty += calculatePenalty(population[i]);
        }
        avgFit /= POPULATION;
        double avgConflicts = (double)totalConflicts / POPULATION;
        double avgPenalty = (double)totalPenalty / POPULATION;
        
        printf("\nGeneration %d Statistics:", gen);
        printf("\n  Best Fitness: %.4f", bestFit);
        printf("\n  Average Fitness: %.4f", avgFit);
        printf("\n  Average Conflicts: %.1f", avgConflicts);
        printf("\n  Average Penalty: %.1f\n", avgPenalty);
        
        if (bestFit == 1.0) {
            printf("\n*** PERFECT SOLUTION FOUND! ***\n");
            printf("Perfect chromosome: ");
            for (int i = 0; i < POPULATION; i++) {
                if (fitnessScores[i] == 1.0) {
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
    }
    printf("\n=== EVOLUTION LOOP END ===\n");
}

int main() {
    srand(time(NULL));
    
    printf("=== CHESS PIECE PLACEMENT GENETIC ALGORITHM ===\n");
    
    printf("\n=== SET GA PARAMETERS ===\n");
    int MAX_GENERATIONS;
    printf("Enter number of generations: ");
    scanf("%d", &MAX_GENERATIONS);
    
    int POPULATION_SIZE;
    printf("Enter population size: ");
    scanf("%d", &POPULATION_SIZE);
    
    printf("GA Parameters set: Generations=%d, Population=%d, Pc=%.1f, Pm=%.1f\n", 
           MAX_GENERATIONS, POPULATION_SIZE, PC, PM);
    
    char board[ROWS][COLS];
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    printBoard(board);
    
    int nQ, nR, nB, nK;
    while (1) {
        printf("\nEnter number of Queens (max 16): "); scanf("%d", &nQ);
        printf("Enter number of Rooks (max %d): ", 16 - nQ); scanf("%d", &nR);
        printf("Enter number of Bishops (max %d): ", 16 - (nQ + nR)); scanf("%d", &nB);
        printf("Enter number of Knights (max %d): ", 16 - (nQ + nR + nB)); scanf("%d", &nK);
        int total = nQ + nR + nB + nK;
        if (nQ < 0 || nQ > 16 || nR < 0 || nR > 16 - nQ || nB < 0 || nB > 16 - (nQ + nR) || nK < 0 || nK > 16 - (nQ + nR + nB)) {
            printf("Each piece must be between 0 and its maximum allowed count.\n");
            continue;
        }
        if (total > 16) {
            printf("Total pieces cannot exceed 16. Currently: %d\n", total);
            continue;
        }
        printf("Piece counts: Q=%d, R=%d, B=%d, K=%d (Total: %d)\n", nQ, nR, nB, nK, total);
        break;
    }
    
    int pieceNum = 1;
    for (int i = 0; i < nQ; i++) readPosition('Q', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nR; i++) readPosition('R', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nB; i++) readPosition('B', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nK; i++) readPosition('K', pieceNum++, board);
    
    printBoard(board);
    
    char chromosome[SIZE];
    int idx = 0;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];
    
    printf("\nInitial chromosome from board: ");
    printArray(chromosome, SIZE);
    printf("\nFitness: %.4f", fitness(chromosome));
    int threatenedPieces[SIZE];
    int conflicts = countThreatenedPieces(chromosome, threatenedPieces);
    int penalty = calculatePenalty(chromosome);
    printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
    
    char population[POPULATION_SIZE][SIZE];
    double fitnessScores[POPULATION_SIZE];
    
    printf("\n=== INITIAL POPULATION CREATION ===\n");
    for (int i = 0; i < POPULATION_SIZE; i++) {
        printf("\nCreating chromosome %d:\n", i);
        printf("  Original: ");
        printArray(chromosome, SIZE);
        printf("\n  Fitness: %.4f", fitness(chromosome));
        printf(" | Conflicts: %d | Penalty: %d\n", conflicts, penalty);
        
        for (int j = 0; j < SIZE; j++)
            population[i][j] = chromosome[j];
        shuffle(population[i]);
        
        printf("\n  After shuffle: ");
        printArray(population[i], SIZE);
        
        fitnessScores[i] = fitness(population[i]);
        conflicts = countThreatenedPieces(population[i], threatenedPieces);
        penalty = calculatePenalty(population[i]);
        printf("\n  Fitness: %.4f | Conflicts: %d | Penalty: %d\n", 
               fitnessScores[i], conflicts, penalty);
    }
    
    printf("\n=== INITIAL POPULATION ===\n");
    printPopulation(population, fitnessScores, POPULATION_SIZE, "Initial Population");
    
    printf("\n\n=== GENETIC ALGORITHM STEPS ===\n");
    
    printf("\n\n=== STEP 1: TOURNAMENT SELECTION ===\n");
    char selected[6][SIZE];
    double selectedFitness[6];
    double initialFitnessCopy[POPULATION_SIZE];
    for(int i = 0; i < POPULATION_SIZE; i++) initialFitnessCopy[i] = fitnessScores[i];
    tournamentSelection(population, initialFitnessCopy, selected, selectedFitness);
    
    printf("\n\n=== STEP 2: CROSSOVER ===\n");
    char finalPopulation[POPULATION_SIZE][SIZE];
    double finalFitness[POPULATION_SIZE];
    crossover(selected, selectedFitness, finalPopulation, finalFitness, POPULATION_SIZE);
    
    printf("\n\n=== STEP 3: MUTATION ===\n");
    mutation(finalPopulation, finalFitness, nQ, nR, nB, nK, POPULATION_SIZE);
    
    printf("\n\n=== STEP 4: REPLACEMENT ===\n");
    char bestPopulation[POPULATION_SIZE][SIZE];
    double bestFitness[POPULATION_SIZE];
    replacement(population, fitnessScores, finalPopulation, finalFitness, bestPopulation, bestFitness);
    
    for (int i = 0; i < POPULATION_SIZE; i++) {
        copyArray(population[i], bestPopulation[i]);
        fitnessScores[i] = bestFitness[i];
    }
    
    printf("\n\n=== POPULATION AFTER ONE COMPLETE CYCLE ===\n");
    printPopulation(population, fitnessScores, POPULATION_SIZE, "Population after one cycle");

    printf("\n\n=== STARTING EVOLUTION LOOP FOR %d GENERATIONS ===\n", MAX_GENERATIONS);
    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, MAX_GENERATIONS);
    
    printf("\n\n=== FINAL RESULTS ===\n");
    printPopulation(population, fitnessScores, POPULATION_SIZE, "Final Population");
    
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
    printf("\nFitness: %.4f", bestFit);
    int bestThreatenedPieces[SIZE];
    int bestConflicts = countThreatenedPieces(population[bestIdx], bestThreatenedPieces);
    int bestPenalty = calculatePenalty(population[bestIdx]);
    printf(" | Conflicts: %d | Penalty: %d\n", bestConflicts, bestPenalty);
    
    printf("\nBoard representation of best solution:\n");
    printf("    0 1 2 3\n");
    printf("    -------\n");
    for (int r = 0; r < ROWS; r++) {
        printf("%d | ", r);
        for (int c = 0; c < COLS; c++) {
            printf("%c ", population[bestIdx][r * COLS + c]);
        }
        printf("\n");
    }
    
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