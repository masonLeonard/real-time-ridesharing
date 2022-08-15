/* Mason Leonard 818030805
 * CS 480
 * Professor Ben Shen
 * Assignment 4: Consumer Thread Logic
 * 4/09/2022
 *
 * consumer.c
 * Consumers rely on a populated buffer.
 * Buffer contents are manipulated.
 * Relevant semaphore is upped and downed before and after processing
 */

#include "ridesharing.h"
#include "io.h"

void *consumerStart(void *arg)
{
/////////////////////WORKING DATA////////////////////////////////
    struct bufferDS *bds = arg;

    ConsumerType cCost = CostAlgoDispatch; //zero
    ConsumerType cFast = FastAlgoDispatch; //one
    RequestType rHuman = HumanDriver;      //zero
    RequestType rRobo = RoboDriver;        //one

    //Discern which particular thread has been invoked
    if(!bds->consumerFlag) //first consumer has accessed the start function
    {
	bds->consumerFlag = true;
	sem_post(&bds->lock);
        
	while(true)
	{
	    mssleep(bds->valC);

            //block until something to consume
	    sem_wait(&bds->unconsumed);

	    //access buffer exclusively
	    sem_wait(&bds->mutex); //CRITICAL SECTION START
            if(isEmpty(bds->queue))
	    {
	        //nothing in queue, do not decrement
		sem_post(&bds->mutex);

	    }
	    else if(front(bds->queue) == HUMAN)
	    {
	        //HUMAN request about to be removed
		bds->rideSummary[cCost][rHuman]++;
		dequeue(bds->queue);
                bds->inRequestQueue[rHuman]--;
                bds->consumed[rHuman]++;
		io_remove_type(cCost, rHuman, bds->inRequestQueue, bds->consumed);
		sem_post(&bds->mutex); //CRITICAL SECTION END

		//inform producer
		sem_post(&bds->availableHumanSlots);
		sem_post(&bds->availableSlots);
	    }
	    else
	    {
	        //ROBOT request about to be removed
                //update rideSummary accordingly

                bds->rideSummary[cCost][rRobo]++;
                dequeue(bds->queue);
                bds->inRequestQueue[rRobo]--;
                bds->consumed[rRobo]++;
                io_remove_type(cCost, rRobo, bds->inRequestQueue, bds->consumed);
                sem_post(&bds->mutex); //CRITICAL SECTION END

                //inform producer
                sem_post(&bds->availableSlots);
	    }

            if(bds->requestCount == bds->valN)
            {
                bds->c1Finished = true;
                break;
            }
	}
    }
    else //second consumer has accessed the start function
    {
        //keep main from exiting with this lock
	sem_post(&bds->lock);

	while(true)
	{
	    mssleep(bds->valF);

            //block until something to consume
            sem_wait(&bds->unconsumed);

            //access buffer exclusively
            sem_wait(&bds->mutex); //CRITICAL SECTION START
            if(isEmpty(bds->queue))
            {
                //nothing in queue, do not decrement
                sem_post(&bds->mutex);

            }
            else if(front(bds->queue) == HUMAN)
            {
                //HUMAN request about to be removed
                //update rideSummary accordingly

                bds->rideSummary[cFast][rHuman]++;
                dequeue(bds->queue);
                bds->inRequestQueue[rHuman]--;
                bds->consumed[rHuman]++;
                io_remove_type(cFast, rHuman, bds->inRequestQueue, bds->consumed);
                sem_post(&bds->mutex); //CRITICAL SECTION END

                //inform producer
		sem_post(&bds->availableHumanSlots);
                sem_post(&bds->availableSlots);
            }
            else
            {
                //ROBOT request about to be removed
                //update rideSummary accordingly

                bds->rideSummary[cFast][rRobo]++;
                dequeue(bds->queue);
                bds->inRequestQueue[rRobo]--;
                bds->consumed[rRobo]++;
                io_remove_type(cFast, rRobo, bds->inRequestQueue, bds->consumed);
                sem_post(&bds->mutex); //CRITICAL SECTION END

                //inform producer
                sem_post(&bds->availableSlots);
            }

            if(bds->requestCount == bds->valN)
            {
                bds->c2Finished = true;
                break;
            }
	}
    }

    if(bds->c1Finished || bds->c2Finished)
    {
       //return lock so main can finish and print summary
       sem_post(&bds->mainLock);
    }
}
