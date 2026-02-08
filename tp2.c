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

// Count conflicts for each piece
void Conflicts(char chrom[], int out[]) {
    // Initialize all to 0
    for (int i = 0; i < SIZE; i++) {
        out[i] = 0;
    }
    
    // Count conflicts
    for (int i = 0; i < SIZE; i++) {
        if (chrom[i] == 'E') continue;
        
        int r1 = i / COLS;
        int c1 = i % COLS;
        char p1 = chrom[i];
        
        // Only check pairs once (j > i)
        for (int j = i + 1; j < SIZE; j++) {
            if (chrom[j] == 'E') continue;
            
            int r2 = j / COLS;
            int c2 = j % COLS;
            char p2 = chrom[j];
            
            // Check if piece at i attacks piece at j
            int attack = 0;
            if (p1 == 'Q') {
                if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                    attack = 1;
            } else if (p1 == 'R') {
                if (r1 == r2 || c1 == c2)
                    attack = 1;
            } else if (p1 == 'B') {
                if (abs(r1 - r2) == abs(c1 - c2))
                    attack = 1;
            } else if (p1 == 'K') {
                if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                    (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                    attack = 1;
            }
            
            // Check if piece at j attacks piece at i
            if (!attack) {
                if (p2 == 'Q') {
                    if (r1 == r2 || c1 == c2 || abs(r1 - r2) == abs(c1 - c2))
                        attack = 1;
                } else if (p2 == 'R') {
                    if (r1 == r2 || c1 == c2)
                        attack = 1;
                } else if (p2 == 'B') {
                    if (abs(r1 - r2) == abs(c1 - c2))
                        attack = 1;
                } else if (p2 == 'K') {
                    if ((abs(r1 - r2) == 2 && abs(c1 - c2) == 1) ||
                        (abs(r1 - r2) == 1 && abs(c1 - c2) == 2))
                        attack = 1;
                }
            }
            
            if (attack) {
                out[i]++;
                out[j]++;
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
    // Divide by 2 because each conflict was counted twice (once for each piece)
    nb_conflicts /= 2;
    return 1.0 / (1.0 + nb_conflicts);
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

void copyArray(char dest[], char src[]) {
    for (int i = 0; i < SIZE; i++)
        dest[i] = src[i];
}

void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[6][SIZE], double selectedFitness[]) 
{
    // Initialize selected arrays
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < SIZE; j++) {
            selected[i][j] = 'E';
        }
        selectedFitness[i] = 0.0;
    }
    
    // Perform 6 tournaments
    for (int t = 0; t < 6; t++) {
        // Randomly select 2 different individuals
        int a, b;
        a = rand() % POPULATION;
        do {
            b = rand() % POPULATION;
        } while (b == a);
        
        // Select the fitter one
        int winner;
        if (fitnessScores[a] > fitnessScores[b]) {
            winner = a;
        } else {
            winner = b;
        }
        
        // Copy the winner
        copyArray(selected[t], population[winner]);
        selectedFitness[t] = fitnessScores[winner];
    }
}

/*void crossover(char selected[6][SIZE], double selectedFitness[],
               char finalPopulation[POPULATION][SIZE], double finalFitness[]) 
{
    // First 4 are the fittest from selected
    // Find the 4 fittest from selected
    int indices[6] = {0, 1, 2, 3, 4, 5};
    
    // Simple bubble sort to get indices of fittest
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5 - i; j++) {
            if (selectedFitness[indices[j]] < selectedFitness[indices[j + 1]]) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // First 4 in final population are the fittest from selected
    for (int i = 0; i < 4; i++) {
        copyArray(finalPopulation[i], selected[indices[i]]);
        finalFitness[i] = selectedFitness[indices[i]];
    }
    
    // Next 6 are children from crossover
    int childIndex = 4;
    for (int p = 0; p < 3 && childIndex < POPULATION; p++) {
        int p1 = p * 2;
        int p2 = p * 2 + 1;
        
        // First child: first half from p1, second half from p2
        if (childIndex < POPULATION) {
            for (int i = 0; i < 8; i++) 
                finalPopulation[childIndex][i] = selected[p1][i];
            for (int i = 8; i < SIZE; i++) 
                finalPopulation[childIndex][i] = selected[p2][i];
            finalFitness[childIndex] = fitness(finalPopulation[childIndex]);
            childIndex++;
        }
        
        // Second child: first half from p2, second half from p1
        if (childIndex < POPULATION) {
            for (int i = 0; i < 8; i++) 
                finalPopulation[childIndex][i] = selected[p2][i];
            for (int i = 8; i < SIZE; i++) 
                finalPopulation[childIndex][i] = selected[p1][i];
            finalFitness[childIndex] = fitness(finalPopulation[childIndex]);
            childIndex++;
        }
    }
}
*/
void crossover(char selected[6][SIZE], double selectedFitness[],
               char finalPopulation[POPULATION][SIZE], double finalFitness[]) 
{
    // First, let's put all selected individuals into the population
    for (int i = 0; i < 6; i++) {
        copyArray(finalPopulation[i], selected[i]);
        finalFitness[i] = selectedFitness[i];
    }
    
    // Now create 4 children from crossover to fill up to POPULATION (10)
    int childIndex = 6;
    
    // Create children using different parent pairs
    for (int p = 0; p < 2 && childIndex < POPULATION; p++) {
        int p1 = p * 2;
        int p2 = p * 2 + 1;
        
        // Child 1: first half from p1, second half from p2
        if (childIndex < POPULATION) {
            for (int i = 0; i < 8; i++) 
                finalPopulation[childIndex][i] = selected[p1][i];
            for (int i = 8; i < SIZE; i++) 
                finalPopulation[childIndex][i] = selected[p2][i];
            finalFitness[childIndex] = fitness(finalPopulation[childIndex]);
            childIndex++;
        }
        
        // Child 2: first half from p2, second half from p1
        if (childIndex < POPULATION) {
            for (int i = 0; i < 8; i++) 
                finalPopulation[childIndex][i] = selected[p2][i];
            for (int i = 8; i < SIZE; i++) 
                finalPopulation[childIndex][i] = selected[p1][i];
            finalFitness[childIndex] = fitness(finalPopulation[childIndex]);
            childIndex++;
        }
    }
    
    // If we still need more individuals, create random ones
    while (childIndex < POPULATION) {
        // Randomly select a parent to copy
        int parent = rand() % 6;
        copyArray(finalPopulation[childIndex], selected[parent]);
        
        // Apply some random swaps
        for (int swap = 0; swap < 3; swap++) {
            int i = rand() % SIZE;
            int j = rand() % SIZE;
            char temp = finalPopulation[childIndex][i];
            finalPopulation[childIndex][i] = finalPopulation[childIndex][j];
            finalPopulation[childIndex][j] = temp;
        }
        
        finalFitness[childIndex] = fitness(finalPopulation[childIndex]);
        childIndex++;
    }
}
void mutation(char population[][SIZE], double fitnessScores[],
              int nQ, int nR, int nB, int nK)
{
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};
    
    for (int c = 0; c < POPULATION; c++) {
        // First, count current pieces
        int counts[4] = {0, 0, 0, 0};
        for (int i = 0; i < SIZE; i++) {
            for (int p = 0; p < 4; p++) {
                if (population[c][i] == pieces[p]) {
                    counts[p]++;
                    break;
                }
            }
        }
        
        // Adjust counts to match targets
        for (int p = 0; p < 4; p++) {
            while (counts[p] < targets[p]) {
                // Need to add this piece type
                int pos;
                do {
                    pos = rand() % SIZE;
                } while (population[c][pos] != 'E');
                
                population[c][pos] = pieces[p];
                counts[p]++;
            }
            
            while (counts[p] > targets[p]) {
                // Need to remove this piece type
                int pos;
                do {
                    pos = rand() % SIZE;
                } while (population[c][pos] != pieces[p]);
                
                population[c][pos] = 'E';
                counts[p]--;
            }
        }
        
        // Recalculate fitness
        fitnessScores[c] = fitness(population[c]);
    }
}

void replacement(char oldPopulation[][SIZE], double oldFitness[],
                 char newPopulation[][SIZE], double newFitness[],
                 char resultPopulation[][SIZE], double resultFitness[]) 
{
    // Combine old and new populations
    char combined[2 * POPULATION][SIZE];
    double combinedFitness[2 * POPULATION];
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(combined[i], oldPopulation[i]);
        combinedFitness[i] = oldFitness[i];
        copyArray(combined[i + POPULATION], newPopulation[i]);
        combinedFitness[i + POPULATION] = newFitness[i];
    }
    
    // Sort combined population by fitness (descending)
    for (int i = 0; i < 2 * POPULATION - 1; i++) {
        for (int j = 0; j < 2 * POPULATION - i - 1; j++) {
            if (combinedFitness[j] < combinedFitness[j + 1]) {
                // Swap fitness
                double tempFit = combinedFitness[j];
                combinedFitness[j] = combinedFitness[j + 1];
                combinedFitness[j + 1] = tempFit;
                
                // Swap chromosomes
                char tempChrom[SIZE];
                copyArray(tempChrom, combined[j]);
                copyArray(combined[j], combined[j + 1]);
                copyArray(combined[j + 1], tempChrom);
            }
        }
    }
    
    // Take the POPULATION fittest individuals
    for (int i = 0; i < POPULATION; i++) {
        copyArray(resultPopulation[i], combined[i]);
        resultFitness[i] = combinedFitness[i];
    }
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], 
                   int nQ, int nR, int nB, int nK, int generations) 
{
    for (int gen = 1; gen <= generations; gen++) {
        // Selection
        char selected[6][SIZE];
        double selectedFitness[6];
        tournamentSelection(population, fitnessScores, selected, selectedFitness);
        
        // Crossover
        char offspring[POPULATION][SIZE];
        double offspringFitness[POPULATION];
        crossover(selected, selectedFitness, offspring, offspringFitness);
        
        // Mutation
        mutation(offspring, offspringFitness, nQ, nR, nB, nK);
        
        // Replacement
        char newPopulation[POPULATION][SIZE];
        double newFitness[POPULATION];
        replacement(population, fitnessScores, offspring, offspringFitness, 
                    newPopulation, newFitness);
        
        // Update population
        for (int i = 0; i < POPULATION; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
        
        // Calculate statistics
        double bestFit = fitnessScores[0];
        double avgFit = 0;
        for (int i = 0; i < POPULATION; i++) {
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
        }
        avgFit /= POPULATION;
        
        printf("\nGeneration %d - Best: %.3f, Average: %.3f\n", gen, bestFit, avgFit);
        
        // Check for perfect solution
        if (bestFit == 1.0) {
            printf("Perfect solution found!\n");
            // Print the best chromosome
            printf("Best solution: ");
            printArray(population[0], SIZE);
            printf("\n");
            break;
        }
        
        // Also print best solution every 5 generations
        if (gen % 5 == 0) {
            printf("Best solution so far: ");
            printArray(population[0], SIZE);
            printf(" (Fitness: %.3f)\n", bestFit);
        }
    }
}

int main() {
    srand(time(NULL));
    
    // Initialize board
    char board[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    printBoard(board);
    
    // Get piece counts from user
    int nQ, nR, nB, nK;
    while (1) {
        printf("\nEnter number of Queens (max 4): "); 
        scanf("%d", &nQ);
        printf("Enter number of Rooks (max 4): "); 
        scanf("%d", &nR);
        printf("Enter number of Bishops (max 4): "); 
        scanf("%d", &nB);
        printf("Enter number of Knights (max 4): "); 
        scanf("%d", &nK);
        
        int total = nQ + nR + nB + nK;
        if (nQ < 0 || nQ > 4 || nR < 0 || nR > 4 || 
            nB < 0 || nB > 4 || nK < 0 || nK > 4) {
            printf("Each piece must be between 0 and 4.\n");
            continue;
        }
        if (total > 16) {
            printf("Total pieces cannot exceed 16. Currently: %d\n", total);
            continue;
        }
        break;
    }
    
    // Read positions for each piece
    int pieceNum = 1;
    for (int i = 0; i < nQ; i++) readPosition('Q', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nR; i++) readPosition('R', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nB; i++) readPosition('B', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nK; i++) readPosition('K', pieceNum++, board);
    
    printBoard(board);
    
    // Convert board to chromosome
    char chromosome[SIZE];
    int idx = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];
    
    // Initialize population
    char population[POPULATION][SIZE];
    double fitnessScores[POPULATION];
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(population[i], chromosome);
        shuffle(population[i]);
        fitnessScores[i] = fitness(population[i]);
    }
    
    // Run evolution
    printf("\nStarting evolutionary algorithm...\n");
    printf("Initial best fitness: %.3f\n", fitnessScores[0]);
    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, 10);
    
    // Print final best solution
    printf("\nFinal best solution:\n");
    printArray(population[0], SIZE);
    printf("\nFitness: %.3f\n", fitnessScores[0]);
    
    // Also display as board
    printf("\nBoard representation:\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%c ", population[0][i]);
        if ((i + 1) % COLS == 0) printf("\n");
    }
    
    return 0;
}