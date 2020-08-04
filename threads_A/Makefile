# Author : Mahmudul Hossain (19303235)
# Purpose : Makefile for lift_sim_A
# Date Modified : 09/05/2020

CC = gcc
EXEC = lift_sim_A

CFLAGS = -Wall -Werror -g -std=c99
T = -pthread
OBJ = lift_sim_A.o file.o



$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(T)

lift_sim_A.o: lift_sim_A.c file.h global.h
	$(CC) -c $(CFLAGS) lift_sim_A.c -o lift_sim_A.o $(T)

file.o: file.c file.h
	$(CC) -c $(CFLAGS) file.c -o file.o 

clean : 
	rm -f $(EXEC) $(OBJ) sim_out
