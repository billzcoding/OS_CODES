#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

int main() {
    int thread_counts[] = {2, 4, 8};
    clock_t begin_t, end_t;
    double time_spent;

    printf("--- TASK 1: Hello World with Threads ---\n");

    for (int k = 0; k < 3; k++) {
        int num_threads = thread_counts[k];
        omp_set_num_threads(num_threads);

        begin_t = clock();

        #pragma omp parallel
        {
            int thread_ID = omp_get_thread_num();
            int num_threads_actual = omp_get_num_threads();
            printf("Hello from thread %d of %d\n", thread_ID, num_threads_actual);
        }

        end_t = clock();
        time_spent = (double)(end_t - begin_t) / CLOCKS_PER_SEC;

        printf("\nTime for %d threads (clock()): %f seconds\n", num_threads, time_spent);
        printf("Note: Prints are unordered.\n\n");
    }
    return 0;
}








#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N_T2 10

int main() {
    printf("--- TASK 2: Array Summation without Work-Sharing ---\n");

    int A[N_T2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int B[N_T2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int C[N_T2];

    clock_t begin_t, end_t;
    double time_spent;

    // Set a thread count for a clear demo of duplication
    omp_set_num_threads(4); 

    begin_t = clock();

    #pragma omp parallel
    {
        int thread_ID = omp_get_thread_num();

        // Loop inside parallel region WITHOUT #pragma omp for
        for (int i = 0; i < N_T2; i++) {
            C[i] = A[i] + B[i];
        }
        printf("Thread %d executed the entire loop (Duplicated Work).\n", thread_ID);
    }

    end_t = clock();
    time_spent = (double)(end_t - begin_t) / CLOCKS_PER_SEC;

    printf("\nResult array C: ");
    for (int j = 0; j < N_T2; j++) {
        printf("%d ", C[j]);
    }
    printf("\nExecution Time (clock()): %f seconds\n", time_spent);
    printf("Homework: Use **#pragma omp for** to fix duplication.\n");

    return 0;
}








#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

#define N_T3 1000000

int main() {
    printf("--- TASK 3: Array Summation with Work-Sharing (for) ---\n");

    // Dynamic allocation is used for N=1,000,000 as large static arrays cause stack overflow.
    int *A = (int*)malloc(N_T3 * sizeof(int));
    int *B = (int*)malloc(N_T3 * sizeof(int));
    int *C = (int*)malloc(N_T3 * sizeof(int));
    if (!A || !B || !C) return 1;

    for (int i = 0; i < N_T3; i++) {
        A[i] = i;
        B[i] = i * 2;
    }

    // 1. Sequential Timing (clock())
    clock_t begin_t = clock();
    for (int i = 0; i < N_T3; i++) {
        C[i] = A[i] + B[i];
    }
    clock_t end_t = clock();
    double time_spent_seq = (double)(end_t - begin_t) / CLOCKS_PER_SEC;
    printf("1. Sequential Time (clock()): %f seconds\n", time_spent_seq);


    // 2. Parallel Timing (omp_get_wtime())
    int thread_counts[] = {2, 4, 0}; // 0 = default

    for (int k = 0; k < 3; k++) {
        if (thread_counts[k] > 0) {
            omp_set_num_threads(thread_counts[k]);
        }

        double start_time_w = omp_get_wtime();

        #pragma omp parallel
        {
            int thread_ID = omp_get_thread_num();

            // Correct work distribution
            #pragma omp for
            for (int i = 0; i < N_T3; i++) {
                C[i] = A[i] + B[i];

                // Print for a few elements
                if (i < 5 || i > N_T3 - 5) {
                    printf("Thread %d computed C[%d]\n", thread_ID, i);
                }
            }
        }

        double end_time_w = omp_get_wtime();
        double time_spent_par = end_time_w - start_time_w;

        printf("2. Parallel Time (%s threads, omp_get_wtime()): %f seconds\n",
               thread_counts[k] > 0 ? "Specified" : "Default", time_spent_par);
    }
    
    printf("Note: Performance should improve (time_spent_par < time_spent_seq).\n");
    free(A); free(B); free(C);
    return 0;
}




     


#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N_T4 1000000

int main() {
    printf("--- TASK 4: Reduction: Parallel Sum of 1 to %d ---\n", N_T4);

    long long sum_seq = 0;
    long long sum_par = 0;

    // 1. Sequential Version (clock())
    clock_t begin_t = clock();
    for (int i = 1; i <= N_T4; i++) {
        sum_seq += i;
    }
    clock_t end_t = clock();
    double time_spent_seq = (double)(end_t - begin_t) / CLOCKS_PER_SEC;

    // 2. Parallel Version with Reduction (omp_get_wtime())
    double start_time_w = omp_get_wtime();

    // Reduction clause for safe parallel summation
    #pragma omp parallel for reduction(+:sum_par)
    for (int i = 1; i <= N_T4; i++) {
        sum_par += i;
    }

    double end_time_w = omp_get_wtime();
    double time_spent_par = end_time_w - start_time_w;

    printf("1. Sequential Sum: %lld | Time (clock()): %f seconds\n", sum_seq, time_spent_seq);
    printf("2. Parallel Sum (Reduction): %lld | Time (omp_get_wtime()): %f seconds\n", sum_par, time_spent_par);
    printf("Expected Result: Both sums are the same. Parallel should be faster.\n");
    
    return 0;
}












#include <stdio.h>
#include <omp.h>
#include <time.h>

#define N_T5_SMALL 1000 // Small static array size for simplicity

int main() {
    printf("--- TASK 5: Sections Construct: Independent Work Distribution ---\n");

    // Small static arrays for simplicity
    int A[N_T5_SMALL];
    int B[N_T5_SMALL];
    int C[N_T5_SMALL];
    
    // Initialize arrays
    for (int i = 0; i < N_T5_SMALL; i++) {
        A[i] = 1; // Simple sum
        B[i] = 2; // Simple product
        C[i] = i;  // Simple max
    }

    long long sum_res = 0, product_res = 1;
    int max_res = 0;
    clock_t begin_t, end_t;
    double start_time_w, end_time_w;


    // --- 1. Sequential Version ---
    printf("\n--- 1. Sequential Run (clock()) ---\n");
    begin_t = clock();
    {
        // Section 1: Sum
        for (int i = 0; i < N_T5_SMALL; i++) sum_res += A[i];
        
        // Section 2: Product
        for (int i = 0; i < 5; i++) product_res *= B[i];

        // Section 3: Max
        max_res = C[0];
        for (int i = 1; i < N_T5_SMALL; i++) if (C[i] > max_res) max_res = C[i];
    }
    end_t = clock();
    double time_spent_seq = (double)(end_t - begin_t) / CLOCKS_PER_SEC;
    printf("Time (clock()): %f seconds\n", time_spent_seq);
    printf("Results: Sum=%lld, Product=%lld, Max=%d\n", sum_res, product_res, max_res);

    // Reset results
    sum_res = 0; product_res = 1; max_res = 0;

    // --- 2. Parallel Version with #pragma omp sections ---
    printf("\n--- 2. Parallel Run (omp_get_wtime()) ---\n");
    start_time_w = omp_get_wtime();

    #pragma omp parallel
    {
        // #pragma omp sections ensures each section runs on only one thread
        #pragma omp sections
        {
            // Section 1: Compute Sum
            #pragma omp section
            {
                for (int i = 0; i < N_T5_SMALL; i++) sum_res += A[i];
                printf("Thread %d handles Sum.\n", omp_get_thread_num());
            }

            // Section 2: Compute Product
            #pragma omp section
            {
                for (int i = 0; i < 5; i++) product_res *= B[i];
                printf("Thread %d handles Product.\n", omp_get_thread_num());
            }

            // Section 3: Find Maximum
            #pragma omp section
            {
                max_res = C[0];
                for (int i = 1; i < N_T5_SMALL; i++) if (C[i] > max_res) max_res = C[i];
                printf("Thread %d handles Max.\n", omp_get_thread_num());
            }
        } 
    }

    end_time_w = omp_get_wtime();
    double time_spent_par = end_time_w - start_time_w;

    printf("Time (omp_get_wtime()): %f seconds\n", time_spent_par);
    printf("Results: Sum=%lld, Product=%lld, Max=%d\n", sum_res, product_res, max_res);
    printf("Expected Outcome: Each section runs in parallel, reducing overall execution time.\n");

    return 0;
}