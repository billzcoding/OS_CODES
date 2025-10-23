#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 1: Private vs Shared Variable Demonstration ---\n");
    int x = 10; // Initial value

    // Set thread count for demonstration
    omp_set_num_threads(4);

    // --- Block 1: Private (x is private) ---
    printf("\n--- Block 1: Private Variable (private(x)) ---\n");
    #pragma omp parallel private(x)
    {
        // Each thread gets its own copy of x, initialized to an undefined value (not 10).
        // The thread-local copy is set to the thread ID.
        x = omp_get_thread_num();
        printf("Thread %d: Private x = %d\n", omp_get_thread_num(), x);
    }
    // The main x remains unchanged after the private copies are destroyed.
    printf("After PRIVATE block: main x = %d (Expected: 10)\n", x);
    

    // --- Block 2: Shared (x is shared implicitly since it's declared outside) ---
    printf("\n--- Block 2: Shared Variable (Race Condition) ---\n");
    // Reset x for the shared test
    x = 10;
    
    #pragma omp parallel
    {
        // All threads modify the SAME x simultaneously, causing a race condition.
        x += 1;
    }

    // The result is likely incorrect (not 10 + 4 = 14) due to overlapping updates.
    printf("After SHARED block: final x = %d (Expected: 14, Actual: Unpredictable Race Value)\n", x);
    
    printf("\nComment: Private variables isolate work, ensuring the original variable is safe. Shared variables require synchronization (like atomic or critical) for safe updates, as shown by the race condition.\n");

    return 0;
}










#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 2: Barrier Demonstration ---\n");

    int N_THREADS = 4;
    omp_set_num_threads(N_THREADS);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();

        // Phase 1: All threads execute this
        printf("Thread %d: Before barrier\n", id);

        // All threads wait here until every thread in the team reaches this point
        #pragma omp barrier 

        // Phase 2: All threads continue together
        printf("Thread %d: After barrier\n", id);
    }

    printf("\nObservation: The \"Before barrier\" messages may appear in any order, but ALL of them will print before ANY of the \"After barrier\" messages.\n");

    return 0;
}










#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 3: Atomic Counter ---\n");

    int N_THREADS = 16;
    int ITERATIONS = 100;
    int TOTAL_EXPECTED = N_THREADS * ITERATIONS;

    // Set thread count
    omp_set_num_threads(N_THREADS);

    // --- 1. Without atomic (Race Condition) ---
    int counter_no_atomic = 0;
    printf("\n--- 1. Without atomic (Race Condition) ---\n");

    #pragma omp parallel for
    for (int i = 0; i < TOTAL_EXPECTED; i++) {
        // Unsafe update -> race condition occurs
        counter_no_atomic++; 
    }

    printf("Final counter (No atomic): %d (Expected: %d)\n", counter_no_atomic, TOTAL_EXPECTED);
    printf("Observation: The final counter is likely less than %d (Wrong Result).\n", TOTAL_EXPECTED);


    // --- 2. With #pragma omp atomic (Correct) ---
    int counter_with_atomic = 0;
    printf("\n--- 2. With #pragma omp atomic (Correct) ---\n");
    
    #pragma omp parallel
    {
        for (int i = 0; i < ITERATIONS; i++) {
            // #pragma omp atomic ensures the update is performed safely by one thread at a time
            #pragma omp atomic
            counter_with_atomic++;
        }
    }

    printf("Final counter (With atomic): %d (Expected: %d)\n", counter_with_atomic, TOTAL_EXPECTED);
    printf("Observation: The final counter is exactly %d (Correct Result).\n", TOTAL_EXPECTED);

    return 0;
}













#include <stdio.h>
#include <omp.h>

int main() {
    printf("--- TASK 4: Master Thread Logging ---\n");

    int N_THREADS = 8;
    omp_set_num_threads(N_THREADS);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();

        // Master Block 1: Start Message
        #pragma omp master
        {
            printf("--------------------------------------\n");
            printf("Master thread %d: STARTING all work.\n", id);
            printf("--------------------------------------\n");
        }

        // Each thread performs a dummy computation
        for (int i = 0; i < 5; i++) {
            printf("Thread %d is performing computation step %d...\n", id, i + 1);
        }

        // Master Block 2: End Message
        #pragma omp master
        {
            printf("--------------------------------------\n");
            printf("Master thread %d: All work done.\n", id);
            printf("--------------------------------------\n");
        }
    }

    printf("\nObservation: The STARTING and END messages appear exactly once, executed only by Thread 0.\n");
    printf("Other threads immediately continue after the master block (no barrier).\n");

    return 0;
}


















#include <stdio.h>
#include <omp.h>

#define N_T5 100 // Array size

int main() {
    printf("--- TASK 5: Parallel Array Summation ---\n");

    int arr[N_T5];
    int expected_sum = 0;
    
    // Initialize array (values 1-100) and calculate expected sum
    for (int i = 0; i < N_T5; i++) {
        arr[i] = i + 1;
        expected_sum += arr[i];
    }
    printf("Expected Sum: %d\n", expected_sum);
    omp_set_num_threads(4); // Use 4 threads

    // --- Method A: Shared variable with Atomic ---
    int sum_A = 0; // Shared variable
    printf("\n--- Method A: Shared Sum with Atomic ---\n");

    #pragma omp parallel for
    for (int i = 0; i < N_T5; i++) {
        // Use atomic for safe update to the shared sum_A
        #pragma omp atomic
        sum_A += arr[i];
    }
    printf("Final Total Sum (Method A - Atomic): %d\n", sum_A);


    // --- Method B: Private Partial Sums with Critical ---
    int sum_B = 0; // Shared final sum
    printf("\n--- Method B: Private Partial Sums with Critical ---\n");

    #pragma omp parallel
    {
        int local_sum = 0; // Private partial sum per thread
        int id = omp_get_thread_num();

        // Calculate local sum (implicit omp for)
        #pragma omp for nowait // Distribute work, don't wait at the end
        for (int i = 0; i < N_T5; i++) {
            local_sum += arr[i];
        }

        printf("Thread %d Partial Sum: %d\n", id, local_sum);

        // Update the shared final sum using a critical section
        #pragma omp critical
        {
            sum_B += local_sum;
        }
    }

    printf("Final Total Sum (Method B - Critical): %d\n", sum_B);
    printf("Observation: Both methods yield the correct sum of %d.\n", expected_sum);

    return 0;
}












