#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "sem.h"

int main(int argc, char* argv[]){
    srand(time(NULL));

    // declaring semaphores for each of the four processes as well as a "mutex" lock
    int mutex_lock = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT); // semaphore that will serve as lock for critical section
    int agent_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    int paper_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    int tobacco_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    int matches_semaphore = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if((mutex_lock || agent_semaphore || paper_semaphore || tobacco_semaphore || matches_semaphore) == -1){
        printf("ERROR: CANNOT CREATE SEMAPHORE!\n"); // incase any of the sempahores fail to be created
    }
    else{
        // creating and initializing each of the semaphores for the processes and lock
        sem_create(mutex_lock, 1); // lock is initialized with value of 1 -- meaning its unlocked
        // process semaphores are intialized with 0 -- meaning they are locked
        sem_create(agent_semaphore, 0);
        sem_create(paper_semaphore, 0);
        sem_create(tobacco_semaphore, 0);
        sem_create(matches_semaphore, 0);
    }

    pid_t paper_pid = fork(); // fork the smoker process that has paper
    if(paper_pid == -1){
        printf("ERROR: fork failed\n");
    }
    else if(paper_pid == 0){
        while(1){
            P(paper_semaphore); // wait operation -- for smoker process with paper to complete
            P(mutex_lock); // lock on critical section
            printf("smoker with paper used the agent's tobacco and matches and has now rolled a cigarette!\n"); // smoker with paper rolls cigarette
            V(agent_semaphore); // signal agent process so it can distribute ingredients
            V(mutex_lock); // release lock on critical section 
        }
    }

    pid_t tobacco_pid = fork(); // fork the smoker process that has tobacco
    if(tobacco_pid == -1){
        printf("ERROR: fork failed\n");
    }
    else if(tobacco_pid == 0){
        while(1){
            P(tobacco_semaphore); // wait operation -- for smoker process with tobacco to complete
            P(mutex_lock); // lock on critical section
            printf("smoker with tobacco used the agent's paper and matches and has now rolled a cigarette!\n"); // smoker with tobacco rolls cigarette
            V(agent_semaphore); // signal agent process so it can distribute ingredients
            V(mutex_lock); // release lock on critical section
        }
    }

    pid_t matches_pid = fork(); // fork the smoker process that has matches
    if(matches_pid == -1){
        printf("ERROR: fork failed\n");
    }
    else if(matches_pid == 0){
        while(1){
            P(matches_semaphore); // wait operation -- for smoker process with matches to complete
            P(mutex_lock); // lock on critical section
            printf("smoker with matches used the agent's tobacco and paper and has now rolled a cigarette!\n"); // smoker with matches rolls cigarette
            V(agent_semaphore); // signal agent process so it can distribute ingredients
            V(mutex_lock); // release lock on critical section
        }
    }

    if(fork() == 0){ // if process is forked then the agent process will first distribute ingredients
        for(int i = 0; i < 10; i++){ // Agent distributes ingredients 10 times
            P(mutex_lock); // wait -- lock critical section so only agent can distribute ingredients and wake up processes
            int random_ID_number = rand() % 3;
            if(random_ID_number == 0){
                printf("Agent has put out tobacco along with matches and proceeds to wake up the smoker with paper.\n");
                V(paper_semaphore); // signal smoker process with paper
            }
            else if(random_ID_number == 1){
                printf("Agent has put out paper along with matches and proceeds to wake up the smoker with tobacco.\n");
                V(tobacco_semaphore); // signal smoker process with tobacco
            }
            else if(random_ID_number == 2){
                printf("Agent has put out tobacco along with paper and proceeds to wake up the smoker with matches.\n");
                V(matches_semaphore); // signal smoker process with matches
            }
            // keeping track of remaining ingredient distributions
            int ingredients_left = 9 - i;
            printf("Remaining Ingredients = %d\n", ingredients_left);
            V(mutex_lock); 
            P(agent_semaphore);
        }
        printf("The smokers are waiting for the agent to give more ingredients!\n");
        printf("The Agent, however is done giving ingredients :(\n");
        P(mutex_lock);
        // cancelling/terminating smoker processes when ingredients run out
        kill(paper_pid, SIGTERM);
        kill(tobacco_pid, SIGTERM);
        kill(matches_pid, SIGTERM);
        V(mutex_lock);
        printf("All smoker threads have now been terminated/cancelled\n");
        exit(0);        
    }
    wait(NULL);
    exit(0);
}