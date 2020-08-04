/* Author : Mahmudul Hossain (1930325)
 * Purpose : Headfile for lift_sim_A.c
 * Date Modified : 09/05/2020
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>


#define FALSE 0
#define TRUE !FALSE

//Store requests read from file
//to be operated by each lift, 
//each request indentified by requestNo	
typedef struct Req
{
	int requestNo;
	int source;
	int destination;
}Req;


//Consumer lift which is identified
//by liftID, keeping track of its 
//current floor and total number of requests
//and movement carried out
typedef struct Elevator
{
	int liftID;
	int currentFloor;
	int sumRequest;
	int sumMovement;
}Elevator;
	
void* request(void* param); //Producer function
void* lift(void* param); //Consumer function



/*
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t element = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t service = PTHREAD_COND_INITIALIZER;

pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//Number of elements in buffer
int counter = 0;
//Keep track of elements in buffer
int in = 0, out = 0;
//bool finished = false;

int sleeptime;

int BUFFERSIZE;

int requestsInFile;

//int lift1, lift2, lift3;

//Buffer of requests
Req* buffer;

int index = 0;
int operate = 0;
*/
