#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>

struct PremierCab {
    int waitState;
    int onRidePremier;
    int pres_rider_index;
    time_t pres_wait_time;
};

struct ShareCab {
    int waitState;
    int onRidePoolFull;
    int onRidePoolOne;
    int pres_riders_indices[2];
    time_t wait_times[2];
};

struct Rider {
    int rider_id;
    int cabType;
    int maxWaitTime;
    int rideTime;
    int payment_status;
    int GotRide;
    int Cab;
    int share_index;
    int arrivalTime;
};

struct Server {
    int busy;
    int pres_user;
    int pres_cab;
    int pres_cab_type;
};

int num_cabs;
int num_riders;
int num_servers;

int premier_num;
int share_num;

int kill_var;

struct PremierCab* PremierCabs;                     //Cab Type 0
struct ShareCab* ShareCabs;                         //Cab Type 1
struct Rider* Riders;
struct Server* Servers;
pthread_t* Rider_Threads;
pthread_t* Server_Threads;
pthread_mutex_t* Rider_mutex;
pthread_mutex_t* Server_mutex;

void* ServerInit(void* argvp) {
    struct Server* s = (struct Server*)(argvp);

    while (1 && kill_var == 0) {
        if (s->busy == 0) {
            continue;
        }
        else if (s->busy == 1 && s->pres_user != -1) {
            sleep(2);
            pthread_mutex_lock((Rider_mutex + s->pres_user));

            struct PremierCab* prem;
            struct ShareCab* share;
            struct Rider* r;

            r = Riders + s->pres_user;

            if (s->pres_cab_type == 1) {
                share = ShareCabs + s->pres_cab;
                if (share->onRidePoolOne == 1 && share->onRidePoolFull == 0) {
                    share->pres_riders_indices[r->share_index] = -1;
                    share->wait_times[r->share_index] = -1;
                    share->onRidePoolOne = 0;
                    share->waitState = 1;
                    // r->payment_status = 1;

                    printf("Share Cab %d in state waitState from onRidePoolOne\n", s->pres_cab);
                }
                else if (share->onRidePoolOne == 0 && share->onRidePoolFull == 1) {
                    share->pres_riders_indices[r->share_index] = -1;
                    share->wait_times[r->share_index] = -1;
                    share->onRidePoolFull = 0;
                    share->onRidePoolOne = 1;
                    share->waitState = 0;
                    // r->payment_status = 1;

                    printf("Share Cab %d in state onRidePoolOne from onRidePoolFull\n", s->pres_cab);

                }
            }
            else {
                prem = PremierCabs + s->pres_cab;
                prem->onRidePremier = 0;
                prem->pres_rider_index = -1;
                prem->pres_wait_time = -1;
                prem->waitState = 1;
                // r->payment_status = 1;

                printf("Premier Cab %d in state waitState from onRidePremier\n", s->pres_cab);
            }
            printf("Mutex No %d\n", s->pres_user);


            r->payment_status = 1;
            s->busy = 0;

            pthread_mutex_unlock(Rider_mutex + s->pres_user);
        }
    }
    return NULL;

}

void *RiderInit(void* argvp) {
    struct Rider* r = ((struct Rider*)argvp);
    int flag = 0;

    r->cabType = rand() % 2;
    r->maxWaitTime = rand() % 50 + 10;
    r->payment_status = 0;
    r->rideTime = rand() % 60 + 20;
    r->arrivalTime = rand() % 10;
    printf("Rider %d wants a cab of type %d after %ds for %ds and will wait for %ds at max\n", r->rider_id, r->cabType, r->arrivalTime, r->rideTime, r->maxWaitTime);

    time_t in = time(NULL);

    time_t n = time(NULL);

    while (n - in <= r->arrivalTime) {
        n = time(NULL);
        continue;
    }

    printf("Rider %d has Arrived\n", r->rider_id);

    while (r->GotRide == 0) {

        time_t now = time(NULL);

        if ((now - in)> r->maxWaitTime) {
            printf("Sorry User %d Timed Out\n", r->rider_id);
            return NULL;
        }


        if (r->cabType == 0) {
            pthread_mutex_lock(Rider_mutex + r->rider_id);
            for (int i = 0; i < premier_num; i++) {
                if ((PremierCabs + i)->waitState == 1) {
                    (PremierCabs + i)->waitState = 0;
                    (PremierCabs + i)->pres_wait_time = r->rideTime;
                    (PremierCabs + i)->pres_rider_index = r->rider_id;
                    (PremierCabs + i)->onRidePremier = 1;
                    r->GotRide = 1;
                    r->Cab = i;
                    printf("Premier Cab %d in state onRidePremier from waitState\n", i);
                    break;
                }
            
            }
            pthread_mutex_unlock(Rider_mutex + r->rider_id);
        }
        else {
            pthread_mutex_lock(Rider_mutex + r->rider_id);
            for (int i = 0; i < share_num; i++) {
                if ((ShareCabs + i)->waitState == 1 && (ShareCabs + i)->onRidePoolOne == 0 && (ShareCabs + i)->onRidePoolFull == 0) {
                    (ShareCabs + i)->waitState = 0;
                    (ShareCabs + i)->wait_times[0] = r->rideTime;
                    (ShareCabs + i)->pres_riders_indices[0] = r->rider_id;
                    (ShareCabs + i)->onRidePoolOne = 1;
                    r->GotRide = 1;
                    r->Cab = i;
                    r->share_index = 0;
                    printf("Share Cab %d in state onRidePoolOne from waitState\n", i);

                    break;
                }
                else if ((ShareCabs + i)->waitState == 0 && (ShareCabs + i)->onRidePoolOne == 1 && (ShareCabs + i)->onRidePoolFull == 0) {
                    (ShareCabs + i)->wait_times[1] = r->rideTime;
                    (ShareCabs + i)->pres_riders_indices[1] = r->rider_id;
                    (ShareCabs + i)->onRidePoolOne = 0;
                    (ShareCabs + i)->onRidePoolFull = 1;
                    r->GotRide = 1;
                    r->Cab = i;
                    r->share_index = 1;
                    printf("Share Cab %d in state onRidePoolFull from onRidePoolOne\n", i);

                    break;
                }
            }
            pthread_mutex_unlock(Rider_mutex + r->rider_id);

        }
    }

    printf("Rider %d took cab %d of type %d and will use it for %ds\n", r->rider_id, r->Cab, r->cabType, r->rideTime);

    time_t init = time(NULL);
    time_t now = time(NULL);
    while ((now - init) <= r->rideTime) {
        now = time(NULL);
        continue;
    }

    printf("Rider %d has finished in cab %d of type %d\n", r->rider_id, r->Cab, r->cabType);

    while (r->payment_status == 0) {
        int pres_server, flag1 = 0;
        for (int i = 0; i < num_servers; i++) {
            if ((Servers + i)->busy == 0) {
                printf("%d Beginning Payment on server %d\n", r->rider_id, i);
                pthread_mutex_lock(Rider_mutex + r->rider_id);
                pres_server = i;
                flag1 = 1;
                (Servers + pres_server)->busy = 1;
                (Servers + pres_server)->pres_user = r->rider_id;
                (Servers + pres_server)->pres_cab = r->Cab;
                (Servers + pres_server)->pres_cab_type = r->cabType;

                pthread_mutex_unlock(Rider_mutex + r->rider_id);
                break;
            }
        }
        if (flag1 == 1) {
            printf("%d waiting for payment on server %d\n", r->rider_id, pres_server);
            while ((Servers + pres_server)->busy == 1 && r->payment_status == 0) {
                continue;
            }
            r->payment_status = 1;

            pthread_mutex_lock(Rider_mutex + r->rider_id);
            printf("Payment For %d is done\n", r->rider_id);

            (Servers + pres_server)->busy = 0;
            (Servers + pres_server)->pres_user = -1;
            (Servers + pres_server)->pres_cab = -1;
            (Servers + pres_server)->pres_cab_type = -1;
            // printf("Payment For %d is done 2 \n", r->rider_id);

            pthread_mutex_unlock(Rider_mutex + r->rider_id);
        }
    }
    return NULL;
}

int main() {
    scanf("%d %d %d", &num_cabs, &num_riders, &num_servers);

    //Assuming There are equal number of premier cabs and pool cabs

    kill_var = 0;

    premier_num = num_cabs / 2;
    share_num = num_cabs - (num_cabs / 2);

    PremierCabs = (struct PremierCab*)malloc(premier_num * sizeof(struct PremierCab));
    ShareCabs = (struct ShareCab*)malloc(share_num * sizeof(struct ShareCab));
    Riders = (struct Rider*)malloc(num_riders * sizeof(struct Rider));
    Servers = (struct Server*)malloc(num_servers * sizeof(struct Server));

    for (int i = 0; i < premier_num; i++) {
        (PremierCabs + i)->onRidePremier = 0;
        (PremierCabs + i)->waitState = 1;
        (PremierCabs + i)->pres_wait_time = -1;
        (PremierCabs + i)->pres_rider_index = -1;
    }

    for (int i = 0; i < share_num; i++) {
        (ShareCabs + i)->onRidePoolFull = 0;
        (ShareCabs + i)->onRidePoolOne = 0;
        (ShareCabs + i)->pres_riders_indices[0] = -1;
        (ShareCabs + i)->pres_riders_indices[1] = -1;
        (ShareCabs + i)->wait_times[0] = -1;
        (ShareCabs + i)->wait_times[1] = -1;
        (ShareCabs + i)->waitState = 1;
    }

    for (int i = 0; i < num_riders; i++) {
        (Riders + i)->cabType = -1;
        (Riders + i)->maxWaitTime = -1;
        (Riders + i)->payment_status = 0;
        (Riders + i)->rider_id = i;
        (Riders + i)->rideTime = -1;
        (Riders + i)->GotRide = 0;
        (Riders + i)->Cab = -1;
        (Riders + i)->share_index = -1;
    }

    for (int i = 0; i < num_servers; i++) {
        (Servers + i)->busy = 0;
        (Servers + i)->pres_user = -1;
        (Servers + i)->pres_cab = -1;
        (Servers + i)->pres_cab_type = -1;
    }

    Rider_Threads = (pthread_t*)malloc(sizeof(pthread_t) * num_riders);
    Rider_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_riders);

    Server_Threads = (pthread_t*)malloc(sizeof(pthread_t) * num_servers);
    Server_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_servers);

    for (int i = 0; i < num_riders; i++) {
        pthread_create(Rider_Threads + i, NULL, RiderInit, (void*)(Riders + i));
        pthread_mutex_init(Rider_mutex + i, NULL);
    }

    for (int i = 0; i < num_servers; i++) {
        pthread_create(Server_Threads + i, NULL, ServerInit, (void*)(Servers + i));
        pthread_mutex_init(Server_mutex + i, NULL);
    }

    // printf("HAHA\n");

    for (int i = 0; i < num_riders; i++) {
        pthread_join(*(Rider_Threads + i), NULL);
        pthread_mutex_destroy(Rider_mutex + i);
    }
    // printf("HAHA\n");

    kill_var = 1;

    for (int i = 0; i < num_servers; i++) {
        pthread_join(*(Server_Threads + i), NULL);
        pthread_mutex_destroy(Server_mutex + i);
    }

    pthread_exit(NULL);
}