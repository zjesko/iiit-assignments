#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define int long long int
#define BUF 1024

int M, N, K, loaded[BUF], slots[BUF];

pthread_t chef_id[BUF], student_id[BUF], table_id[BUF];
pthread_mutex_t chef_table_lock[BUF], table_lock[BUF];

int min(int x, int y){
	if(y > x)
        return x;
    else
        return y;
}

void biryani_ready(int index, int w, int r, int p){

	for(int i=0;i<r;i++){
		bool served = false;
		for(int j=0;j<N;j++){
			if(!pthread_mutex_trylock(&chef_table_lock[j])){
				served = true;
				printf("Vessel #%lld made by Chef #%lld is served (Table #%lld)\n", i+1, index+1, j+1);
		
				loaded[j] = p;
				break;
			}
		}
        int t = served ? 1 : 0;
		i=i-1+t;
	}
}

void *chef(void *index){
	int in = *((int*)index);
    in++;
	while(true){
		int w = rand()%4+2;
        int r = rand()%10+1;
        int p = rand()%4+2;
		printf("Chef #%lld taking %lld seconds to prepare %lld Vessels (each with portions for %lld students)\n", in, w, r, p);

		sleep(w);
		biryani_ready(in-1, w, r, p);
		return NULL;
	}
	return NULL;
}


void *serving_table(void *index){
	int in = *((int*)index);
	while(true){
		
        while(loaded[in] == 0);
        int r = rand()%10;
		slots[in] = min(loaded[in],r+1);
		printf("Serving Table #%lld resulted in %lld slots being available\n", in+1, slots[in]);

        pthread_mutex_unlock(&table_lock[in]);
	    while(slots[in]!=0){
        }
	    pthread_mutex_trylock(&table_lock[in]);
        int s = slots[in];
		loaded[in] -= s;

		if(loaded[in] == 0)
			pthread_mutex_unlock(&chef_table_lock[in]);
		sleep(1);
	}

	return NULL;
}

void student_in_slot(int index, int table){
	index++;
    printf("Student #%lld is eating at Table #%lld: %lld slots available\n", index, table+1, slots[table]);
	sleep(2);
	slots[table]-=1;
}

void wait_for_slot(int index)
{
	bool served = false;
	printf("Student #%lld is waiting for a slot\n", index+1);
	while(served == false){
		for(int i = 0;i<N;i++){
			if(!pthread_mutex_trylock(&table_lock[i])){
				if(slots[i] == 0){
					pthread_mutex_unlock(&table_lock[i]);
					continue;
				}
				student_in_slot(index, i);
				served = true;
				pthread_mutex_unlock(&table_lock[i]);
				break;
			}
		}
	}
}

void *student(void *index)
{
	int in = *((int*)index);
	wait_for_slot(in);

	return NULL;
}

void destroy_mutex(){
    int i=0;
    while(i<N){
        pthread_mutex_destroy(&table_lock[i]);
        pthread_mutex_destroy(&chef_table_lock[i]);
        i++;
    } 
}

signed main()
{
	srand(time(NULL));
    int n,m,k;
	printf("Chefs #: ");
	scanf("%lld", &m);
	printf("Tables #: ");
	scanf("%lld", &n);
	printf("Students #: ");
	scanf("%lld", &k);
	
    int chefin[BUF], servein[BUF], studentin[BUF];
    N=n;M=m;K=k;
	for(int i = 0;i<M;i++)
		chefin[i] = i;
	
    for(int i = 0;i<N;i++)
		servein[i] = i;

    for(int i = 0;i<K;i++)
		studentin[i] = i;
    
    for(int i=0;i<M;i++){
		if(pthread_create(&(chef_id[i]), NULL, &chef, &chefin[i]))
			printf("Error in creating thread for Chef #%lld\n", i+1);
		sleep(0.0001);
	}
	for(int i = 0;i<N;i++){
		
        if(pthread_mutex_init(&chef_table_lock[i], NULL))
			return 0;
		
        if(pthread_mutex_init(&table_lock[i], NULL))
			return 0;
		
        pthread_mutex_lock(&table_lock[i]);
		
        pthread_create(&(table_id[i]), NULL, &serving_table, &servein[i]);
		sleep(0.0001);
	}
	for(int i = 0;i<K;i++){
		sleep(2);
		pthread_create(&(student_id[i]), NULL, &student, &studentin[i]);
		sleep(0.0001);
	}

	for(int i=0;i<K;i++)
		pthread_join(student_id[i], NULL);
    
    destroy_mutex();


	return 0;
}
