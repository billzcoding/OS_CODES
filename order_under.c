#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <algorithm> // For std::sort

#define ARRAY_SIZE 16
#define SEGMENT_SIZE (ARRAY_SIZE / 4)

int main() {
    printf("--- TASK 1: Merge Sort Simulation with Ordered Output ---\n");
    
    // 1. Generate a static array of 16 integers.
    // Use fixed values for simplicity and verification.
    int arr[ARRAY_SIZE] = {38, 27, 43, 3, 9, 82, 10, 50, 15, 6, 22, 91, 7, 77, 4, 30};

    // Print initial array
    printf("Initial Array: ");
    for(int i=0; i < ARRAY_SIZE; i++) printf("%d ", arr[i]);
    printf("\n\n");

    omp_set_num_threads(4);
    
    // --- Run 1: Without ordered (Random Output Order) ---
    printf("--- RUN 1: Parallel Sorting WITHOUT ordered ---\n");
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        int start = i * SEGMENT_SIZE;
        // Sort the segment independently
        std::sort(arr + start, arr + start + SEGMENT_SIZE);

        // Printing without ordered: Output will be in random thread completion order
        printf("Segment %d sorted by Thread %d: ", i + 1, omp_get_thread_num());
        for(int j = 0; j < SEGMENT_SIZE; j++) {
            printf("%d ", arr[start + j]);
        }
        printf("\n");
    }

    // --- Run 2: With ordered (Sequential Output Order) ---
    printf("\n--- RUN 2: Parallel Sorting WITH ordered ---\n");
    
    // Re-initialize array for a clean run if needed, but sorting is generally in-place
    // We just rely on the independent sorting being done in the loop.

    #pragma omp parallel for ordered
    for (int i = 0; i < 4; i++) { // i represents the segment index (0 to 3)
        int start = i * SEGMENT_SIZE;
        
        // 1. Independent Parallel Work (Sorting)
        std::sort(arr + start, arr + start + SEGMENT_SIZE);

        // 2. Ordered Output (Printing)
        // The block inside #pragma omp ordered will execute sequentially (i=0, then i=1, etc.)
        #pragma omp ordered
        {
            printf("Segment %d sorted by Thread %d: ", i + 1, omp_get_thread_num());
            for(int j = 0; j < SEGMENT_SIZE; j++) {
                printf("%d ", arr[start + j]);
            }
            printf("\n");
        }
    }

    printf("\nExpected Output:\n");
    printf("Run 1 (Without ordered) shows segments printed in random order.\n");
    printf("Run 2 (With ordered) shows segments printed in correct order (Segment 1, 2, 3, 4).\n");

    return 0;
}


















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
















#include <stdio.h>
#include <omp.h>
#include <time.h> // For timing

#define N_T3 20 // Numbers 1 to 20

int main() {
    printf("--- TASK 3: Parallel Computation with Ordered Output ---\n");

    double start_w, end_w;
    omp_set_num_threads(4); 
    
    // --- Run 1: Without ordered ---
    printf("\n--- RUN 1: WITHOUT ordered (Random Output Order) ---\n");
    start_w = omp_get_wtime();

    #pragma omp parallel for
    for (int Y = 1; Y <= N_T3; Y++) { // Y is the loop variable (1 to 20)
        int Z = Y * Y; // Computation
        int thread_ID = omp_get_thread_num();
        
        // Output order is random
        printf("Thread %d computed square of %d = %d\n", thread_ID, Y, Z);
    }

    end_w = omp_get_wtime();
    printf("Time WITHOUT ordered: %f s\n", end_w - start_w);


    // --- Run 2: With ordered ---
    printf("\n--- RUN 2: WITH ordered (Sequential Output Order) ---\n");
    start_w = omp_get_wtime();

    #pragma omp parallel for ordered
    for (int Y = 1; Y <= N_T3; Y++) { // Y is the loop variable (1 to 20)
        int Z = Y * Y; // Computation
        int thread_ID = omp_get_thread_num();
        
        // The print block is serialized by the loop index Y
        #pragma omp ordered
        {
            printf("Thread %d computed square of %d = %d\n", thread_ID, Y, Z);
        }
    }

    end_w = omp_get_wtime();
    printf("Time WITH ordered: %f s\n", end_w - start_w);

    printf("\nExpected Output:\n");
    printf("Run 1 (Without ordered) output is random.\n");
    printf("Run 2 (With ordered) output follows numeric order (1, 2, 3, ... 20).\n");
    printf("Measure and discuss: The 'ordered' version will likely be slightly slower due to synchronization overhead.\n");

    return 0;
}




