#include <stdio.h>
#include <omp.h>
#include <time.h> // For timing

#define N_MAT 4 // Size 4x4

int main() {
    printf("--- TASK 2: Matrix Multiplication with and without ordered ---\n");

    int A[N_MAT][N_MAT] = {
        {1, 2, 3, 4},
        {4, 3, 2, 1},
        {5, 6, 7, 8},
        {8, 7, 6, 5}
    };
    int B[N_MAT][N_MAT] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };
    int C[N_MAT][N_MAT]; // Result matrix

    omp_set_num_threads(4);
    double start_w, end_w;
    
    // --- Run 1: Without ordered ---
    printf("\n--- RUN 1: WITHOUT ordered (Random Output Order) ---\n");
    start_w = omp_get_wtime();

    // Parallelize the outer loop (distribute rows)
    #pragma omp parallel for
    for (int i = 0; i < N_MAT; i++) {
        for (int j = 0; j < N_MAT; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N_MAT; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
            // Print: Order is non-deterministic (random)
            printf("C[%d][%d] = %d\n", i, j, C[i][j]);
        }
    }

    end_w = omp_get_wtime();
    printf("Time WITHOUT ordered: %f s\n", end_w - start_w);


    // --- Run 2: With ordered ---
    // Reset C for a clean run, though not strictly required if using new matrices
    for (int i = 0; i < N_MAT; i++) {
        for (int j = 0; j < N_MAT; j++) {
            C[i][j] = 0;
        }
    }

    printf("\n--- RUN 2: WITH ordered (Sequential Output Order) ---\n");
    start_w = omp_get_wtime();

    // Use nested parallel for to distribute work over i and j
    // The ordered clause must be on the 'for' loop whose index (i) we want to enforce order on.
    #pragma omp parallel for ordered(2) // ordered(2) specifies ordering based on the two loop variables i and j
    for (int i = 0; i < N_MAT; i++) {
        for (int j = 0; j < N_MAT; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N_MAT; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
            
            // The print block executes in increasing order of (i, j)
            #pragma omp ordered
            {
                printf("C[%d][%d] = %d\n", i, j, C[i][j]);
            }
        }
    }

    end_w = omp_get_wtime();
    printf("Time WITH ordered: %f s\n", end_w - start_w);

    printf("\nExpected Output:\n");
    printf("Run 1 (Without ordered) shows results in random thread completion order.\n");
    printf("Run 2 (With ordered) prints elements strictly in row-major order (0,0 -> 0,1 ... -> 3,3).\n");

    return 0;
}