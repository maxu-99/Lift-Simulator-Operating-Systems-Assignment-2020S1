/* Author : Mahmudul Hossain (19303235)
 * Purpose : Lift simulator using processes
 * 			 upon a bounded buffer. 
 * 			 Implemented using POSIX shared memory and
 * 			 semaphores
 * Date Modified : 10/05/2020
 */
 
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>


#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

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

		//Error check to determine if functions are performing correctly
		int error = 0;

		//Assign valid parameters to variable
		sleeptime = atoi(argv[2]);
		BUFFERSIZE = atoi(argv[1]);
	
		//Bounded buffer
		Req* buffer;

		//Store the semaphores to be used to ensure
		//mutual exclusion between processes which is
		//to be shared between processes
		Semaphore* condition;

		//Variables to be shared between producer request() and 
		//consumer lift()
		SharedData* count;

		//Lift-1, 2 and 3 
		//To be shared between processes to keep track of the total 
		//number of requests and movements carried out
		Elevator* consumer;

		//Temporary array to store requests that are to be loaded into
		//the buffer
		Req array[requestsInFile];		

		
		//Stores each file descriptor, used to keep track of the
		//created shared memory blocks.Allows shared memory to be 
		//manipulated independtly.
		int dataFd;
		int semFd;
		int bufferFd;
		int elevatorFd;
		
		//Create POSIX shared memory objects assigning to appropriate 
		//file descriptor
		dataFd = shm_open("sharedData", O_CREAT | O_RDWR, 0666);
		semFd = shm_open("sharedSemaphore", O_CREAT | O_RDWR, 0666);
		bufferFd = shm_open("sharedBuffer", O_CREAT | O_RDWR, 0666);
		elevatorFd = shm_open("elevator", O_CREAT | O_RDWR, 0666);

		//Determines the size of each shared memory regions
		error += ftruncate(dataFd, sizeof(SharedData));
		error += ftruncate(semFd, sizeof(Semaphore));
		error += ftruncate(bufferFd, sizeof(Req) * BUFFERSIZE);
		error += ftruncate(elevatorFd, sizeof(Elevator) * 3);

		//Check to see if ftruncate failed
		if(error != 0)
		{
			fprintf(stderr, "ftruncate initialisation failed \n");
			return -1; //Exit on error
		}


		//Map the shared regions of memory to appropriate pointer variables
		//The shared memory region will be shared among processes where they can perform both
		//read and write operations
		count = (SharedData*)mmap(0, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, dataFd, 0);	
		condition = (Semaphore*)mmap(0, sizeof(Semaphore), PROT_READ | PROT_WRITE, MAP_SHARED, semFd, 0);
		buffer = (Req*)mmap(0, sizeof(Req) * BUFFERSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bufferFd, 0);
		consumer = (Elevator*)mmap(0, sizeof(Elevator) * 3, PROT_READ | PROT_WRITE, MAP_SHARED, elevatorFd, 0);


		//Initialise SharedData count 
		count->in = 0;
		count->out = 0;
		//count->counter = 0; 
		count->operate = 0;
		
		//Initialise the semaphore to valid values
		error = initialiseSemaphores(condition);

		//If an error occurs initialising, exit on error
		if(error != 0)
		{
			fprintf(stderr, "Semaphore initialisation failed \n");
			return -1;
		}
		
		//Update the array will all the requests stored in the file
		getAllReqFile("sim_input", requestsInFile, array);

			

		//Initialise lifts where all lifts start from floor 1;
		for(int i = 0; i < 3; i++)
		{
			consumer[i].liftID = i + 1;
			consumer[i].currentFloor = 1;	
			consumer[i].sumRequest = 0;
			consumer[i].sumMovement = 0;
		}
		


		//Used for parent process to wait for child process
		//to terminate
		int status = 0;

		//Obtain the id of the root process
		pid_t root = getpid();

		//Used to determine if the process is a child or parent
		pid_t pid = -1;

		//Determines which elevator construct in consumer array to be 
		//passed into lift()
		int j = -1;

		for(int i = 0; i < 3; i++)
		{
			if(pid != 0)
			{
				j++;
				//Only the parent process creates child processes
				pid = fork();
			}
		}

		if(pid == 0)
		{
			//Run consumer process lift() on child processes
			lift(&consumer[j], condition, count, buffer);
		}
		else if(pid > 0)
		{
			//Run producer process request() on parent processes
			request(array, condition, count, buffer);
		}
		else if(pid < 0)
		{
			//If process creation failed, output error to user and exit
			fprintf(stderr, "fork() failed : %s\n", strerror(pid));
			return -1;
		}

		//Parent needs to wait for the child processes to finish executing before 
		//termination to prevent zombie processes
		for(int i = 0; i < 3; i++)
		{
			wait(&status);
		}

		//Only the root process writes final state to sim_out and 
		//perfrom necessary memory freeing
		//All other processes exits normally
		if(root == getpid())
		{
			//After request() and lift() finished operating, write the total lift operation to sim_out
			writeTotalOperation("sim_out",consumer, 3);	
		
			//Destroy the shared semaphores used during the operation, to prevent any memory leaks
			error = destroySemaphores(condition);

			//If an error occurs while destroying, output error to user and exit on error
			if(error != 0)
			{
				fprintf(stderr, "Semaphores failed to destroy itself \n");
				return -1;
			}
			
			//Unmap all the shared memory shared between processes
			error += munmap(count, sizeof(SharedData));
			error += munmap(condition, sizeof(Semaphore));
			error += munmap(buffer, sizeof(Req) * BUFFERSIZE);
			error += munmap(consumer, sizeof(Elevator) * 3);

			//If an error occured while unmapping, output error to the user and exit on error
			if(error != 0)
			{
				fprintf(stderr, "Memory unmap failed \n");
				return -1;
			}

			//Unlink the POSIX shared memory objects by deallocating and destroying all the contents of the associated memory region
			error += shm_unlink("sharedData");
			error += shm_unlink("sharedSemaphore");
			error += shm_unlink("sharedBuffer");
			error += shm_unlink("elevator");

			//If an error occured while unlinking, output error to the user and exit on error
			if(error != 0)
			{
				fprintf(stderr, "Memory unlink failed\n");
				return -1;
			}

			//Close all the file descriptors linked to the shared memory region
			error += close(dataFd);
			error += close(semFd);
			error += close(bufferFd);
			error += close(elevatorFd);

			//If an error occurs while closing, output error to user and exit on error
			if(error != 0)
			{
				fprintf(stderr, "Failed to close file descriptor\n");
				return -1;
			}
	

		}
	}
	else
	{
		//Outout to the user, the correct arguments that need to be passed
		//in order to run the program
		fprintf(stderr,"Need exactly 3 arguments\n");
		fprintf(stderr, "./EXEC <buffer size>  <request_time>\n");
		fprintf(stderr, "buffer size >= 1 and request_time >= 0\n");
	}

	return 0;
}

//Perform lift operation, with the correct lift imported
//Code adapted from lecture 3 slide 5 and 27
void lift(Elevator* inLift, Semaphore* condition, SharedData* count, Req* buffer)
{

	//Perform lift operation until all requests are carried out
	do
	{
		//Simulate the lift operation by causing the lift to sleep
		//to make it seem like its carrying out the request
		sleep(sleeptime);

		//Wait when there are no full slots in the buffer i.e full <= 0 is true
		sem_wait(&condition->full);
		//Enter critical section by acquiring binary mutex semaphore lock
		//Beging modifying shared resources
		sem_wait(&condition->mutex);

		//Pull out a request item from buffer
		Req inReq = buffer[count->out];

		//The buffer is taken to be circular
		//out will now point out to the next item in the buffer
		//if there exists any
		count->out = (count->out + 1) % BUFFERSIZE;

		//Keep track on the total number of requests carried out by all
		//the lifts
		count->operate++;

		//Output the task carried out by the specific lift to sim_out
		writeLift("sim_out", inReq, inLift);

		//Exit from critical section
		sem_post(&condition->mutex);

		//Signal the producer that buffer slots are emptied and add more requests to buffer to be consumed by the lift
		sem_post(&condition->empty);

	}
	while(count->operate < requestsInFile - 2);
	
	
}

//Place one request into the buffer at a time from imported
//temporary array as required until the all the elements 
//from the array has entered 
//the buffer, the buffer will hold atmost BUFFERSIZE elements
//Code adapted from lecture 3 slide 5 and 27
void request(Req* array, Semaphore* condition, SharedData* count, Req* buffer)
{
	//Determine the next element to transfer from array
	//to buffer
	int index = 0;

	//Fill up the buffer with requests until all elements
	//in the array are placed 
	do
	{
		//Wait when the buffer is full i.e empty <= 0 is true
		sem_wait(&condition->empty);

		//Enter critical section by acquiring mutex lock to 
		//begin modifying the shared resources
		sem_wait(&condition->mutex);

		//Place one request from array into buffer
		buffer[count->in] = array[index];

		//Write the contents of the request placed to sim_out
		writeRequest("sim_out", buffer[count->in]);

		//The buffer is take to be circular
		//out will now point to the next occupied position in buffer
		//if there exists any
		count->in = (count->in + 1) % BUFFERSIZE;

		//Increment index to fetch another request from array to 
		//place it into buffer
		index++;

		//Exit from critical section
		sem_post(&condition->mutex);

		//Signal consumer that there is now a request in the buffer
		//to be consumed
		sem_post(&condition->full);
	
	}while(index < requestsInFile);
}

//Initialise shared semaphores to appropriate value
int initialiseSemaphores(Semaphore* s)
{
	//Used to detect any error returned by sem_init
	int error = 0;

	//Number of requests currently in buffer
	//Initialised to 0
	error += sem_init(&s->full, 1, 0);

	//Number of empty slots in the buffer
	//Initialised to BUFFERSIZE
	error += sem_init(&s->empty, 1, BUFFERSIZE);

	//Binary semaphore to provide mutual exclusion between processes
	//in critical section
	//Initialised to 1 
	error += sem_init(&s->mutex, 1, 1);

	return error;
}

//Destroy the imported shared semaphores
//To prevent memory leaks
int destroySemaphores(Semaphore* s)
{
	//Used to detect any error returned by sem_destroy
	int error = 0;

	//Destroy the semaphore indicated by at the address
	//pointed by s
	error += sem_destroy(&s->full);	
	error += sem_destroy(&s->empty);	
	error += sem_destroy(&s->mutex);

	return error;
}
