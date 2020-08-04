/* Author : Mahmudul Hossain (19303235)
 * Purpose : Header file for lift_sim_B.c
 * Date Modified : 10/05/2020
 */


#include <pthread.h>
#include <semaphore.h>

#define FALSE 0
#define TRUE !FALSE

//Stores valid information of each 
//request stored in the buffer
typedef struct Req
{
	int requestNo;
	int source;
	int destination;
}Req;

//Consumer lift which is identified
//by liftID, keeping track of its 
//current floor and total number of requests
////and movement carried out

typedef struct Elevator
{
	int liftID;
	int currentFloor;
	int sumRequest;
	int sumMovement;
}Elevator;


//Semaphore struct to store all the semaphores shared
//among the processes
typedef struct Semaphore
{
	sem_t full;
	sem_t empty;
	sem_t mutex;

}Semaphore;	

//Store the shared variables that are manipulated
//in the critical section of the processes
typedef struct SharedData
{
	int in;
	int out;
	int operate;
}SharedData;


void request(Req* array, Semaphore* condition, SharedData* count, Req* buffer); //Producer
void lift(Elevator* inLift, Semaphore* condition, SharedData* count, Req* buffer); //Consumer
int initialiseSemaphores(Semaphore* s);
int destroySemaphores(Semaphore* s);
