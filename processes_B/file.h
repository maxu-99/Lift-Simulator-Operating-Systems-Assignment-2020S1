/* Author : Mahmudul Hossain (19303235)
 * Purpose : Store necessary function header for file.c
 * Date Modified : 09/05/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>



#include "lift_sim_B.h"

int checkValidFile(char* filename, int* totalReq);
void getAllReqFile(char* filename, int totalReq, Req* array);
void writeRequest(char* filename,Req inReq);
void writeLift(char* filename, Req inReq, Elevator* inLift);
void writeTotalOperation(char* filename,Elevator* consumer,int size);

