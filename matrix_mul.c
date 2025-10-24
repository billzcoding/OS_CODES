#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Define the matrix size. N=500 is used to ensure a measurable time difference.
#define N 500

// Matrices must be declared globally (or statically) for large N to avoid stack overflow.
int A[N][N];
int B[N][N];
int C_seq[N][N];
int C_par[N][N];

int main() {
    double start_time, end_time;
    // Declare loop variables at the top of main for simplicity
    int i, j, k;

    printf("--- Final Matrix Multiplication Test (N=%d) ---\n", N);
    
    // =========================================================
    // 1. INITIALIZATION
    // =========================================================
    printf("Initializing matrices A and B...\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            A[i][j] = i + 1;
            B[i][j] = j + 1;
            C_seq[i][j] = 0;
            C_par[i][j] = 0;
        }
    }

    // =========================================================
    // 2. SEQUENTIAL EXECUTION
    // =========================================================
    printf("\nSequential Multiplication (Baseline)...\n");
    start_time = omp_get_wtime();
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            int sum = 0;
            for (k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C_seq[i][j] = sum;
        }
    }

    end_time = omp_get_wtime();
    printf("   Sequential Time: %.6f seconds\n", end_time - start_time);


    // =========================================================
    // 3. PARALLEL EXECUTION (Meets all constraints)
    // =========================================================
    printf("\nParallel Multiplications (2, 4, 8 Threads)...\n");
    // Enable nested parallelism, necessary for parallelizing the inner 'k' loop
    omp_set_nested(1); 

    int thread_counts[] = {2, 4, 8};
    for (int t = 0; t < 3; t++) {
        int threads = thread_counts[t];
        omp_set_num_threads(threads);

        // Reset C_par for the new run
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) { C_par[i][j] = 0; }
        }

        start_time = omp_get_wtime();

        // Requirement 1: #pragma omp parallel for to divide rows among threads (outer loop 'i')
        // Loop variable 'i' is automatically private.
        #pragma omp parallel for 
        for (i = 0; i < N; i++) {
            // The 'j' loop is executed sequentially by the thread assigned to row 'i', so 'j' is safe.
            for (j = 0; j < N; j++) {
                int sum = 0; // This 'sum' is initialized and used for C_par[i][j]
                
                // Requirement 2: Use reduction(+:sum) inside the inner loop ('k')
                // Loop variable 'k' is automatically private. The 'sum' variable is made private, 
                // and the final result is safely reduced (combined) at the end of the k-loop.
                #pragma omp parallel for reduction(+:sum)
                for (k = 0; k < N; k++) {
                    sum += A[i][k] * B[k][j];
                }
                
                C_par[i][j] = sum;
            }
        }

        end_time = omp_get_wtime();
        printf("   Parallel Time (%d Threads): %.6f seconds\n", threads, end_time - start_time);
    }
    
    // =========================================================
    // 4. VERIFICATION
    // =========================================================
    printf("\n--- Verification ---\n");
    if (C_seq[0][0] == C_par[0][0]) {
        printf("Verification successful! C[0][0] = %d\n", C_seq[0][0]);
    } else {
        printf("Verification FAILED. Sequential C[0][0]=%d, Parallel C_par[0][0]=%d\n", C_seq[0][0], C_par[0][0]);
    }

    printf("\n--- Test Complete ---\n");

    return 0;
}
