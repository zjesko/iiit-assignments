#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
void runSort();
void swap();
int partition();
int partition_r();
void quickSort();
void printArray();
int quickSort_normal();
int brr[100000];
int crr[100000];
int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}
struct arg{
    int low;
    int high;
    int* arr;    
};

void *threaded_mergesort(void* a){
    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg*) a;

    int l = args->low;
    int r = args->high;
    int *arr = args->arr;
    if(l>r) return NULL;    
    
    //insertion sort
    if(r-l+1<=5){
        int a[5], mi=INT_MAX, mid=-1;
        for(int i=l;i<r;i++)
        {
            int j=i+1; 
            for(;j<=r;j++)            
                if(arr[j]<arr[i] && j<=r) 
                {
                    int temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
        }
        return NULL;
    }

    //sort left half array
    int pi = partition_r(arr, l, r);
    struct arg a1;
    a1.low = l;
    a1.high = pi - 1;
    a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_mergesort, &a1);
    
    //sort right half array
    struct arg a2;
    a2.low = pi + 1;
    a2.high = r;
    a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_mergesort, &a2);
    
    //wait for the two halves to get sorted
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
}
int quickSort_normal(int arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now 
        at right place */
        int pi = partition_r(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort_normal(arr, low, pi - 1);
        quickSort_normal(arr, pi + 1, high);
    }
}
void runSorts(long long int n)
{

    struct timespec ts;

    //getting shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++)
    {
        scanf("%d", arr + i);
        brr[i] = arr[i];
        crr[i] = arr[i];
    }
    printf("Running concurrent_QuickSort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

    quickSort(arr, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en - st;

    pthread_t tid;
    struct arg a;
    a.low = 0;
    a.high = n-1;
    a.arr = crr;
    printf("Running threaded_concurrent_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    //multithreaded mergesort
    pthread_create(&tid, NULL, threaded_mergesort, &a);
    pthread_join(tid, NULL);    
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;

    printf("Running normal_QuickSort for n = %lld\n", n);
        quickSort_normal(brr, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;
    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n\n", t1/t3, t2/t3);
    printArray(arr, n);
    shmdt(arr);
}
void swap(int *a, int *b)
{   
    int t;
    t = *b;
    *b = *a;
    *a = t;
}
int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1);     // Index of smaller element
    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {

            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
int partition_r(int arr[], int low, int high)
{
    // Generate a random number in between
    // low .. high
    srand(time(NULL));
    int random = low + rand() % (high - low);

    // Swap A[random] with A[high]
    swap(&arr[random], &arr[high]);

    return partition(arr, low, high);
}
// Generates Random Pivot, swaps pivot with
// end element and calls the partition function
/* The main function that implements QuickSort 
arr[] --> Array to be sorted, 
low --> Starting index, 
high --> Ending index */
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {

        if (high - low + 1 <= 5)
        {
            int a[5], mi = INT_MAX, mid = -1;
            for (int i = low; i < high; i++)
            {
                int j = i + 1;
                for (; j <= high; j++)
                    if (arr[j] < arr[i] && j <= high)
                    {
                        int temp = arr[i];
                        arr[i] = arr[j];
                        arr[j] = temp;
                    }
            }
            return;
        }
        /* pi is partitioning index, arr[p] is now 
        at right place */
        int pi = partition_r(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        int pid1 = fork();
        int pid2;
        if (pid1 == 0)
        {
            //sort left half array
            quickSort(arr, low, pi - 1);
            _exit(1);
        }
        else
        {
            pid2 = fork();
            if (pid2 == 0)
            {
                //sort right half array
                quickSort(arr, pi + 1, high);
                _exit(1);
            }
            else
            {
                //wait for the right and the left half to get sorted
                int status;
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
            }
        }
    }
    else
    {
        _exit(1);
    }
}
/* Function to print an array */
void printArray(int arr[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

//Driver program to test above functions

int main()
{
    long long int n;
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}
