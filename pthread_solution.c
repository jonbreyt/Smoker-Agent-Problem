#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void *smoker(void *arg);
void *agent();

pthread_t agent_process_thread, paper_process_thread, tobacco_process_thread, matches_process_thread; // initializing processes(threads)

// initializing mutex locks
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t agent_mutex_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t paper_mutex_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tobacco_mutex_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matches_mutex_lock = PTHREAD_MUTEX_INITIALIZER;

void *smoker(void *arg){
    // for converting string character into an integer
    const char *p_ID = (char *)arg;
    int ID = atoi(p_ID);
    while (1){
        if(ID == 0){
            pthread_mutex_lock(&paper_mutex_lock); // smoker with paper goes to sleep
        }
        else if(ID == 1){
            pthread_mutex_lock(&tobacco_mutex_lock); // smoker with tobacco goes to sleep
        }
        else if(ID == 2){
            pthread_mutex_lock(&matches_mutex_lock); // smoker with matches goes to sleep
        }

        pthread_mutex_lock(&mutex_lock); // lock critical section

        if(ID == 0){ // smoker process with paper rolls cigarette
            printf("smoker with paper used the agent's tobacco and matches and has now rolled a cigarette!\n");
        }
        else if(ID == 1){ // smoker process with tobacco rolls cigarette
            printf("smoker with tobacco used the agent's paper and matches and has now rolled a cigarette!\n");
        }
        else if(ID == 2){ // smoker process with matches rolls cigarette
            printf("smoker with matches used the agent's tobacco and paper and has now rolled a cigarette!\n");
        }

        pthread_mutex_unlock(&agent_mutex_lock); // unlock agent process to distribute more ingredients
        pthread_mutex_unlock(&mutex_lock); // unlock mutex on critical section
    }
}

void *agent(){
    for(int i = 0; i < 10; i++){ // Agent distributes ingredients 10 times
        pthread_mutex_lock(&mutex_lock); // mutex lock on critical section
        int random_ID_number = rand() % 3; // selecting a random number to decide which smoker process agent wakes up
        if(random_ID_number == 0){ // if 0, agent distributes two ingredients and wakes smoker process with paper
            printf("Agent has put out tobacco along with matches and proceeds to wake up the smoker with paper.\n");
            pthread_mutex_unlock(&paper_mutex_lock);
        }
        else if(random_ID_number == 1){ // if 1, agent distributes two ingredients and wakes smoker process with tobacco
            printf("Agent has put out paper along with matches and proceeds to wake up the smoker with tobacco.\n");
            pthread_mutex_unlock(&tobacco_mutex_lock);
        }
        else if(random_ID_number == 2){ // if 2, agent distributes two ingredients and wakes smoker process with matches
            printf("Agent has put out tobacco along with paper and proceeds to wake up the smoker with matches.\n");
            pthread_mutex_unlock(&matches_mutex_lock);
        }
        // to keep track of number of times agents distributes ingredients to smoker processes
        int ingredients_left = 9 - i;
        printf("Remaining Ingredient Distributions = %d\n", ingredients_left);
        pthread_mutex_unlock(&mutex_lock); // release lock on critical section so smoker process can roll cigarette
        pthread_mutex_lock(&agent_mutex_lock); // lock agent process
    }
    printf("The smokers are waiting for the agent to give more ingredients!\n");
    printf("The Agent, however, is finished giving out ingredients :(\n");
    // cancelling smoker process threads when ingredients run out
    pthread_mutex_lock(&mutex_lock);
    pthread_cancel(&paper_process_thread);
    pthread_cancel(&tobacco_process_thread);
    pthread_cancel(&matches_process_thread);
    pthread_mutex_unlock(&mutex_lock);
    printf("All smoker threads have now been cancelled\n");
}

int main(int argc, char* argv[]){
    srand(time(NULL));

    // assigning a value to each of the smoker processes for execution
    char *paper_ID = "0";
    char *tobacco_ID = "1";
    char *matches_ID = "2"; 

    // initially keeping all the processes locked
    pthread_mutex_lock(&agent_mutex_lock);
    pthread_mutex_lock(&paper_mutex_lock);
    pthread_mutex_lock(&tobacco_mutex_lock);
    pthread_mutex_lock(&matches_mutex_lock);

    // creating the p-threads for each process
    pthread_create(&agent_process_thread, NULL, agent, NULL);
    pthread_create(&paper_process_thread, NULL, smoker, (void *)paper_ID);
    pthread_create(&tobacco_process_thread, NULL, smoker, (void *)tobacco_ID);
    pthread_create(&matches_process_thread, NULL, smoker, (void *)matches_ID);
    
    // waiting for agent thread to complete execution
    pthread_join(agent_process_thread, NULL);

    exit(0);
}