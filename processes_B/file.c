/* Author : Mahmudul Hossain (19303235)
 * Purpose : Carry out file I/O operations as required
 * Date Modified : 9/05/2020
 */


#include <stdio.h>
#include "file.h"


//Check that the imported filename is valid 
//containing 50 to 100 requests
//Each request presenting as : 
//<source floor>  <destination floor>
//where floor numbers are between 1 and 20 inclusive 
//Update requestsInFile to store the total number of requests
int checkValidFile(char* filename, int* requestsInFile)
{
	//num1 and num2 is used to check whether the floor 
	//numbers are between 1 and 20 inclusive
	//total stores the total number of succcessful requests read
	int num1, num2, total = 0;

	//Assuming that the imported file is valid
	bool flag = true; 

	//Open file
	FILE* fptr = fopen(filename, "r");

	//Store the number of parameters read
	int read;
	if(fptr != NULL)
	{
		//Continue reading until EOF and the file being valid
		while(!(feof(fptr)) && flag )
		{
			//Parse the read string and assign numbers to num1 and num2
			read = fscanf(fptr, "%d %d\n", &num1, &num2);
			
			//Assuming there will not be any empty lines
			//An error or invalid format will make the file invalid
			if(ferror(fptr) || read != 2 || (num1 < 1 || num1 > 20) || (num2 < 1 || num2 > 20))
			{
				perror("Error in file format : \n");
				flag = false;
			}

			//Total number of requests read
			total++;
		}
		
		//Total number of requests must be 50 and 100 inclusive
		if((total < 50) || (total > 100))
		{
			flag = false;
		}

		//Updaye requestsInFile with total
		*requestsInFile = total;
		fclose(fptr);
	}
	else
	{
		perror("Error in handling file : \n");
	}

	//Return true if file is valid else false
	return flag;
}


//Read the valid input file. Generate a request from
//each line read and store it into array
void getAllReqFile(char* filename, int size, Req* array)
{
	FILE* fptr = fopen(filename, "r");

	//Request number
	int ii = 0;
	int src, dest;

	if(fptr != NULL)
	{
		//Read till EOF
		while(!(feof(fptr)))
		{
			//Parse each line read into src and dest
			fscanf(fptr, "%d %d\n", &src, &dest);

			//Req struct to hold each request
			Req request;

			//Assuming the first request starts from 1
			request.requestNo = ii + 1;

			//Assign the source
			request.source = src;

			//Assigne the destination
			request.destination = dest;

			//The request created is now stored into the 
			//array
			array[ii] = request;

			ii++;
		}
		fclose(fptr);
	}
	else
	{
		perror("Error in handling file : \n");
	}

}


//Write the new lift request that has been added
//to the buffer using the imported filename
void writeRequest(char* filename,Req inReq)
{
	//Write from the end of the file by appending
	FILE* fptr = fopen(filename, "a");

	if(fptr != NULL)
	{
		//Write the components of the request in a 
		//specific format
		fprintf(fptr,"------------------------------\n");
		fprintf(fptr,"\tNew Lift Request From Floor %d ", inReq.source);
		fprintf(fptr,"to Floor %d\n", inReq.destination);
		fprintf(fptr,"\tRequest No: %d\n",inReq.requestNo);
		fprintf(fptr,"------------------------------\n");
		fclose(fptr);
	}
	else
	{
		perror("Error opening file : \n");
	}
}

//Write each request carried out by a specific lift
//to the imported filename
void writeLift(char* filename, Req inReq, Elevator* inLift)
{
	//Write from the end of the file by appending
	FILE* fptr = fopen(filename, "a");

	if(fptr != NULL)
	{
		//Calculate the total movement carried out by the 
		//specific lift for the imported request
		//movment = |current - source| + |source - destination| 
		int movement = abs(inLift-> currentFloor - inReq.source);
		movement = movement + abs(inReq.destination - inReq.source);

		//Increment the total number of requests carried out by the
		//lift during the operation
		inLift -> sumRequest++;

		//Update the total number of movements carried out by the lift
		//during the operation
		inLift -> sumMovement += movement;

		//Write the contents of the request and lift into specific format
		fprintf(fptr,"Lift-%d Operation\n", inLift->liftID);
		fprintf(fptr,"Previous position : Floor %d\n", inLift-> currentFloor);
	    fprintf(fptr,"Request:Floor %d to Floor %d\n", 
				inReq.source, inReq.destination);
		fprintf(fptr,"Detailed operations : \n");
		fprintf(fptr,"\t Go from Floor %d to Floor %d\n",
			    inLift-> currentFloor, inReq.source);
		fprintf(fptr,"\t Go from Floor %d to Floor %d\n",
				inReq.source, inReq.destination);	
		fprintf(fptr,"\t #movement for this request : %d\n", movement);
		fprintf(fptr,"\t #request : %d\n", inLift -> sumRequest);
		fprintf(fptr,"\t Total #movement : %d\n", inLift -> sumMovement);
		fprintf(fptr,"Current position: Floor %d\n\n", inReq.destination);

		//Update the current floor of the lift to be the destination of the 
		//request
		inLift -> currentFloor = inReq.destination;

		fclose(fptr);
	}
	else
	{
		perror("Error handling file : \n");
	}
}

//Calculate and write total number of requests and movement by the 
//lifts into sim_out.txt
void writeTotalOperation(char* filename,Elevator* consumer,int size)
{
	FILE* fptr = fopen(filename, "a");

	if(fptr != NULL)
	{
		//Total number of request calculated from each lift
		int totalRequests = 0;
		//Total number of movement carried out by each lift
		int totalMovements = 0;

		//Loop thorugh each lift struct to update totalRequests and 
		//totalMovements
		for(int i = 0; i < size; i++)
		{
			totalRequests += consumer[i].sumRequest;
			totalMovements += consumer[i].sumMovement;
		}

		//Write the calculated total requests and movements in a 
		//specific format
		fprintf(fptr,"---------------------------------------\n");
		fprintf(fptr,"Total number of requests : %d\n",totalRequests);
		fprintf(fptr,"Total number of movements : %d\n", totalMovements);
		fprintf(fptr,"---------------------------------------\n");

		fclose(fptr);
	}
	else
	{
		perror("Error handling file : \n");
	}
}


