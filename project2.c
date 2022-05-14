// Created By: Quinnlan Hipp
// Date Created: 3/14/22
// Last Modified: 3/21/22
// Project #: 2

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

// Defining macros to save lines of repetitive functions or equations
#define getRangeSize(range) (range)->high - (range)->low + 1
#define getDifferenceTimeOfClock(startTime, endTime) ((double)(endTime - startTime)) / CLOCKS_PER_SEC
#define getDifferenceTimeOfDay(startTimeOfDay, endTimeOfDay) (endTimeOfDay.tv_sec - startTimeOfDay.tv_sec) + (endTimeOfDay.tv_usec - startTimeOfDay.tv_usec) / 1e6

// Creating a struct called range that contains a low and high
struct range
{
    int low;
    int high;
};
// Declaring input argument variables to be used in program
int n, s, r, p, t;
char *a, *m, *m3;
int *arr = NULL;
struct range *ranges = NULL;
clock_t startTime, endTime;
struct timeval startTimeOfDay, endTimeOfDay;

// Declaring variables to hold each of the required times
double createTime, initTime, shuffleTime, sortCpuTime;

// Declaring each method so they can be placed below main without warnings or errors
void initializeArgs(int argc, char *argv[]);
int checkForErrors();
void createArray();
void shuffle();
int isSorted();
void *runner(void *parameter);
void insertionSort(int arr[], int high, int low);
void shellSort(int high, int low);
void quickSort(int high, int low);
int partition(int low, int high);

// Main Method
int main(int argc, char *argv[])
{
    clock_t programStartTime = clock();
    struct timeval programStartTimeOfDay;
    gettimeofday(&programStartTimeOfDay, NULL);
    // gcc -o project2 project2.c
    // ./project2 -n 700000 -s 10 -t 4

    if (((argc % 2) != 1) || (argc < 3) || (argc > 17))
    {
        fprintf(stderr, "Incorrect number of arguments.\n"); // Prints error message
    }
    else
    {
        printf("Correct number of arguments. %d\n", argc);
        initializeArgs(argc, argv);
        if (checkForErrors() == 1)
        {
            printf("\n\nAn error was found in the arguments.\n\n"); // Prints error message
            return 1;
        }
        // Time to create the array
        startTime = clock();
        arr = malloc(n * sizeof(int));
        endTime = clock();
        createTime = getDifferenceTimeOfClock(startTime, endTime);
        ranges = malloc(p * sizeof(struct range));

        // Time to fill the array
        startTime = clock();
        createArray();
        endTime = clock();
        initTime = getDifferenceTimeOfClock(startTime, endTime);

        // Time to shuffle the array
        startTime = clock();
        shuffle();
        endTime = clock();
        shuffleTime = getDifferenceTimeOfClock(startTime, endTime);

        // If multithreading is enabled
        if (strcmp(m, "Y") == 0 || strcmp(m, "y") == 0)
        {
            startTime = clock();
            int pieceNum = 1;
            bool isM3 = false;

            struct range pieces[p];
            struct range *initialPiece = &pieces[0];
            initialPiece->low = 0;
            initialPiece->high = n - 1;

            while (pieceNum < p)
            {
                // Finding the biggest piece in the range array thus far
                struct range *biggestPiece = &pieces[0];
                int biggestPieceSize = getRangeSize(biggestPiece);

                // Redefining the biggest piece if any others are bigger than the current one
                for (int i = 1; i < pieceNum; i++)
                {
                    struct range *piece = &pieces[i];

                    int pieceSize = getRangeSize(piece);

                    if (biggestPieceSize < pieceSize)
                    {
                        biggestPiece = piece;
                        biggestPieceSize = pieceSize;
                    }
                }

                // Call partition using the low and high of the biggest piece
                int pivotIndex = partition(biggestPiece->low, biggestPiece->high);

                // Creates the new pieces after the partition
                struct range *newPiece = &pieces[pieceNum];

                // Set the low and high of the new piece
                newPiece->low = pivotIndex + 1;
                newPiece->high = biggestPiece->high;

                // Decrease the size of the biggest piece since we just partitioned it in "half"
                biggestPiece->high = pivotIndex - 1;

                // Increment number of pieces
                pieceNum++;
            }
            endTime = clock();
            // Sort the pieces in descending order to easily access the next biggest
            for (int i = 1; i < pieceNum; i++)
            {
                struct range key = pieces[i]; 
                int j = i - 1;

                while (j >= 0 && getRangeSize(&key) > getRangeSize(&pieces[j]))
                {
                    pieces[j + 1] = pieces[j];
                    --j;
                }
                pieces[j + 1] = key;
            }
            // Time to sort the array
            startTime = clock();
            gettimeofday(&startTimeOfDay, NULL);

            pthread_t threads[t];
            pthread_attr_t threadAttributes[t];

            int nextPieceToSchedule = 0;

            // Begin sorting as many pieces and there are MaxThreads
            for (int i = 0; i < t; i++)
            {
                // Points to the piece to run next
                struct range *piece = &pieces[nextPieceToSchedule];

                // Initialize thread attributes
                pthread_attr_init(&threadAttributes[i]);

                // Create the thread
                pthread_create(&threads[i], &threadAttributes[i], runner, piece);

               // Increment the next piece to schedule
                nextPieceToSchedule++;
            }

            // Check for completed threads starting at the first one
            int threadIndex = 0;

            // Continue to seek completed threads until there are no remaining unscheduled pieces
            while (nextPieceToSchedule < pieceNum)
            {
                // Get the thread at the thread index.
                pthread_t *thread = &threads[threadIndex];

                // Check if threat has completed
                // if (pthread_tryjoin_np(*thread, NULL) == 0)
                if (pthread_join(*thread, NULL) == 0)
                {
                    // Points to the piece to run next
                    struct range *piece = &pieces[nextPieceToSchedule];

                    // Initialize the thread attributes
                    pthread_attr_init(&threadAttributes[threadIndex]);

                    // Create the thread
                    pthread_create(&threads[threadIndex], &threadAttributes[threadIndex], runner, piece);

                    // Increment to the next piece to schedule
                    nextPieceToSchedule++;

                    // Reset thread index to zero to start back from the beginning
                    threadIndex = 0;
                }
                else // Unable to join the thread
                {
                    // Increment thread index
                    threadIndex++;

                    // Stop searching if index >= max threads to save CPU
                    if (threadIndex >= t)
                    {
                        // Reset index and sleep for 50ms to give threads a little more time to complete
                        threadIndex = 0;
                        usleep(50000);
                    }
                }
            }

            // Check if threads have finished
            for (int i = 0; i < t; i++)
            {
                pthread_join(threads[i], NULL);
            }
        }
        else // If multithreading is disabled
        {
            // Start real time and CPU time to sort the array
            startTime = clock();
            gettimeofday(&startTimeOfDay, NULL);
            // Sort the whole array using hybrid quicksort
            quickSort(n - 1, 0);
        }
    }

    // End time to sort the array and make calculations
    endTime = clock();
    gettimeofday(&endTimeOfDay, NULL);
    sortCpuTime = getDifferenceTimeOfClock(startTime, endTime);
    double sortWallClockTime = getDifferenceTimeOfDay(startTimeOfDay, endTimeOfDay);

    // Calculate total time to run program
    clock_t programEndTime = clock();
    struct timeval programEndTimeOfDay;
    gettimeofday(&programEndTimeOfDay, NULL);
    double allCpuTime = getDifferenceTimeOfClock(programStartTime, programEndTime);
    double allWallClockTime = getDifferenceTimeOfDay(programStartTimeOfDay, programEndTimeOfDay);

    // Check if array is sorted correctly
    isSorted();

    // Print out all timings in specific format
    printf("Array created in %6.3f seconds \nArray initialized in %7.3f seconds \nArray scrambled in %7.3f seconds\n", createTime, initTime, shuffleTime);
    printf("Seconds spent sorting: Wall Clock: %7.3f / CPU: %7.3f\n", sortWallClockTime, sortCpuTime);
    printf("Total Run Time (sec): %7.3f\n", allWallClockTime);
    return 0;
}

// Initialize the input arguments
void initializeArgs(int argc, char *argv[])
{
    // Sets a few optional arguments to their default values
    // *Will be overridden if user passes in different value*
    s = 10;
    a = "s";
    m = "y";
    p = 10;
    t = 4;
    m3 = "n";

    // Loops through all passed in arguments skipping "./project2" which is at index 0
    int i = 1;
    while (i < argc)
    {
        // Initializes Size n
        if (strcmp(argv[i], "-n") == 0)
        {
            n = atoi(argv[i + 1]);
            printf("-n : %d\n", n);
        }
        // Initialized Threshold s
        else if (strcmp(argv[i], "-s") == 0)
        {
            s = atoi(argv[i + 1]);
            printf("-s : %d\n", s);
        }
        // Initialized Alternate a
        else if (strcmp(argv[i], "-a") == 0)
        {
            a = argv[i + 1];
            printf("-a : %s\n", a);
        }
        // Initialized Seed r
        else if (strcmp(argv[i], "-r") == 0)
        {
            r = atoi(argv[i + 1]);
            printf("-r : %d\n", r);
        }
        // Initialized Multithread? m
        else if (strcmp(argv[i], "-m") == 0)
        {
            m = argv[i + 1];
            printf("-m : %s\n", m);
        }
        // Initialized Pieces p
        else if (strcmp(argv[i], "-p") == 0)
        {
            p = atoi(argv[i + 1]);
            printf("-p : %d\n", p);
        }
        // Initialized Max Threads t
        else if (strcmp(argv[i], "-t") == 0)
        {
            t = atoi(argv[i + 1]);
            printf("-t : %d\n", t);
        }
        // Initialized Median of 3? m3
        else if (strcmp(argv[i], "-m3") == 0)
        {
            m3 = argv[i + 1];
            printf("-m : %s\n", m3);
        }
        // An unknown parameter was passed
        else
        {
            fprintf(stderr, ", Unknown parameter inputted.\n"); // Prints error message
        }
        i += 2;
    }
    // Sets pieces and max threads to 0 if multithreaded is disabled
    if (strcmp(m, "N") == 0 || strcmp(m, "n") == 0)
    {
        p = 0;
        t = 0;
    }
}

// Checks a few situations that should not be 
int checkForErrors()
{
    // Checks if Size n < 1
    if (n < 1)
    {
        return 1;
    }
    // Checks if Max Threads t > Pieces p
    else if (t > p)
    {
        return 1;
    }
    return 0;
}

// Runs a standard insertion sort that passes an array and sorts only between the low and high indexes
void insertionSort(int arr[], int high, int low)
{
    int i, j, currValue;
    for (i = low + 1; i <= high; i++)
    {
        currValue = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j] > currValue)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = currValue;
    }
}

// Create Array (fills)
void createArray()
{
    for (int i = 0; i < n; i++)
    {
        arr[i] = i;
    }
}

// Shuffle Array
void shuffle()
{   
    // Checks seed specifications
    if (r == -1)
    {
        srand(clock());
    }
    else if (r < -2 || r >= 0)
    {
        srand(r);
    }
    // Shuffles the array
    for (int i = 0; i < n; i++)
    {
        int j = rand() % n;
        int hold = arr[i];
        arr[i] = arr[j];
        arr[j] = hold;
    }
}

// Checks if array is sorted correctly 
int isSorted()
{
    bool hasError = false;
    for (int i = 0; i < n; i++)
    {
        // Checks for errors in the array
        // if (i != arr[i])
        if ((arr[i] + 1 != arr[i + 1]) && i != n - 1)
        {
            printf("\nThe list is not sorted... at index[%d]... the size of array is: %d", i, n);
            hasError = true;
        }
    }
    if (hasError)
    {
        return 1;
    }
    // Prints this if no errors were found
    printf("The list is sorted!\n");
    return 0;
}

// Runner function
void *runner(void *parameter)
{
    // Performs the sort between low and high indexes then exits thread
    struct range *rangeParameter = (struct range *)parameter;
    quickSort(rangeParameter->high, rangeParameter->low);
    pthread_exit(0);
}

// // Runs a Hibbard's sequence shell sort that passes an array and sorts only between the low and high indexes
void shellSort(int high, int low)
{
    int k = 1;
    int size = high - low + 1;
    while (size >= k)
    {
        k *= 2;
    }
    k = (k / 2) - 1;
    do
    {
        for (int i = low; i < (high - (k - 1)); i++)
        {

            for (int j = i; j >= 0; j -= k)
            {
                if (arr[j] <= arr[j + k])
                {
                    break;
                }
                else
                {
                    int temp = arr[j];
                    arr[j] = arr[j + k];
                    arr[j + k] = temp;
                }
            }
        }
        k = k >> 1;
    } while (k > 0);
}

// Hybrid Function
void quickSort(int high, int low)
{
    // Errors if high or low are < 0
    if (high < 0 || low < 0)
    {
        printf("\n\n!!!Number less than zero passed!!!\n\n");
        printf("High: %d\nLow: %d\n\n", high, low);
    }
    int pSize = high - low + 1;

    // Returns since array is already sorted
    if (pSize < 2)
    {
        return;
    }

    // Compare and swap values if necessary
    else if (pSize == 2)
    {
        if (arr[low] > arr[high])
        {
            int temp = arr[low];
            arr[low] = arr[high];
            arr[high] = temp;
            return;
        }
    }

    // Use shell sort
    else if (pSize > 2 && pSize <= s && (strcmp(a, "S") == 0 || strcmp(a, "s") == 0))
    {
        shellSort(high, low);
    }

    // Use insertion sort
    else if (pSize > 2 && pSize <= s && (strcmp(a, "I") == 0 || strcmp(a, "i") == 0))
    {
        insertionSort(arr, high, low);
    }

    // Recursively call QuickSort to both sides of partition
    else if (pSize > s)
    {
        int currentIdx = 0;
        bool isM3 = false;

        int pivotIndex = 0;
        int pivot;
        if ((strcmp(m3, "y") == 0 || strcmp(m3, "Y") == 0)) // Use median-of-3 for pivot
        {
            isM3 = true;
            int arrm3[3] = {arr[low], arr[(high - low) / 2 + low], arr[high]};
            insertionSort(arrm3, 2, 0);
            pivot = arrm3[1];
        }
        else
        {
            pivot = arr[high]; // Uses high index as pivot
        }

        int i = (low - 1);

        for (int j = low; j <= high; j++)
        {
            // If current element is smaller than the pivot
            if (arr[j] < pivot)
            {
                i++;

                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }

        // If Median of 3 is enabled
        if (isM3)
        {
            // Finds index of the pivot
            for (int x = low; x <= high; x++)
            {
                if (arr[x] == pivot)
                {
                    pivotIndex = x;
                    break;
                }
            }

            // Swaps pivot and i+1
            int temp2 = arr[i + 1];
            arr[i + 1] = arr[pivotIndex];
            arr[pivotIndex] = temp2;
            pivotIndex = i + 1;
        }
        else
        {
            // Swaps pivot (on the far right) and i+1
            int temp2 = arr[i + 1];
            arr[i + 1] = arr[high];
            arr[high] = temp2;
            pivotIndex = i + 1;
        }

        // Calls quicksort on lower half and upper half of partition
        quickSort(pivotIndex - 1, low);
        quickSort(high, pivotIndex + 1);
    }
}

// Partitioning Function to split array into p pieces
int partition(int low, int high)
{
    int currentIdx = 0;
    bool isM3 = false;

    int pivotIndex = 0;
    int pivot;
    if ((strcmp(m3, "y") == 0 || strcmp(m3, "Y") == 0)) // Use median-of-3 for pivot
    {
        isM3 = true;
        int arrm3[3] = {arr[low], arr[(high - low) / 2 + low], arr[high]};
        insertionSort(arrm3, 2, 0);
        pivot = arrm3[1];
    }
    else
    {
        pivot = arr[high]; // Uses high index as pivot
    }

    int i = (low - 1); // Index of smaller element and indicates the
                       // right position of pivot found so far

    for (int j = low; j <= high; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++; // increment index of smaller element

            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // If Median of 3 is enabled 
    if (isM3)
    {
        // Finds index of pivot
        for (int x = low; x <= high; x++)
        {
            if (arr[x] == pivot)
            {
                pivotIndex = x;
                break;
            }
        }

        // Swaps pivot and i+1
        int temp2 = arr[i + 1];
        arr[i + 1] = arr[pivotIndex];
        arr[pivotIndex] = temp2;
        pivotIndex = i + 1;
    }
    else
    {
        // Swaps pivot (on the far right) and i+1
        int temp2 = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp2;
        pivotIndex = i + 1;
    }
    return pivotIndex;
}