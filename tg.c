#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

const int SIZE = 16;
const int ROWS = 4;
const int COLS = 4;
const int POPULATION = 10;

// GA Parameters from the image
const double CROSSOVER_PROB = 0.8;
const double MUTATION_PROB = 0.1;

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

// Counts the number of pieces that are currently threatened by others
int countThreatenedPieces(char chrom[]) {
    int is_threatened[SIZE];
    for (int i = 0; i < SIZE; i++) is_threatened[i] = 0;

    for (int i = 0; i < SIZE; i++) { // i is the Attacker
        if (chrom[i] == 'E') continue;
        
        int r1 = i / COLS;
        int c1 = i % COLS;
        char p1 = chrom[i];

        for (int j = 0; j < SIZE; j++) { // j is the Potential Victim
            if (i == j || chrom[j] == 'E') continue;

            int r2 = j / COLS;
            int c2 = j % COLS;
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

            if (attack) {
                is_threatened[j] = 1; // Mark piece j as threatened
            }
        }
    }

    int total_threatened = 0;
    for (int k = 0; k < SIZE; k++) {
        total_threatened += is_threatened[k];
    }
    return total_threatened;
}

double fitness(char chrom[]) {
    // 1. Calculate nb_conflicts (Number of threatened pieces)
    int nb_conflicts = countThreatenedPieces(chrom);

    // 2. Calculate Penalty (Columns containing more than 1 Queen)
    int penalty = 0;
    for (int c = 0; c < COLS; c++) {
        int queensInCol = 0;
        for (int r = 0; r < ROWS; r++) {
            if (chrom[r * COLS + c] == 'Q') {
                queensInCol++;
            }
        }
        if (queensInCol > 1) {
            penalty++; // Penalty adds 1 for every column with >1 Queen
        }
    }

    // Equation (2): F = 1 / (1 + nb_conflicts + penalty)
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
    // Simple logic for brevity: Just picking random tournaments
    double tempFitnessScores[POPULATION];
    for (int k = 0; k < POPULATION; k++) tempFitnessScores[k] = fitnessScores[k];
    
    for (int s = 0; s < 6; s++) {
        int a = rand() % POPULATION;
        int b = rand() % POPULATION;
        // Basic unique check
        while(b == a) b = rand() % POPULATION;

        int winner;
        if (tempFitnessScores[a] > tempFitnessScores[b]) winner = a;
        else winner = b;

        copyArray(selected[s], population[winner]);
        selectedFitness[s] = tempFitnessScores[winner];
    }
}

// Modified Crossover with Probability Pc = 0.8
void crossover(char selected[6][SIZE], double selectedFitness[],
               char finalPopulation[POPULATION][SIZE], double finalFitness[]) 
{
    char tempPopulation[12][SIZE];
    double tempFitness[12];

    // Keep parents just in case
    for (int i = 0; i < 6; i++) {
        copyArray(tempPopulation[i], selected[i]);
        tempFitness[i] = selectedFitness[i];
    }
    
    int nextChild = 6;
    for (int p = 0; p < 3; p++) {
        int p1 = p * 2;
        int p2 = p * 2 + 1;
        
        // Generate random number [0, 1]
        double r = (double)rand() / RAND_MAX;

        if (r <= CROSSOVER_PROB) {
            // Perform Crossover
            for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p1][i];
            for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p2][i];
            nextChild++;
            
            for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p2][i];
            for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p1][i];
            nextChild++;
        } else {
            // No Crossover - Copy Parents directly
            copyArray(tempPopulation[nextChild], selected[p1]);
            nextChild++;
            copyArray(tempPopulation[nextChild], selected[p2]);
            nextChild++;
        }
    }

    // Fill final population (take 4 parents + 6 children to fill 10)
    for (int i = 0; i < 4; i++) {
        copyArray(finalPopulation[i], tempPopulation[i]);
    }
    for (int i = 0; i < 6; i++) {
        copyArray(finalPopulation[i + 4], tempPopulation[i + 6]);
    }
}

// This function ensures the chromosome has the exact number of pieces required.
// Crossover can destroy counts (e.g., resulting in 5 Queens), so this repairs it.
void repairCounts(char chrom[], int nQ, int nR, int nB, int nK) {
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};

    for (int p = 0; p < 4; p++) {
        int count = 0;
        for (int i = 0; i < SIZE; i++) {
            if (chrom[i] == pieces[p]) count++;
        }

        // Add if missing
        while (count < targets[p]) {
            int pos = rand() % SIZE;
            if (chrom[pos] == 'E') {
                chrom[pos] = pieces[p];
                count++;
            }
        }
        
        // Remove if too many
        while (count > targets[p]) {
            int pos = rand() % SIZE;
            if (chrom[pos] == pieces[p]) {
                chrom[pos] = 'E';
                count--;
            }
        }
    }
}

// Probabilistic Mutation (Swap) with Pm = 0.1
void randomMutation(char population[][SIZE], int nQ, int nR, int nB, int nK) {
    for (int c = 0; c < POPULATION; c++) {
        // 1. First, repair counts messed up by crossover
        repairCounts(population[c], nQ, nR, nB, nK);

        // 2. Apply Probabilistic Mutation (Swap)
        double r = (double)rand() / RAND_MAX;
        
        if (r <= MUTATION_PROB) {
            // Swap two random positions
            int idx1 = rand() % SIZE;
            int idx2 = rand() % SIZE;
            
            char temp = population[c][idx1];
            population[c][idx1] = population[c][idx2];
            population[c][idx2] = temp;
        }
    }
}

void replacement(char oldPopulation[][SIZE], double oldFitness[],
                 char newPopulation[][SIZE], double newFitness[],
                 char resultPopulation[][SIZE], double resultFitness[]) 
{
    // Elitism: Combine and pick best 10
    char combined[20][SIZE];
    double combinedFitness[20];
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(combined[i], oldPopulation[i]);
        combinedFitness[i] = oldFitness[i];
        copyArray(combined[i + POPULATION], newPopulation[i]);
        combinedFitness[i + POPULATION] = newFitness[i];
    }
    
    // Sort descending
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
    
    for (int i = 0; i < POPULATION; i++) {
        copyArray(resultPopulation[i], combined[i]);
        resultFitness[i] = combinedFitness[i];
    }
}

void evolutionLoop(char population[][SIZE], double fitnessScores[], 
                   int nQ, int nR, int nB, int nK, int generations) 
{
    printf("\n=== EVOLUTION LOOP START (Gen: %d, Pc: %.1f, Pm: %.1f) ===\n", generations, CROSSOVER_PROB, MUTATION_PROB);
    
    for (int gen = 1; gen <= generations; gen++) {
        
        double currentFitness[POPULATION];
        for (int i = 0; i < POPULATION; i++) currentFitness[i] = fitnessScores[i];

        char selected[6][SIZE];
        double selectedFitness[6];
        char offspring[POPULATION][SIZE];
        double offspringFitness[POPULATION];
        char newPopulation[POPULATION][SIZE];
        double newFitness[POPULATION];
        
        // 1. Selection
        tournamentSelection(population, currentFitness, selected, selectedFitness);
        
        // 2. Crossover (Probabilistic)
        crossover(selected, selectedFitness, offspring, offspringFitness);
        
        // 3. Mutation (Repair + Probabilistic Swap)
        randomMutation(offspring, nQ, nR, nB, nK);

        // Recalculate fitness for offspring
        for(int i=0; i<POPULATION; i++) {
            offspringFitness[i] = fitness(offspring[i]);
        }
        
        // 4. Replacement
        replacement(population, fitnessScores, offspring, offspringFitness, 
                    newPopulation, newFitness);
        
        for (int i = 0; i < POPULATION; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }
        
        double bestFit = fitnessScores[0];
        printf("Generation %d Best Fitness: %.4f\n", gen, bestFit);
        
        if (bestFit >= 0.99) { // Close to 1 (Optimal)
             printf("\n*** OPTIMAL SOLUTION FOUND! ***\n");
             break;
        }
    }
}

int main() {
    srand(time(NULL));
    
    printf("=== CHESS PIECE PLACEMENT GA (Modified) ===\n");
    
    char board[ROWS][COLS];
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            board[r][c] = 'E';
    
    printBoard(board);
    
    int nQ, nR, nB, nK;
    printf("\nEnter number of Queens: "); scanf("%d", &nQ);
    printf("Enter number of Rooks: "); scanf("%d", &nR);
    printf("Enter number of Bishops: "); scanf("%d", &nB);
    printf("Enter number of Knights: "); scanf("%d", &nK);
    
    int pieceNum = 1;
    for (int i = 0; i < nQ; i++) readPosition('Q', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nR; i++) readPosition('R', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nB; i++) readPosition('B', pieceNum++, board);
    pieceNum = 1;
    for (int i = 0; i < nK; i++) readPosition('K', pieceNum++, board);
    
    int numberofgen;
    printf("Enter Number of Generations: ");
    scanf("%d", &numberofgen);

    char chromosome[SIZE];
    int idx = 0;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];
    
    char population[POPULATION][SIZE];
    double fitnessScores[POPULATION];
    
    // Initialize Population
    for (int i = 0; i < POPULATION; i++) {
        for (int j = 0; j < SIZE; j++) population[i][j] = chromosome[j];
        shuffle(population[i]);
        fitnessScores[i] = fitness(population[i]);
    }
    
    // Run Evolution
    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, numberofgen);
    
    // Result
    int bestIdx = 0;
    for (int i = 1; i < POPULATION; i++) {
        if (fitnessScores[i] > fitnessScores[bestIdx]) bestIdx = i;
    }
    
    printf("\nBest Solution Found:\n");
    printArray(population[bestIdx], SIZE);
    printf("\nFitness: %.4f\n", fitnessScores[bestIdx]);
    
    printf("\nBoard representation:\n");
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