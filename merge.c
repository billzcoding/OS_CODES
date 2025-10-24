#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 16
#define SEGMENT_SIZE (ARRAY_SIZE / 4)
#define N_THREADS 4

// --- Helper function for Merge Sort: Merges two sorted sub-arrays ---
void merge(int arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    int L[n1], R[n2];

    // Copy data to temp arrays L[] and R[]
    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temp arrays back into arr[left..right]
    i = 0; // Initial index of first sub-array
    j = 0; // Initial index of second sub-array
    k = left; // Initial index of merged sub-array
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// --- Helper function for Merge Sort: Recursive divide-and-conquer ---
void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        // Recursive calls
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Merge the two halves
        merge(arr, left, mid, right);
    }
}


int main() {
    printf("--- Task 1: Merge Sort with Ordered Output (%d threads) ---\n", N_THREADS);
    
    // Seed random number generator
    srand(time(NULL)); 

    int arr[ARRAY_SIZE];
    double start_w, end_w;

    // Set thread count
    omp_set_num_threads(N_THREADS);


    // =======================================================
    // --- RUN 1: Parallel Sorting WITHOUT ordered (Random Output) ---
    // =======================================================
    
    // 1. Generate array of random integers
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 100; // Random numbers 0-99
    }
    printf("\nInitial Array (Sample): %d, %d, %d, ...\n", arr[0], arr[1], arr[2]);
    printf("\n--- RUN 1: Parallel Sorting WITHOUT ordered ---\n");
    start_w = omp_get_wtime();

    // Parallelize the loop over the 4 segments
    #pragma omp parallel for
    for (int i = 0; i < N_THREADS; i++) { // i = segment index (0, 1, 2, 3)
        int start = i * SEGMENT_SIZE;
        int end = start + SEGMENT_SIZE - 1;
        
        // 1. Sort the segment using the defined mergeSort function
        mergeSort(arr, start, end);

        // 2. Print: Output order is non-deterministic (random)
        printf("Segment %d sorted by Thread %d: ", i + 1, omp_get_thread_num());
        for(int j = 0; j < SEGMENT_SIZE; j++) {
            printf("%d ", arr[start + j]);
        }
        printf("\n");
    }

    end_w = omp_get_wtime();
    printf("\nTime WITHOUT ordered: %f s\n", end_w - start_w);


    // =======================================================
    // --- RUN 2: Parallel Sorting WITH ordered (Sequential Output) ---
    // =======================================================
    
    // Re-generate array for a fair comparison of the run structure
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % 100;
    }
    printf("\n--- RUN 2: Parallel Sorting WITH ordered ---\n");
    start_w = omp_get_wtime();

    // Parallelize the loop over the 4 segments and add the 'ordered' clause
    #pragma omp parallel for ordered
    for (int i = 0; i < N_THREADS; i++) { // i = segment index (0, 1, 2, 3)
        int start = i * SEGMENT_SIZE;
        int end = start + SEGMENT_SIZE - 1;
        
        // 1. Parallel Work (Sorting) - This happens concurrently
        mergeSort(arr, start, end);

        // 2. Ordered Output (Printing)
        // The block executes in the sequence of loop index 'i' (1, 2, 3, 4)
        #pragma omp ordered
        {
            printf("Segment %d sorted by Thread %d: ", i + 1, omp_get_thread_num());
            for(int j = 0; j < SEGMENT_SIZE; j++) {
                printf("%d ", arr[start + j]);
            }
            printf("\n");
        }
    }

    end_w = omp_get_wtime();
    printf("\nTime WITH ordered: %f s\n", end_w - start_w);

    printf("\nExpected: Run 1 prints in mixed order. Run 2 prints strictly Segment 1, 2, 3, 4.\n");

    return 0;
}