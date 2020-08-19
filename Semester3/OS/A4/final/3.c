#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

#define int long long int

int num_cabs;
int num_riders;
int num_servers;
int premier_num;
int share_num;
bool kill_var;

typedef struct PremierCab {
    int waitState;
    int onRidePremier;
    int pres_rider_index;
    time_t pres_wait_time;
} PC;

typedef struct ShareCab {
    int waitState;
    int onRidePoolFull;
    int onRidePoolOne;
    int pres_riders_indices[2];
    time_t wait_times[2];
} SC;

pthread_t* Rider_Threads;
pthread_t* Server_Threads;
pthread_mutex_t* Rider_mutex;
pthread_mutex_t* Server_mutex;

typedef struct Rider {
    int rider_id;
    int cabType;
    int maxWaitTime;
    int rideTime;
    int payment_status;
    int GotRide;
    int Cab;
    int share_index;
    int arrivalTime;
}R;

typedef struct Server {
    int busy;
    int pres_user;
    int pres_cab;
    int pres_cab_type;
}S;

PC* PremierCabs;                     //Cab Type 0
SC* ShareCabs;                         //Cab Type 1
R* Riders;
S* Servers;

void* ServerInit(void* argvp) {
    S* s = (S*)(argvp);

    while (true && !kill_var) {
        if (!s->busy)
            continue;
        else if (s->busy == 1 && s->pres_user != -1) {
            sleep(2);
            pthread_mutex_lock((Rider_mutex + s->pres_user));

            PC* prem;
            SC* share;
            R* r;

            r = Riders + s->pres_user;

            if (!(s->pres_cab_type -1)) {
                share = ShareCabs + s->pres_cab;
                if (share->onRidePoolOne == 1 && !share->onRidePoolFull) {
                    
                    printf("Share Cab #%lld in state wait State from onRidePoolOne\n", s->pres_cab);
                    share->pres_riders_indices[r->share_index] = -1;
                    share->wait_times[r->share_index] = -1;
                    share->onRidePoolOne *= 0;
                    share->waitState = 1;

                }
                else if (!share->onRidePoolOne && share->onRidePoolFull == 1) {
                    printf("Share Cab %lld in state onRidePoolOne from onRidePoolFull\n", s->pres_cab);
                    share->pres_riders_indices[r->share_index] = -1;
                    share->wait_times[r->share_index] = -1;
                    share->onRidePoolFull *= 0;
                    share->onRidePoolOne = 1;
                    share->waitState *= 0;


                }
            }
            else {
                printf("Premier Cab #%lld in state waitState from onRidePremier\n", s->pres_cab);
                
                prem = PremierCabs + s->pres_cab;
                prem->onRidePremier *= 0;
                prem->pres_rider_index = -1;
                prem->pres_wait_time = -1;
                prem->waitState = 1;

            }

            r->payment_status = 1;
            s->busy *= 0;

            printf("Mutex No %lld\n", s->pres_user);
            
            pthread_mutex_unlock(Rider_mutex + s->pres_user);
        }
    }
    return NULL;
}

void *RiderInit(void* argvp) {
    R* r = ((R*)argvp);
    bool flag = false;

    r->cabType = rand() % 2;
    r->maxWaitTime = rand() % 50;
    r->maxWaitTime += 10;
    r->payment_status *= 0;
    r->rideTime = rand() % 60;
    r->rideTime += 20;
    r->arrivalTime = rand() % 10;
     
    time_t in = time(NULL);
    time_t n = time(NULL);
    printf("Rider #%lld wants a cab of type #%lld after %llds for %llds and will wait for %llds at max\n", r->rider_id, r->cabType, r->arrivalTime, r->rideTime, r->maxWaitTime);


    while (0 <= r->arrivalTime + in - n) {
        n = time(NULL);
        continue;
    }

    printf("Rider #%lld has Arrived\n", r->rider_id);

    while (!(r->GotRide)) {

        time_t now;
        now = time(NULL);

        if (now - in - r->maxWaitTime > 0) {
            printf("Sorry User #%lld Timed Out\n", r->rider_id);
            return NULL;
        }


        if (!(r->cabType)) {
            pthread_mutex_lock(Rider_mutex + r->rider_id);
            for (int i = 0; i < premier_num; i++) {
                if (!((PremierCabs + i)->waitState - 1)) {
                    printf("Premier Cab #%lld in state onRidePremier from waitState\n", i);
                    r->GotRide = 1;
                    r->Cab = i;
                    
                    (PremierCabs + i)->waitState *= 0;
                    (PremierCabs + i)->pres_rider_index = r->rider_id;
                    (PremierCabs + i)->pres_wait_time = r->rideTime;
                    (PremierCabs + i)->onRidePremier = 0;
                    (PremierCabs + i)->onRidePremier++;
                    break;
                }
            
            }
            pthread_mutex_unlock(Rider_mutex + r->rider_id);
        }
        else {
            pthread_mutex_lock(Rider_mutex + r->rider_id);
            for (int i = 0; i < share_num; i++) {
                if (!((ShareCabs + i)->waitState -1) && !((ShareCabs + i)->onRidePoolOne)  && !((ShareCabs + i)->onRidePoolFull)) {
                    printf("Share Cab #%lld in state onRidePoolOne from waitState\n", i);
                    (ShareCabs + i)->wait_times[0] = r->rideTime;
                    r->GotRide = 0;
                    r->Cab = i;
                    r->share_index *= 0;
                    (ShareCabs + i)->pres_riders_indices[0] = r->rider_id;
                    (ShareCabs + i)->waitState *= 0;
                    (ShareCabs + i)->onRidePoolOne = 1;
                    r->GotRide++;
                    break;
                }
                else if (!((ShareCabs + i)->waitState) && (ShareCabs + i)->onRidePoolOne == 1 &&!((ShareCabs + i)->onRidePoolFull)) {
                    printf("Share Cab #%lld in state onRidePoolFull from onRidePoolOne\n", i);
                    (ShareCabs + i)->wait_times[1] = r->rideTime;
                    r->GotRide = 1;
                    r->Cab = i;
                    r->share_index = 1;
                    (ShareCabs + i)->pres_riders_indices[1] = r->rider_id;
                    (ShareCabs + i)->onRidePoolOne *= 0;
                    (ShareCabs + i)->onRidePoolFull = 1;

                    break;
                }
            }
            pthread_mutex_unlock(Rider_mutex + r->rider_id);

        }
    }

    time_t init = time(NULL);
    time_t now = time(NULL);
    printf("Rider #%lld took cab #%lld of type #%lld and will use it for #%llds\n", r->rider_id, r->Cab, r->cabType, r->rideTime);

    while (now - init - r->rideTime <= 0) {
        now = time(NULL);
        continue;
    }

    printf("Rider #%lld has finished in cab #%lld of type #%lld\n", r->rider_id, r->Cab, r->cabType);

    while (!(r->payment_status)) {
        int pres_server;
        bool flag1 = false;
        for (int i = 0; i < num_servers; i++) {
            if (!((Servers + i)->busy)) {
                flag1 = true;
                pthread_mutex_lock(Rider_mutex + r->rider_id);
                pres_server = i;
                
                (Servers + pres_server)->pres_user = r->rider_id;
                (Servers + pres_server)->pres_cab = r->Cab;
                printf("Rider #%lld Beginning Payment on server #%lld\n", r->rider_id, i);
                (Servers + pres_server)->pres_cab_type = r->cabType;
                (Servers + pres_server)->busy = 1;

                pthread_mutex_unlock(Rider_mutex + r->rider_id);
                break;
            }
        }
        if (flag1) {
            printf("Rider #%lld waiting for payment on server #%lld\n", r->rider_id, pres_server);
            while ((Servers + pres_server)->busy == 1 && !(r->payment_status));
            r->payment_status = 1;

            pthread_mutex_lock(Rider_mutex + r->rider_id);

            printf("Payment for rider #%lld is done\n", r->rider_id);
            (Servers + pres_server)->pres_user = -1;
            (Servers + pres_server)->pres_cab = -1;
            (Servers + pres_server)->busy *= 0;
            (Servers + pres_server)->pres_cab_type = -1;

            pthread_mutex_unlock(Rider_mutex + r->rider_id);
        }
    }
    return NULL;
}

signed main() {
    
    printf("Enter number of cabs: ");
    scanf("%lld", &num_cabs);    
    printf("Enter number of riders: ");
    scanf("%lld", &num_riders);    
    printf("Enter number of servers: ");
    scanf("%lld", &num_servers);    

    //Assuming There are equal number of premier cabs and pool cabs

    kill_var = false;

    premier_num = num_cabs / 2;
    share_num = num_cabs - (num_cabs / 2);

    PremierCabs = (PC*)malloc(premier_num * sizeof(PC));
    ShareCabs = (SC*)malloc(share_num * sizeof(SC));
    Riders = (R*)malloc(num_riders * sizeof(R));
    Servers = (S*)malloc(num_servers * sizeof(S));

    for (int i = 0; i < premier_num; i++) {
        (PremierCabs + i)->pres_rider_index = -1;
        (PremierCabs + i)->onRidePremier *= 0;
        (PremierCabs + i)->waitState = 1;
        (PremierCabs + i)->pres_wait_time = -1;
    }

    for (int i = 0; i < share_num; i++) {
        for(int j=0;j<=1;j++)
            (ShareCabs + i)->pres_riders_indices[j] = -1;
        for(int j=0;j<=1;j++)
            (ShareCabs + i)->wait_times[j] = -1;
        (ShareCabs + i)->waitState = 1;
        (ShareCabs + i)->onRidePoolFull = (ShareCabs + i)->onRidePoolOne = 0;
    }

    for (int i = 0; i < num_riders; i++) {
        (Riders + i)->cabType = (Riders + i)->maxWaitTime = (Riders + i)->Cab = -1;
        (Riders + i)->rider_id = i;
        (Riders + i)->rideTime = (Riders + i)->share_index = -1;
        (Riders + i)->GotRide = (Riders + i)->payment_status = 0;
    }

    for (int i = 0; i < num_servers; i++) {
        (Servers + i)->busy = 0;
        (Servers + i)->pres_user = (Servers + i)->pres_cab = (Servers + i)->pres_cab_type = -1;
    }

    Server_Threads = (pthread_t*)malloc(sizeof(pthread_t) * num_servers);
    Rider_Threads = (pthread_t*)malloc(sizeof(pthread_t) * num_riders);
    
    Server_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_servers);
    Rider_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_riders);


    for (int i = 0; i < num_servers; i++) {
        pthread_create(Server_Threads + i, NULL, ServerInit, (void*)(Servers + i));
        pthread_mutex_init(Server_mutex + i, NULL);
    }

    for (int i = 0; i < num_riders; i++) {
        pthread_create(Rider_Threads + i, NULL, RiderInit, (void*)(Riders + i));
        pthread_mutex_init(Rider_mutex + i, NULL);
    }
    for (int i = 0; i < num_riders; i++) {
        pthread_join(*(Rider_Threads + i), NULL);
        pthread_mutex_destroy(Rider_mutex + i);
    }

    kill_var = true;

    for (int i = 0; i < num_servers; i++) {
        pthread_join(*(Server_Threads + i), NULL);
        pthread_mutex_destroy(Server_mutex + i);
    }

    pthread_exit(NULL);
}
