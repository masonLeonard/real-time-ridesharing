/* Mason Leonard 818030805
 * CS 480
 * Professor Ben Shen
 * Assignment 4: Main.c includes
 * 4/12/2022
 *
 * Bird's eye view of the program.
 *
 * Code that is not my own:
 * Queue structure
 *     Source: https://www.geeksforgeeks.org/queue-set-1introduction-
 *     and-array-implementation/
 * mssleep()
 *     Source: https://stackoverflow.com/questions/7684359/how-to-use-
 *     nanosleep-in-c-what-are-tim-tv-sec-and-tim-tv-nsec/33412960#33412960
 */
#define _GNU_SOURCE

#define HUMAN 4
#define ROBOT 5
#define MAXREQUESTS 12
#define TYPEN 2

#include <stdlib.h>      //NULL, EXIT SUCCESS/FAILURE, strtol, free, calloc
#include <stdio.h>       //printf
#include <getopt.h>      //getopt logic
#include <pthread.h>     //pthrad logic
#include <sys/types.h>   //get thread id
#include <unistd.h>      //get thread id
#include <sys/syscall.h> //get thread id
#include <stdbool.h>     //bool type
#include <semaphore.h>
#include <time.h>        //nanosleep
#include <limits.h>      //INT_MIN

/* Instantiated N times to simulate a toy ride sharing service
 * "Produces" requests from riders in a repeated fashion
 */
void *producerStart(void *arg);

/* Instantiated N times to simulate a toy ride sharing service
 * "Consumes" requests from riders in a repeated fashion
 */
void *consumerStart(void *arg);

int nanosleep(const struct timespec *req, struct timespec *rem);

int mssleep(long miliseconds);

struct Queue* createQueue(unsigned capacity);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int isEmpty(struct Queue* queue);

#ifndef RIDESHARING_H
#define RIDESHARING_H


/* Common data struct for the producers and consumers to collaborate
 *
 */
typedef struct bufferDS
{
    struct Queue *queue;
    bool producerFlag, consumerFlag;
    sem_t lock;        //force order of operations for pthread_create
    sem_t mainLock;    //force main to wait for threads
    sem_t availableSlots; //empty
    sem_t unconsumed;     //full
    sem_t availableHumanSlots;    //restrict number of human drivers
    sem_t mutex;       //force exclusivity when accessing bufferDS

    int inRequestQueue[TYPEN];
    int produced[TYPEN];
    int consumed[TYPEN];
    int *rideSummary[TYPEN];

    int valN, valC, valF, valH, valA;

    int requestCount; //compare to valN to determine if all requests are done
    bool p1Finished, p2Finished, c1Finished, c2Finished;

} bufferDS;

/*
 * Arrays with producer and consumer names
 * These can be indexed with the enumerated types below
 * and are defined in io.c
 */
extern const char *producerNames[];
extern const char *producerAbbrevs[];  
extern const char *consumerNames[]; 

/*
 * Enumerated types to be used by the producers and consumers
 * These are expected in the input/output functions (io.h)
 * should be useful in your producer and consumer code.
 */

/**
 * The broker can hold up to a maximum of 12 undispatched requests 
 * in its request queue at any given time.
 * To promote the use of autonomous cars (as human drivers cost more), 
 * the broker would hold no more than 4 undispatched requests for a human driver at any given time. 
*/

/*
 * Two rider request services (producers of requests) are offered: 
 *   one for publishing (producing) a ride request for a human driver, 
 *   one for publishing (producing) a ride request for an autonomous car.
 * Each ride request service (producer) only publishes (produces) 
 * one type of requests,
 * so RequestType is synomonous with the producer type
 */
typedef enum Requests {
  HumanDriver = 0,   // ride with a human driver
  RoboDriver = 1,  // ride with an autonomous car 
  RequestTypeN = 2,   // number of ride request types
} RequestType;

/* 
 * Two dispatcher services (consumers of requests) are available using 
 * different driver search algorithms
 *   one uses a cost saving algorithm to find a nearby driver
 *   one uses a fast-matching algorithm to find a nearby driver 
 *   Requests are taken off from the broker request queue (by dispatchers) 
 *   in the order that they are published (produced).
*/
typedef enum Consumers {
  CostAlgoDispatch = 0,   // dispatch to a nearby driver based on cost
  FastAlgoDispatch = 1,  // dispatch to a nearby driver based on fast matching 
  ConsumerTypeN = 2,   // Number of consumers
} ConsumerType;

#endif
