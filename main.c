/* Mason Leonard 818030805
 * CS 480
 * Professor Ben Shen
 * Assignment 4: Rideshare program
 * 4/09/2022
 *
 * Main handles the cooperation between requests and consumers.
 * Main prints to screen appropriate output.
 * Main ...
 */

#include "ridesharing.h"
#include "io.h"

int main(int argc, char *argv[])
{
    int opt, valN, valC, valF, valH, valA;
    bool boolN = false, boolC = false, boolF = false;
    bool boolH = false, boolA = false;
///////////////////COMMAND LINE ARGS////////////////////////////
    while((opt = getopt(argc, argv, "n:c:f:h:a:")) != -1)
    {
        switch(opt)
        {
            //Total number of requests (production limit). 
            //Default is 120 if not specified.
            case 'n':
                valN = strtol(optarg, NULL, 10);
                boolN = true;
                break;
            
            //Specifies the number of milliseconds N that the cost-saving
            //dispatcher (consumer) requires dispatching a request and should
            //be invoked each time the cost-saving dispatcher removes a request
            //from queue regardless of the request type.
            //You would simulate this time to consume a request by putting
            //the consumer thread to sleep for N milliseconds
            //Default value is 0 if not specified.
            case 'c':
                valC = strtol(optarg, NULL, 10);
                boolC = true;
                break;

            //Similar argument for the fast-matching dispatcher.
            //Default value is 0 if not specified.
            case 'f':
                valF = strtol(optarg, NULL, 10);
                boolF = true;
                break;
            
            //Specifies the number of milliseconds required to produce
            //a ride request for a human driver.
            //Default value is 0 if not specified.
            case 'h':
                valH = strtol(optarg, NULL, 10);
                boolH = true;
                break;

            //Specifies the number of milliseconds required to produce
	    //a ride request for an autonomous car
	    //Default value is 0 if not specified.
            case 'a':
                valA = strtol(optarg, NULL, 10);
		boolA = true;
                break;

            default:
                printf("Usage: %s n:c:f:h:a:\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    //set uninitialized args to default value
    if(!boolN)
        valN = 120;
    if(!boolC)
        valC = 0;
    if(!boolF)
        valF = 0;
    if(!boolH)
        valH = 0;
    if(!boolA)
        valA = 0;
///////////////////SHARED DATA FOR THREADS////////////////////////////
    int s; //handle return code for error checking
    struct bufferDS *bds;
    bds = calloc(1, sizeof(struct bufferDS));
    if(bds == NULL) {
        printf("Problem with calloc\n");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < 2; i++)
    {
        bds->rideSummary[i] = (int *)malloc(2 * sizeof(int));
    }

    sem_init(&bds->lock, 0, 1); //binary semaphore, 0/1 only
    sem_init(&bds->availableHumanSlots, 0, HUMAN);
    sem_init(&bds->availableSlots, 0, MAXREQUESTS);
    sem_init(&bds->unconsumed, 0, MAXREQUESTS);
    sem_init(&bds->mutex, 0, 1); //maintain buffer exclusivity
    sem_init(&bds->mainLock, 0, 0); //immediately wait until condition filled

    struct Queue* queue = createQueue(MAXREQUESTS);
    bds->queue = queue;

    bds->producerFlag = false; //thread start functions know whos who
    bds->consumerFlag = false;

    bds->valN = valN; //request number limit
    bds->valC = valC; //cost saving wait time in ms
    bds->valF = valF; //fast matching wait time in ms
    bds->valH = valH; //human request wait time in ms
    bds->valA = valA; //autonomous request wait time in ms

    //@CLEANUP: possible unnecessary or could be done a better way
    //c1/c2Finished, would a semphore be simpler
    //requestCount, one of the bds arrays maybe has this number...
    bds->c1Finished = false; //has the cost consumer thread finished?
    bds->c2Finished = false; //has the fast consumer thread finished?
    bds->requestCount = 0; //initially zero requests have been filled
    
///////////////////SPAWN THREADS FROM FATHER MAIN////////////////////////////
    pthread_t producerWorker, consumerWorker;
    pthread_t producerWorker2, consumerWorker2;

    sem_wait(&bds->lock); //first producer accesses producerStart
    s = pthread_create(&producerWorker, NULL, &producerStart, (void *) bds);
    if(s != 0) {
        printf("producer: pthread_create error\n");
	exit(EXIT_FAILURE);
    }

    sem_wait(&bds->lock); //second producer accesses producerStart
    s = pthread_create(&producerWorker2, NULL, &producerStart, (void *) bds);
    if(s != 0) {
        printf("producer: pthread_create error\n");
        exit(EXIT_FAILURE);
    }

    sem_wait(&bds->lock); //first consumer accesses consumerStart
    s = pthread_create(&consumerWorker, NULL, &consumerStart, (void *) bds);
    if(s != 0) {
        printf("consumer: pthread_create error\n");
        exit(EXIT_FAILURE);
    }

    sem_wait(&bds->lock); //second consumer accesses consumerStart
    s = pthread_create(&consumerWorker2, NULL, &consumerStart, (void *) bds);
    if(s != 0) {
        printf("consumer: pthread_create error\n");
        exit(EXIT_FAILURE);
    }
////////////////////MAIN WAITS FOR CONSUMERS//////////////////////////

    //wait until everythings been used up, at the end of the consuming process
    sem_wait(&bds->mainLock);

////////////////////REJOIN THREADS WITH MOTHER MAIN////////////////////////////
/*
    s = pthread_join(producerWorker, NULL);
    if(s != 0) {
        printf("producer1: pthread_join error\n");
        exit(EXIT_FAILURE);
    }
    printf("p1 joined\n");

    s = pthread_join(producerWorker2, NULL);
    if(s != 0) {
        printf("producer2: pthread_join error\n");
        exit(EXIT_FAILURE);
    }
    printf("p2 joined\n");

    s = pthread_join(consumerWorker, NULL);
    if(s != 0) {
        printf("consumer1: pthread_join error\n");
        exit(EXIT_FAILURE);
    }
    printf("c1 joined\n");

    s = pthread_join(consumerWorker2, NULL);
    if(s != 0) {
        printf("consumer2: pthread_join error\n");
        exit(EXIT_FAILURE);
    }
    printf("c2 joined\n");
*/
////////////////////FINAL STEPS////////////////////////////

    //output summary
    io_production_report(bds->produced, bds->rideSummary);

    //give back memory
    free(bds);

    for(int i = 0; i < 2; i++)
    {
        free(bds->rideSummary[i]);
    }

    //printf("main: end\n");
    return EXIT_SUCCESS;
}
