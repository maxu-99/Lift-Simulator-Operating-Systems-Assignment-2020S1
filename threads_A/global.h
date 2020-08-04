/* Author : Mahmudul Hossain (19303235)
 * Purpose : Store all the global variables to be used in
 * 			 lift_sim_A
 * Date Modified : 09/05/2020
 */

#include <pthread.h>

//Mutex lock to ensure mutual exclusion between threads
//Prevent posible race conditions
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Conditional variables to allow mutex to be release
//when required

//Pthread condition to signal/wait that a request has been
//serviced
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;


//Pthread condition to signal/wait that a request has been 
//added to the buffer
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

//Number of elements in buffer
int counter = 0;

//Keep next free and occupied position in buffer 
int in = 0, out = 0;

//Store the time to sleep for each consumer
//thread
int sleeptime;

//Size of the buffer
int BUFFERSIZE;

//Number of requests read from file
int requestsInFile;

//Buffer of requests shared by threads
Req* buffer;

//Store the total number of lift operations carried
//out among the lifts
int operate = 0;

