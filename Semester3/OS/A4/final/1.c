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

int statusg;
#define int long long int
#define BUF 100000

void runSort();
void swap();
int partition();
int partition_r();
void quickSort();
void quickSort_normal();
int brr[BUF];
int crr[BUF];

int *shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    return (int *)shmat(shmget(mem_key, size, IPC_CREAT | 0666), NULL, 0);
}

typedef struct arg{
    int low;
    int high;
    int* arr;    
} A;

void swap(int *a, int *b){   
    int t;
    t = *b;
    *b = *a;
    *a = t;
}

void swapa(int i, int j, int a[]){   

    int temp = a[i];
    a[i] = a[j];
    a[j] = temp;
}
void *threaded_quicksort(void* a){
    A *args = (A*) a;

    int *arr = args->arr;
    int l = args->low;
    int r = args->high;
    if(r<l) 
        return NULL;    

    if(r<=4+l){
        int a[5];
        for(int i=l;i<r;i++){
            int j=i+1; 
            for(;j<=r;j++)            
                if(arr[j]<arr[i] && !(j>r)){
                    swapa(i,j,arr);
                }
        }
        return NULL;
    }

    A a1;
    A a2;

    int pi = partition_r(arr, l, r);

    a1.low = l;
    a1.high = pi - 1;
    a1.arr = arr;

    a2.low = pi + 1;
    a2.high = r;
    a2.arr = arr;

    pthread_t tid2;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_quicksort, &a1);
    pthread_create(&tid2, NULL, threaded_quicksort, &a2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
}

void quickSort_normal(int arr[], int low, int high){

    if(high<=4+low){
        int a[5];
        for(int i=low;i<high;i++){
            int j=i+1; 
            for(;j<=high;j++)            
                if(arr[j]<arr[i] && !(j>high)){
                    swapa(i,j,arr);
                }
        }
    }

    if (high > low){
        int pi = partition_r(arr, low, high);

        quickSort_normal(arr, low, pi - 1);
        quickSort_normal(arr, pi + 1, high);
    }
}

void runSorts(int n){

    struct timespec ts;

    //getting shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    printf("Enter the unsorted array: ");
    for (int i = 0; i < n; i++){
        scanf("%lld", arr + i);
        brr[i] = arr[i];
        crr[i] = arr[i];
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

    quickSort(arr, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("\nRunning time for concurrent quicksort (n = %lld): %Lf\n", n, en - st);
    long double t1 = en - st;

    pthread_t tid;
    A a;
    a.low *= 0;
    a.arr = crr;
    a.high = n-1;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL);    

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    long double t2 = en-st;
    printf("Running time for threaded concurrent quicksort (n = %lld): %Lf\n",n, t2);

    quickSort_normal(brr, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    long double t3 = en - st;
    printf("Running time for normal quicksort (n = %lld): %Lf\n\n",n, t3);
    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n", t1/t3, t2/t3);
    printf("Sorted array: \n");
    for(int i=0;i<n;i++)
        printf("%lld ", arr[i]);
    printf("\n");
    shmdt(arr);
}

int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = low;
    i--;    // Index of smaller element
    for (int j = low; j < high; j++){
        if (arr[j] <= pivot){

            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    i++;
    return i;
}

int partition_r(int arr[], int low, int high){

    int range = high - low;
    int random = low + rand() % range;

    swap(&arr[random], &arr[high]);

    return partition(arr, low, high);
}

void quickSort(int arr[], int low, int high){
    if (!(high < low)){

        if (high <= 4 + low){
            int a[5];
            for (int i = low; i < high; i++){
                int j = i + 1;
                for (; j <= high; j++)
                    if (arr[j] < arr[i] && j <= high)
                        swapa(i,j,arr);
            }
            return;
        }
        int pi = partition_r(arr, low, high);

        int pid1 = fork();
        if (pid1 == 0){
            quickSort(arr, low, pi - 1);
            _exit(1);
        }
        else
        {
            int pid2 = fork();
            if (pid2 == 0){
                quickSort(arr, pi + 1, high);
                _exit(1);
            }
            else{
                waitpid(pid1, &statusg, 0);
                waitpid(pid2, &statusg, 0);
            }
        }
    }
    else
        _exit(1);
}

signed main(){
    srand(time(NULL));
    int n;
    printf("Enter n (number of elements): ");
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}
