#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Gets a random number
float getRandTime() {
    return((rand() % 9000) + 1000.0);
}

// Struct that holds info about each philosopher and the mutexes
struct phil {
    int numPhils;
    int id;
    int meals;
    int numTries;
    pthread_mutex_t* muts;
};

// Threaded function that controls the eating of the philosophers
void* doTheEat(void* args) {
    struct phil* data = (struct phil*)args;
    // Forks that correspond to the mutexes that the philosopher needs to use to eat
    // This is based on their id
    int fork1;
    int fork2;
    // If this is the first philosopher
    if (data->id == 0) {
        fork1 = (data->numPhils) - 1;
        fork2 = data->id;
    }
    // Otherwise these are the forks needed
    else {
        fork1 = ((data->id) - 1);
        fork2 = data->id;
    }

    // Eat until all the meals are gone
    while (data->meals != 0) {
        // Try to lock both the forks, otherwise sleep and release and forks they have
        // Always increase the amount of tries
        if(pthread_mutex_trylock(&(data->muts[fork1])) == 0) {
            if(pthread_mutex_trylock(&(data->muts[fork2])) == 0) {
                data->meals--;
                usleep(getRandTime());
                pthread_mutex_unlock(&(data->muts[fork2]));
                pthread_mutex_unlock(&(data->muts[fork1]));
                data->numTries++;
            }
            else {
                data->numTries++;
                pthread_mutex_unlock(&(data->muts[fork1]));
                usleep(getRandTime());
            }
        }
        else {
            data->numTries++;
            usleep(getRandTime());
        }
    }
    return NULL;
}

float dine_philosophers_dine(int numPhilosophers, int mealSize)
{
    // Setting up random number generator
    time_t t;
    srand((unsigned) time(&t));
   
    // Variables to calculate tries with
    float totalTries = 0;
    int tries = 0;
    // Struct list
    struct phil s_list[numPhilosophers];
    // Thread list
    pthread_t tID[numPhilosophers];
    // Mutex list
    pthread_mutex_t mutList[numPhilosophers];
    for (int i = 0; i < numPhilosophers; i++) {
        // Create mutextes
        pthread_mutex_init(&(mutList[i]), NULL);
        // Create struct 
        s_list[i].numPhils = numPhilosophers;
        s_list[i].id = i;
        s_list[i].meals = mealSize;
        s_list[i].muts = mutList;
        s_list[i].numTries = tries;
        // Launch threads
        pthread_create(&(tID[i]), NULL, doTheEat, &(s_list[i])); 
    }

    // Joining the threads
    for (int i = 0; i < numPhilosophers; i++) {
        pthread_join(tID[i], NULL);
    }

    // Destroy the mutexes
    for (int i = 0; i < numPhilosophers; i++) {
        pthread_mutex_destroy(&(mutList[i]));
    }
    
    // Grab the total tries from each struct
    for (int i = 0; i < numPhilosophers; i++) {
        totalTries += s_list[i].numTries;
    }
    // Do the math to find the ratio
    float denom = numPhilosophers * mealSize;
    return totalTries/denom;
}
