#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int SIZE = 16;
const int ROWS = 4;
const int COLS = 4;
const int POPULATION = 10;

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

void Conflicts(char chrom[], int out[]) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = 0;
        if (chrom[i] == 'E') continue;
        int r1 = i / COLS;
        int c1 = i % COLS;
        char p1 = chrom[i];
        for (int j = 0; j < SIZE; j++) {
            if (i == j || chrom[j] == 'E') continue;
            int r2 = j / COLS;
            int c2 = j % COLS;
            if (p1 == 'Q') {
                if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                    out[i]++;
            } else if (p1 == 'R') {
                if (r1 == r2 || c1 == c2)
                    out[i]++;
            } else if (p1 == 'B') {
                if (abs(r1 - r2) == abs(c1 - c2))
                    out[i]++;
            } else if (p1 == 'K') {
                if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                    (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                    out[i]++;
            }
        }
    }
}

double fitness(char chrom[]) {
    int conflicts[SIZE];
    Conflicts(chrom, conflicts);
    int nb_conflicts = 0;
    for (int i = 0; i < SIZE; i++)
        nb_conflicts += conflicts[i];
    return 1.0 / (1 + nb_conflicts);
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
        printf(" | Fitness: %.4f\n", fitnessScores[i]);
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
        printf("  %d: Fitness = %.4f\n", k, tempFitnessScores[k]);
    }
    
    for (int s = 0; s < 6; s++) {
        int a, b;
        do {
            a = rand() % POPULATION;
        } while (tempFitnessScores[a] == -1.0);
        do {
            b = rand() % POPULATION;
        } while (b == a || tempFitnessScores[b] == -1.0);

        printf("\nTournament %d: Candidate %d (Fitness=%.4f) vs Candidate %d (Fitness=%.4f)\n",
               s+1, a, tempFitnessScores[a], b, tempFitnessScores[b]);
        
        int winner;
        if (tempFitnessScores[a] > tempFitnessScores[b]) {
            winner = a;
            printf("  Winner: Candidate %d (Fitness=%.4f)\n", a, tempFitnessScores[a]);
        } else {
            winner = b;
            printf("  Winner: Candidate %d (Fitness=%.4f)\n", b, tempFitnessScores[b]);
        }

        copyArray(selected[s], population[winner]);
        selectedFitness[s] = tempFitnessScores[winner];
        tempFitnessScores[winner] = -1.0;
        
        printf("  Selected chromosome: ");
        printArray(selected[s], SIZE);
        printf("\n");
    }
    
    printf("\n=== TOURNAMENT SELECTION END - Selected chromosomes ===\n");
    for (int i = 0; i < 6; i++) {
        printf("Selected[%d]: ", i);
        printArray(selected[i], SIZE);
        printf(" | Fitness: %.4f\n", selectedFitness[i]);
    }
}

void crossover(char selected[6][SIZE], double selectedFitness[],
               char finalPopulation[POPULATION][SIZE], double finalFitness[]) 
{
    printf("\n=== CROSSOVER START ===\n");
    printf("Selected parents for crossover:\n");
    for (int i = 0; i < 6; i++) {
        printf("  Parent[%d]: ", i);
        printArray(selected[i], SIZE);
        printf(" | Fitness: %.4f\n", selectedFitness[i]);
    }
    
    char tempPopulation[12][SIZE];
    double tempFitness[12];

    for (int i = 0; i < 12; i++)
        for (int j = 0; j < SIZE; j++)
            tempPopulation[i][j] = 'E';
    
    for (int i = 0; i < 6; i++) {
        copyArray(tempPopulation[i], selected[i]);
        tempFitness[i] = selectedFitness[i];
    }
    
    int nextChild = 6;
    for (int p = 0; p < 3; p++) {
        int p1 = p * 2;
        int p2 = p * 2 + 1;
        
        printf("\nCrossover between Parent[%d] and Parent[%d]:\n", p1, p2);
        
        for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p1][i];
        for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p2][i];
        tempFitness[nextChild] = fitness(tempPopulation[nextChild]);
        printf("  Child[%d]: ", nextChild);
        printArray(tempPopulation[nextChild], SIZE);
        printf(" | Fitness: %.4f\n", tempFitness[nextChild]);
        nextChild++;
        
        for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p2][i];
        for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p1][i];
        tempFitness[nextChild] = fitness(tempPopulation[nextChild]);
        printf("  Child[%d]: ", nextChild);
        printArray(tempPopulation[nextChild], SIZE);
        printf(" | Fitness: %.4f\n", tempFitness[nextChild]);
        nextChild++;
    }

    printf("\n=== CROSSOVER END - All chromosomes in tempPopulation ===\n");
    for (int i = 0; i < 12; i++) {
        printf("Temp[%d]: ", i);
        printArray(tempPopulation[i], SIZE);
        printf(" | Fitness: %.4f\n", tempFitness[i]);
    }
    
    printf("\nSelecting final population (first 4 parents + 6 children):\n");
    for (int i = 0; i < 4; i++) {
        copyArray(finalPopulation[i], tempPopulation[i]);
        finalFitness[i] = tempFitness[i];
        printf("  Final[%d] (from parent): ", i);
        printArray(finalPopulation[i], SIZE);
        printf(" | Fitness: %.4f\n", finalFitness[i]);
    }
    for (int i = 0; i < 6; i++) {
        copyArray(finalPopulation[i + 4], tempPopulation[i + 6]);
        finalFitness[i + 4] = tempFitness[i + 6];
        printf("  Final[%d] (from child): ", i + 4);
        printArray(finalPopulation[i + 4], SIZE);
        printf(" | Fitness: %.4f\n", finalFitness[i + 4]);
    }
}

void mutation(char population[][SIZE], double fitnessScores[],
              int nQ, int nR, int nB, int nK)
{
    printf("\n=== MUTATION START ===\n");
    printf("Target counts: Q=%d, R=%d, B=%d, K=%d\n", nQ, nR, nB, nK);
    
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};
    
    for (int c = 0; c < POPULATION; c++) {
        printf("\nChromosome %d before mutation: ", c);
        printArray(population[c], SIZE);
        printf(" | Fitness before: %.4f\n", fitnessScores[c]);
        
        int countsBefore[4] = {0};
        for (int i = 0; i < SIZE; i++) {
            if (population[c][i] == 'Q') countsBefore[0]++;
            else if (population[c][i] == 'R') countsBefore[1]++;
            else if (population[c][i] == 'B') countsBefore[2]++;
            else if (population[c][i] == 'K') countsBefore[3]++;
        }
        printf("  Counts before: Q=%d, R=%d, B=%d, K=%d\n", 
               countsBefore[0], countsBefore[1], countsBefore[2], countsBefore[3]);
        
        for (int p = 0; p < 4; p++) {
            int count = 0;
            for (int i = 0; i < SIZE; i++)
                if (population[c][i] == pieces[p])
                    count++;

            printf("  Processing %c: current=%d, target=%d\n", pieces[p], count, targets[p]);
            
            while (count < targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == 'E') {
                    population[c][pos] = pieces[p];
                    count++;
                    printf("    Added %c at position %d\n", pieces[p], pos);
                }
            }
            
            while (count > targets[p]) {
                int pos = rand() % SIZE;
                if (population[c][pos] == pieces[p]) {
                    population[c][pos] = 'E';
                    count--;
                    printf("    Removed %c from position %d\n", pieces[p], pos);
                }
            }
        }
        
        int countsAfter[4] = {0};
        for (int i = 0; i < SIZE; i++) {
            if (population[c][i] == 'Q') countsAfter[0]++;
            else if (population[c][i] == 'R') countsAfter[1]++;
            else if (population[c][i] == 'B') countsAfter[2]++;
            else if (population[c][i] == 'K') countsAfter[3]++;
        }
        printf("  Counts after: Q=%d, R=%d, B=%d, K=%d\n", 
               countsAfter[0], countsAfter[1], countsAfter[2], countsAfter[3]);

        fitnessScores[c] = fitness(population[c]);
        printf("  Chromosome after mutation: ");
        printArray(population[c], SIZE);
        printf(" | Fitness after: %.4f\n", fitnessScores[c]);
    }
    printf("\n=== MUTATION END ===\n");
}

void replacement(char oldPopulation[][SIZE], double oldFitness[],
                 char newPopulation[][SIZE], double newFitness[],
                 char resultPopulation[][SIZE], double resultFitness[]) 
{
    printf("\n=== REPLACEMENT START ===\n");
    printf("Old population (size=%d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  Old[%d]: Fitness=%.4f\n", i, oldFitness[i]);
    }
    
    printf("New population (size=%d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  New[%d]: Fitness=%.4f\n", i, newFitness[i]);
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
        printf(" | Fitness: %.4f\n", combinedFitness[i]);
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
        printf("  Combined[%d]: Fitness=%.4f\n", i, combinedFitness[i]);
    }
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(resultPopulation[i], combined[i]);
        resultFitness[i] = combinedFitness[i];
    }
    
    printf("\nFinal result population (top %d):\n", POPULATION);
    for (int i = 0; i < POPULATION; i++) {
        printf("  Result[%d]: ", i);
        printArray(resultPopulation[i], SIZE);
        printf(" | Fitness: %.4f\n", resultFitness[i]);
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

        crossover(selected, selectedFitness, offspring, offspringFitness);
        
        mutation(offspring, offspringFitness, nQ, nR, nB, nK);
        
        replacement(population, fitnessScores, offspring, offspringFitness, 
                    newPopulation, newFitness);
        
        for (int i = 0; i < POPULATION; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
        
        double bestFit = fitnessScores[0];
        double avgFit = 0;
        for (int i = 0; i < POPULATION; i++) {
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
        }
        avgFit /= POPULATION;
        
        printf("\nGeneration %d Statistics:", gen);
        printf("\n  Best Fitness: %.4f", bestFit);
        printf("\n  Average Fitness: %.4f\n", avgFit);
        
        if (bestFit == 1.0) {
            printf("\n*** PERFECT SOLUTION FOUND! ***\n");
            printf("Perfect chromosome: ");
            for (int i = 0; i < POPULATION; i++) {
                if (fitnessScores[i] == 1.0) {
                    printArray(population[i], SIZE);
                    printf("\n");
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
    
    char board[ROWS][COLS];
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    printBoard(board);
    
    int nQ, nR, nB, nK;
    while (1) {
        printf("\nEnter number of Queens (max 4): "); scanf("%d", &nQ);
        printf("Enter number of Rooks (max 4): "); scanf("%d", &nR);
        printf("Enter number of Bishops (max 4): "); scanf("%d", &nB);
        printf("Enter number of Knights (max 4): "); scanf("%d", &nK);
        int total = nQ + nR + nB + nK;
        if (nQ < 0 || nQ > 4 || nR < 0 || nR > 4 || nB < 0 || nB > 4 || nK < 0 || nK > 4) {
            printf("Each piece must be between 0 and 4.\n");
            continue;
        }
        if (total > 16) {
            printf("Total pieces cannot exceed 16. Currently: %d\n", total);
            continue;
        }
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
    printf("Enter Number of Generation\n");
    int numberofgen ;
    scanf("%d", &numberofgen);
    char chromosome[SIZE];
    int idx = 0;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];
    
    printf("\nInitial chromosome from board: ");
    printArray(chromosome, SIZE);
    printf("\n");
    
    char population[POPULATION][SIZE];
    double fitnessScores[POPULATION];
    
    printf("\n=== INITIAL POPULATION CREATION ===\n");
    for (int i = 0; i < POPULATION; i++) {
        printf("\nCreating chromosome %d:\n", i);
        printf("  Original: ");
        printArray(chromosome, SIZE);
        
        for (int j = 0; j < SIZE; j++)
            population[i][j] = chromosome[j];
        shuffle(population[i]);
        
        printf("\n  After shuffle: ");
        printArray(population[i], SIZE);
        
        fitnessScores[i] = fitness(population[i]);
        printf("\n  Fitness: %.4f\n", fitnessScores[i]);
    }
    
    printf("\n=== INITIAL POPULATION ===\n");
    printPopulation(population, fitnessScores, POPULATION, "Initial Population");
    
    char selected[6][SIZE];
    double selectedFitness[6];
    char finalPopulation[POPULATION][SIZE];
    double finalFitness[POPULATION];

    printf("\n\n=== GENETIC ALGORITHM STEPS ===\n");
    
    printf("\n\n=== STEP 1: TOURNAMENT SELECTION ===\n");
    double initialFitnessCopy[POPULATION];
    for(int i = 0; i < POPULATION; i++) initialFitnessCopy[i] = fitnessScores[i];
    tournamentSelection(population, initialFitnessCopy, selected, selectedFitness);
    
    printf("\n\n=== STEP 2: CROSSOVER ===\n");
    crossover(selected, selectedFitness, finalPopulation, finalFitness);
    
    printf("\n\n=== STEP 3: MUTATION ===\n");
    mutation(finalPopulation, finalFitness, nQ, nR, nB, nK);
    
    printf("\n\n=== STEP 4: REPLACEMENT ===\n");
    char best10[POPULATION][SIZE];
    double best10Fitness[POPULATION];
    replacement(population, fitnessScores, finalPopulation, finalFitness, best10, best10Fitness);
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(population[i], best10[i]);
        fitnessScores[i] = best10Fitness[i];
    }
    
    printf("\n\n=== POPULATION AFTER ONE COMPLETE CYCLE ===\n");
    printPopulation(population, fitnessScores, POPULATION, "Population after one cycle");

    printf("\n\n=== STARTING EVOLUTION LOOP FOR %d GENERATIONS ===\n", numberofgen);
    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, numberofgen);
    
    printf("\n\n=== FINAL RESULTS ===\n");
    printPopulation(population, fitnessScores, POPULATION, "Final Population");
    
    double bestFit = fitnessScores[0];
    int bestIdx = 0;
    for (int i = 1; i < POPULATION; i++) {
        if (fitnessScores[i] > bestFit) {
            bestFit = fitnessScores[i];
            bestIdx = i;
        }
    }
    
    printf("\nBest Solution Found:\n");
    printf("Chromosome: ");
    printArray(population[bestIdx], SIZE);
    printf("\nFitness: %.4f\n", bestFit);
    
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
    
    return 0;
}