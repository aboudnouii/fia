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

/* ===================== DEBUG PRINT HELPERS ===================== */

void printPopulation(char population[][SIZE], double fitnessScores[], int n, const char *title) {
    printf("\n===== %s =====\n", title);
    for (int i = 0; i < n; i++) {
        printf("Chromosome %d: ", i);
        printArray(population[i], SIZE);
        printf(" | Fitness = %.4f\n", fitnessScores[i]);
    }
}

void printSelected(char selected[][SIZE], double fitnessScores[], int n, const char *title) {
    printf("\n===== %s =====\n", title);
    for (int i = 0; i < n; i++) {
        printf("Selected %d: ", i);
        printArray(selected[i], SIZE);
        printf(" | Fitness = %.4f\n", fitnessScores[i]);
    }
}

/* =============================================================== */

void copyArray(char dest[], char src[]) {
    for (int i = 0; i < SIZE; i++)
        dest[i] = src[i];
}

void tournamentSelection(char population[][SIZE], double fitnessScores[],
                         char selected[6][SIZE], double selectedFitness[]) 
{
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < SIZE; j++)
            selected[i][j] = 'E';

    double tempFitnessScores[POPULATION];
    for (int k = 0; k < POPULATION; k++) {
        tempFitnessScores[k] = fitnessScores[k];
    }
    
    for (int s = 0; s < 6; s++) {
        int a, b;
        do { a = rand() % POPULATION; } while (tempFitnessScores[a] == -1.0);
        do { b = rand() % POPULATION; } while (b == a || tempFitnessScores[b] == -1.0);

        int winner = (tempFitnessScores[a] > tempFitnessScores[b]) ? a : b;
        copyArray(selected[s], population[winner]);
        selectedFitness[s] = tempFitnessScores[winner];
        tempFitnessScores[winner] = -1.0;
    }
}

void crossover(char selected[6][SIZE], double selectedFitness[],
               char finalPopulation[POPULATION][SIZE], double finalFitness[]) 
{
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

        for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p1][i];
        for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p2][i];
        tempFitness[nextChild++] = fitness(tempPopulation[nextChild - 1]);

        for (int i = 0; i < 8; i++) tempPopulation[nextChild][i] = selected[p2][i];
        for (int i = 8; i < SIZE; i++) tempPopulation[nextChild][i] = selected[p1][i];
        tempFitness[nextChild++] = fitness(tempPopulation[nextChild - 1]);
    }

    for (int i = 0; i < 4; i++) {
        copyArray(finalPopulation[i], tempPopulation[i]);
        finalFitness[i] = tempFitness[i];
    }
    for (int i = 0; i < 6; i++) {
        copyArray(finalPopulation[i + 4], tempPopulation[i + 6]);
        finalFitness[i + 4] = tempFitness[i + 6];
    }
}

void mutation(char population[][SIZE], double fitnessScores[],
              int nQ, int nR, int nB, int nK)
{
    char pieces[4] = {'Q', 'R', 'B', 'K'};
    int targets[4] = {nQ, nR, nB, nK};

    for (int c = 0; c < POPULATION; c++) {
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
    char combined[20][SIZE];
    double combinedFitness[20];

    for (int i = 0; i < POPULATION; i++) {
        copyArray(combined[i], oldPopulation[i]);
        combinedFitness[i] = oldFitness[i];
        copyArray(combined[i + POPULATION], newPopulation[i]);
        combinedFitness[i + POPULATION] = newFitness[i];
    }

    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19 - i; j++) {
            if (combinedFitness[j] < combinedFitness[j + 1]) {
                double tf = combinedFitness[j];
                combinedFitness[j] = combinedFitness[j + 1];
                combinedFitness[j + 1] = tf;

                char tc[SIZE];
                copyArray(tc, combined[j]);
                copyArray(combined[j], combined[j + 1]);
                copyArray(combined[j + 1], tc);
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
    for (int gen = 1; gen <= generations; gen++) {

        char selected[6][SIZE];
        double selectedFitness[6];
        char offspring[POPULATION][SIZE];
        double offspringFitness[POPULATION];
        char newPopulation[POPULATION][SIZE];
        double newFitness[POPULATION];

        tournamentSelection(population, fitnessScores, selected, selectedFitness);
        printSelected(selected, selectedFitness, 6, "After Tournament Selection");

        crossover(selected, selectedFitness, offspring, offspringFitness);
        printPopulation(offspring, offspringFitness, POPULATION, "After Crossover");

        mutation(offspring, offspringFitness, nQ, nR, nB, nK);
        printPopulation(offspring, offspringFitness, POPULATION, "After Mutation");

        replacement(population, fitnessScores, offspring, offspringFitness, newPopulation, newFitness);
        printPopulation(newPopulation, newFitness, POPULATION, "After Replacement");

        for (int i = 0; i < POPULATION; i++) {
            copyArray(population[i], newPopulation[i]);
            fitnessScores[i] = newFitness[i];
        }

        double bestFit = fitnessScores[0], avgFit = 0;
        for (int i = 0; i < POPULATION; i++) {
            if (fitnessScores[i] > bestFit) bestFit = fitnessScores[i];
            avgFit += fitnessScores[i];
        }
        avgFit /= POPULATION;

        printf("\nGeneration %d - Best: %.3f, Average: %.3f\n", gen, bestFit, avgFit);

        if (bestFit == 1.0) {
            printf("Perfect solution found!\n");
            break;
        }
    }
}

int main() {
    srand(time(NULL));

    char board[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            board[r][c] = 'E';

    printBoard(board);

    int nQ, nR, nB, nK;
    while (1) {
        printf("\nEnter number of Queens (max 4): "); scanf("%d", &nQ);
        printf("Enter number of Rooks (max 4): "); scanf("%d", &nR);
        printf("Enter number of Bishops (max 4): "); scanf("%d", &nB);
        printf("Enter number of Knights (max 4): "); scanf("%d", &nK);

        if (nQ < 0 || nQ > 4 || nR < 0 || nR > 4 || nB < 0 || nB > 4 || nK < 0 || nK > 4 ||
            nQ + nR + nB + nK > 16) {
            printf("Invalid input.\n");
            continue;
        }
        break;
    }

    int num = 1;
    for (int i = 0; i < nQ; i++) readPosition('Q', num++, board);
    num = 1;
    for (int i = 0; i < nR; i++) readPosition('R', num++, board);
    num = 1;
    for (int i = 0; i < nB; i++) readPosition('B', num++, board);
    num = 1;
    for (int i = 0; i < nK; i++) readPosition('K', num++, board);

    printBoard(board);

    char chromosome[SIZE];
    int idx = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            chromosome[idx++] = board[r][c];

    char population[POPULATION][SIZE];
    double fitnessScores[POPULATION];

    for (int i = 0; i < POPULATION; i++) {
        for (int j = 0; j < SIZE; j++)
            population[i][j] = chromosome[j];
        shuffle(population[i]);
        fitnessScores[i] = fitness(population[i]);
    }

    printPopulation(population, fitnessScores, POPULATION, "Initial Population");

    evolutionLoop(population, fitnessScores, nQ, nR, nB, nK, 10);
    return 0;
}
