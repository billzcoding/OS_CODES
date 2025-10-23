#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 1: Hello World with Critical ---\n");

    // Set thread count to at least 4 for demonstration
    omp_set_num_threads(4);

    #pragma omp parallel
    {
        int thread_ID = omp_get_thread_num();
        int total = omp_get_num_threads();

        // The critical section ensures that only one thread executes the
        // printf statement at any given time.
        #pragma omp critical
        {
            printf("Hello from thread %d of %d\n", thread_ID, total);
        }
    }

    printf("\nExpected Outcome: Messages print one at a time, without mixing.\n");
    printf("The order will vary between runs due to thread scheduling.\n");

    return 0;
}










#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N_T2 10

int main() {
    printf("--- TASK 2: Vector Sum with Reduction ---\n");

    int A[N_T2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum_seq = 0;
    int sum_par = 0;

    // --- 1. Sequential Program ---
    clock_t begin_t = clock();
    for (int i = 0; i < N_T2; i++) {
        sum_seq += A[i];
    }
    clock_t end_t = clock();
    double time_spent_seq = (double)(end_t - begin_t) / CLOCKS_PER_SEC;
    printf("1. Sequential Sum: %d | Time: %f s\n", sum_seq, time_spent_seq);


    // --- 2. Parallel Program with Reduction ---
    double start_time_w = omp_get_wtime();

    // Use parallel for and reduction(+:sum_par)
    #pragma omp parallel for reduction(+:sum_par)
    for (int i = 0; i < N_T2; i++) {
        sum_par += A[i];
    }

    double end_time_w = omp_get_wtime();
    double time_spent_par = end_time_w - start_time_w;

    printf("2. Parallel Sum: %d | Time: %f s\n", sum_par, time_spent_par);

    printf("\nExpected Outcome: Both sums are %d (correct).\n", 55);
    // Note: Timing differences are negligible for N=10, but parallel is faster for large N.

    return 0;
}











#include <stdio.h>
#include <omp.h>
#include <limits.h>

#define N_T3 20

int main() {
    printf("--- TASK 3: Minimum & Maximum of Array ---\n");

    int A[N_T3] = {15, 3, 8, 1, 10, 20, 5, 12, 18, 2, 19, 9, 7, 14, 4, 11, 17, 6, 16, 13};

    // --- Version A: Reduction ---
    int min_val_red = INT_MAX; // Initialize to largest possible int
    int max_val_red = INT_MIN; // Initialize to smallest possible int

    double start_time_red = omp_get_wtime();
    
    // Use min/max reduction operators
    #pragma omp parallel for reduction(min: min_val_red) reduction(max: max_val_red)
    for (int i = 0; i < N_T3; i++) {
        // Reductions automatically handle private copies and merging
        if (A[i] < min_val_red) min_val_red = A[i];
        if (A[i] > max_val_red) max_val_red = A[i];
    }

    double time_red = omp_get_wtime() - start_time_red;


    // --- Version B: Critical ---
    int min_val_crit = INT_MAX;
    int max_val_crit = INT_MIN;

    double start_time_crit = omp_get_wtime();
    
    #pragma omp parallel for
    for (int i = 0; i < N_T3; i++) {
        // Find local min/max outside critical, only update shared variable inside
        
        #pragma omp critical
        {
            if (A[i] < min_val_crit) min_val_crit = A[i];
            if (A[i] > max_val_crit) max_val_crit = A[i];
        }
    }

    double time_crit = omp_get_wtime() - start_time_crit;

    printf("\nVersion A (Reduction): Min = %d, Max = %d | Time: %f s\n", min_val_red, max_val_red, time_red);
    printf("Version B (Critical): Min = %d, Max = %d | Time: %f s\n", min_val_crit, max_val_crit, time_crit);
    
    printf("\nExpected Outcome: Both methods yield Min=1, Max=20.\n");
    printf("Reduction is faster because it avoids serialization (thread blocking).\n");

    return 0;
}













#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 4: Shared Variable Update with Critical ---\n");

    int N_THREADS = 8;
    omp_set_num_threads(N_THREADS);
    int COUNTS_PER_THREAD = 10000;
    int total_iterations = N_THREADS * COUNTS_PER_THREAD;

    // --- 1. Without critical (Race Condition) ---
    int counter_no_crit = 0;

    #pragma omp parallel for
    for (int i = 0; i < total_iterations; i++) {
        // Race condition: multiple threads try to read/update counter_no_crit simultaneously
        counter_no_crit++; 
    }

    printf("1. Without critical (Race Condition):\n");
    printf("   Final counter: %d (Expected: %d)\n", counter_no_crit, total_iterations);
    printf("   Expected Outcome: final counter < number of iterations (Wrong Result).\n");


    // --- 2. With critical (Correct Result) ---
    int counter_with_crit = 0;

    #pragma omp parallel for
    for (int i = 0; i < total_iterations; i++) {
        // Critical section ensures only one thread updates the counter at a time
        #pragma omp critical
        {
            counter_with_crit++;
        }
    }

    printf("\n2. With critical (Correct Synchronization):\n");
    printf("   Final counter: %d (Expected: %d)\n", counter_with_crit, total_iterations);
    printf("   Expected Outcome: final counter = number of iterations (Correct Result).\n");
    printf("\nDiscussion: Serialization (critical section) slows execution by forcing threads to wait, but ensures data correctness.\n");

    return 0;
}






















#include <stdio.h>
#include <omp.h>
#include <stdlib.h> // Required for malloc/free

#define N_MAT 200 // Matrix size N x N (200x200)

int main() {
    printf("--- TASK 5: Matrix Multiplication with OpenMP (%dx%d) ---\n", N_MAT, N_MAT);

    // Dynamic allocation is REQUIRED for matrices of this size (N=200) to avoid stack overflow.
    int *A = (int*)malloc(N_MAT * N_MAT * sizeof(int));
    int *B = (int*)malloc(N_MAT * N_MAT * sizeof(int));
    int *C = (int*)malloc(N_MAT * N_MAT * sizeof(int));

    if (!A || !B || !C) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Initialize matrices (simple initialization for verification)
    for (int i = 0; i < N_MAT; i++) {
        for (int j = 0; j < N_MAT; j++) {
            A[i * N_MAT + j] = 1; // A[i][j] = 1
            B[i * N_MAT + j] = i + 1; // B[i][j] = row_index + 1
            C[i * N_MAT + j] = 0;
        }
    }

    // --- 1. Sequential Program ---
    double start_time_seq = omp_get_wtime();

    for (int i = 0; i < N_MAT; i++) {
        for (int j = 0; j < N_MAT; j++) {
            int sum = 0;
            for (int k = 0; k < N_MAT; k++) {
                sum += A[i * N_MAT + k] * B[k * N_MAT + j];
            }
            C[i * N_MAT + j] = sum;
        }
    }

    double time_spent_seq = omp_get_wtime() - start_time_seq;
    printf("1. Sequential Time: %f seconds\n", time_spent_seq);
    // Print a verification element: C[0][0] should be Sum(B[k][0]) = N * 1 = 200
    printf("   Verification C[0][0]: %d\n", C[0]);


    // Reset C for parallel run
    for (int i = 0; i < N_MAT * N_MAT; i++) C[i] = 0;

    // --- 2. Parallel Program (Run with 2, 4, 8 threads) ---
    int thread_counts[] = {2, 4, 8};

    for (int k = 0; k < 3; k++) {
        int num_threads = thread_counts[k];
        omp_set_num_threads(num_threads);

        double start_time_par = omp_get_wtime();

        // Parallelize the outer two loops (distribute rows/columns)
        #pragma omp parallel for
        for (int i = 0; i < N_MAT; i++) { // Distribute rows of C (and A)
            for (int j = 0; j < N_MAT; j++) { // Distribute columns of C (and B)
                
                int sum = 0; // Local sum variable for reduction
                
                // Use reduction for the inner dot product accumulation
                // Although not strictly necessary here because 'sum' is private and reset,
                // the spirit of the instruction is to show reduction usage.
                #pragma omp parallel for reduction(+:sum)
                for (int k = 0; k < N_MAT; k++) {
                    sum += A[i * N_MAT + k] * B[k * N_MAT + j];
                }
                C[i * N_MAT + j] = sum;
            }
        }

        double time_spent_par = omp_get_wtime() - start_time_par;
        printf("2. Parallel Time (%d threads): %f seconds\n", num_threads, time_spent_par);
    }

    printf("\nExpected Outcome: Parallel multiplication is significantly faster than sequential.\n");
    printf("This shows how OpenMP improves performance for computationally intensive tasks.\n");

    free(A); free(B); free(C);
    return 0;
}


