/* Author : Mahmudul Hossain (19303235)
 * Purpose : Lift simulator using pthreads
 * 			 using a bounded buffer
 * Date Modified : 09/05/2020
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>

#include "file.h"
#include "global.h"

int main (int argc, char** argv)
{
	//If 3 arguments passed ./exec <buffer_size> <sleep_time>
	if(argc == 3)
	{
		//Buffer size needs to be > 0 and sleeptime >= 0
		if(atoi(argv[1]) <= 0 || atoi(argv[2]) < 0)
		{
			fprintf(stderr, "Invalid arguments passed\n");
			return -1;
		}

		//Check that the input file is valid and update requestsInFile
		//with the number of requests stored in file
		if(!checkValidFile("sim_input", &requestsInFile))
		{
			fprintf(stderr, "Invalid file format :\n ");	
			return -1;
		}
		
		//Assign valid parameters to variables
		sleeptime = atoi(argv[2]);
		BUFFERSIZE = atoi(argv[1]);

		//Allocate the bounded buffer
		buffer = (Req*)malloc(BUFFERSIZE * sizeof(Req));

		//Temporary array to store requests that are to be loaded into
		//the buffer
		Req array[requestsInFile];

		//Update the array with all the requests stored in the file
		getAllReqFile("sim_input",requestsInFile, array);

		//Error checking to determine if threads perform correctly
		int error;

		//Producer thread that will perform request()
		pthread_t liftR; 
			
		//Consumer thread operating lift()
		pthread_t liftID[3]; //3 lift threads

		Elevator consumer[3]; //Lift 1, 2, 3 


		//Initialise lifts where all lifts start from floor 1;
		for(int i = 0; i < 3; i++)
		{
			consumer[i].liftID = i + 1;
			consumer[i].currentFloor = 1;	
			consumer[i].sumRequest = 0;
			consumer[i].sumMovement = 0;
		}
		//Run producer thread by creating it running request()
		error = pthread_create(&liftR, NULL, request, &array);
		if(error != 0)
		{
			//If an error occurs while creating, output error message and exit
			fprintf(stderr, "Error creating liftR pthread : %s \n", strerror(error));
			return -1;
		}

		//Run consumer (lift) threads
		for(int i = 0; i < 3; i++)
		{
			//Create consumer thread running lift()
			error = pthread_create(&liftID[i], NULL, lift, &consumer[i]);
			if(error != 0)
			{
				//If an error occurs while creating, output error message and exit
				fprintf(stderr, "Error creating consumer pthread : %s \n", strerror(error));
				return -1;
			}
		}


		//Join consumer thread
		for(int i = 2; i >= 0; i--)
		{
			error = pthread_join(liftID[i], NULL);
			if(error != 0)
			{
				//If an error occurs while joining, output error message and exit
				fprintf(stderr, "Error joining consumer pthread : %s \n", strerror(error));
				return -1;
			}

		}
		//Join producer thread
		error = pthread_join(liftR, NULL);
		if(error != 0)
		{
			//If an error occurs while joining, output error message and exit
			fprintf(stderr, "Error joining liftR pthread : %s \n", strerror(error));
			return -1;
		}



		//After all tasks have finished, write the total lift operation to sim_out
		writeTotalOperation("sim_out",consumer, 3);	

		//Free the malloced buffer
		free(buffer);
		
		//Destroy all the mutex lock and conditional variables that were created and used
		//throughout the operation to prevent any memory leaks
		error += pthread_mutex_destroy(&mutex);
		error += pthread_cond_destroy(&full);
		error += pthread_cond_destroy(&empty);
		if(error != 0)
		{
			//If an error occurs while destroying, output error message and exit
			fprintf(stderr, "Error destroying : \n");
			return -1;
		}


	}
	else
	{
		//Output to the user, the correct arguments that need to passed 
		//in order to run the program
		fprintf(stderr,"Need exactly 3 arguments\n");
		fprintf(stderr, "./EXEC <buffer size>  <request_time>\n");
		fprintf(stderr, "buffer size >= 1 and request_time >= 0\n");
	}

	return 0;
}

//Perform lift operation, with the correct lift imported
//Code adapted from lecture 2 slide 24 and lecture 3 slide 5
//The buffer will hold atmost BUFFERSIZE elements
void* lift(void* param)
{
	//Typecast param to correct lift struct
	Elevator* inLift = (Elevator*)param;

	//Determines the operation should stop, by executing the 
	//final lift operation
	int finalTask = FALSE;

	do
	{
		//Simulate the lift operation by causing the lift to sleep
		//to make it seem like its carrying out the request
		sleep(sleeptime);

		//Enter critical section, by acquiring the mutex lock
		//ensuring mutual exclusion
		//Begin modifying the shared resources
		pthread_mutex_lock(&mutex);
		
		//If the buffer is empty
		while(counter == 0)
		{
			//Release the acquired mutex and wait for the buffer to have atleast 1 element
			//allowing request() to fill the buffer as required maintaining synchronisation
			pthread_cond_wait(&full, &mutex);
		}
		
		//Pull out a request item from buffer
		Req inReq = buffer[out];

		//The buffer is taken to be circular
		//out will now point out to the next item in the buffer
		//if there exists any
		out = (out + 1) % BUFFERSIZE;

		//Decrement counter to let other threads know that the 
		//buffer now contains 1 item less
		counter = counter - 1;

		//Keep track of the total number of requests carried out by all the 
		//consumer thread
		operate++;
		
		//Output the task carried out by specific lift to sim_out
		writeLift("sim_out", inReq, inLift);

		//Check to see whether the current task is the final task
		if(operate >= requestsInFile - 2)
		{
			//Let all consumer threads know that the task they are 
			//currently carrying out is their final taks and after
			//completion they should exit
			finalTask = TRUE;
		}

		//Let all other threads know that the task has been accomplished
		pthread_cond_signal(&empty);

		//Release the mutex lock to let other threads enter their critical sections
		pthread_mutex_unlock(&mutex);


	}
	while(!finalTask);
	
	//Exit when finalTask has been carried out by the lift
	pthread_exit(NULL);
}

//Place one request into the buffer at a time from imported 
//temporary array as required 
//until the all the elements from the array has entered 
//the buffer, the buffer will hold atmost BUFFERSIZE elements
//Code adapted from lecture 2 slide 24 and lecture 3 slide 5
void* request(void* param)
{
	//Typecast param to appropriate array of requests
	Req* array = (Req*)param;

	//Determine the next element to transfer from
	//array to buffer
	int index = 0;

	//Fill up the buffer with requests untill all elements in
	//array are placed 
	do
	{
		//Enter critical section by acquiring mutex lock to modify
		//the shared resources
		pthread_mutex_lock(&mutex);

		//If buffer if full allow the consumers to pull
		//out requests from buffer
		while(counter == BUFFERSIZE)
		{
			//Release the mutex lock and wait for the consumer
			//to empty the buffer maintaining synchronisation
			pthread_cond_wait(&empty, &mutex);
		}

		//Place one request from array into buffer
		buffer[in] = array[index];

		//Write the contents of the request placed to 
		//sim_out
		writeRequest("sim_out", buffer[in]);

		//The buffer is taken to be circular
		//in will now point out to the next empty position in buffer
		//if there exists any
		in = (in + 1) % BUFFERSIZE;

		//Increment counter to let other threads know that there
		//is one more item in the buffer
		counter = counter + 1;

		//Increment index to fetch another request from array to 
		//place it into buffer
		index = index + 1;
		
		//Let other threads know that an element has been added to
		//the buffer
		pthread_cond_signal(&full);

		//Release the mutex lock to let other threads enter their critical sections
		pthread_mutex_unlock(&mutex);

	}while(index < requestsInFile);

	pthread_exit(NULL);
}

