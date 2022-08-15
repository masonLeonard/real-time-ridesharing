/* Mason Leonard 818030805
 * CS 480
 * Professor Ben Shen
 * Assignment 4: Producer Thread Logic
 * 4/09/2022
 *
 * producer.c
 * Requests are generated.
 * Producers populate a common buffer.
 * Relevant semaphore is upped and downed before and after processing
 */

#include "ridesharing.h"
#include "io.h"

void *producerStart(void *arg)
{
/////////////////////WORKING DATA////////////////////////////////
    struct bufferDS *bds = arg;
    
    RequestType rHuman = HumanDriver; //zero
    RequestType rRobo = RoboDriver;   //one
    
    //Discern which particular thread has been invoked
    if(!bds->producerFlag) //first producer accesses this code when false
    {
	bds->producerFlag = true; //prepare for second producer
	sem_post(&bds->lock);     //unlock semaphore for second producer

	while(true)
	{
	    mssleep(bds->valH); //sleep for given amount to produce request
   
            sem_wait(&bds->availableHumanSlots); //attempt to queue max 4 HUMAN
            sem_wait(&bds->availableSlots); //make sure we have room
           
	    //Critical Section Start
            sem_wait(&bds->mutex); //access buffer exclusively

	    if(bds->requestCount == bds->valN) //work finished
	    {
	        //return mutex
		//exit
		sem_post(&bds->mutex);
		break;
	    }
	    bds->requestCount++;
	    enqueue(bds->queue, HUMAN);
	    bds->inRequestQueue[rHuman]++;
	    bds->produced[rHuman]++;
            io_add_type(rHuman, bds->inRequestQueue, bds->produced);
            sem_post(&bds->mutex);
            //Critical Section End

	    //inform consumer
	    sem_post(&bds->unconsumed);
	}
    }
    else //second producer accesses this code once flag is true 
    {
	sem_post(&bds->lock); //unlock for consumer threads to activate
	
	while(true)
        {
            //autonomous drivers have no constraint compared to humans

            mssleep(bds->valA); //sleep for given amount to produce request

            //make sure we have room
            sem_wait(&bds->availableSlots);

            //access buffer exclusively
            sem_wait(&bds->mutex);   //CRITICAL SECTION START

            if(bds->requestCount == bds->valN)
	    {
	        sem_post(&bds->mutex);
		break;
	    }

            bds->requestCount++;
            enqueue(bds->queue, ROBOT);
            bds->inRequestQueue[rRobo]++;
            bds->produced[rRobo]++;
            io_add_type(rRobo, bds->inRequestQueue, bds->produced);
            sem_post(&bds->mutex); //CRITICAL SECTION END

            //inform consumer
            sem_post(&bds->unconsumed);
        }
    }
}
