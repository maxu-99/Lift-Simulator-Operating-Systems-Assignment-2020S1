# Author : Mahmudul Hossain (19303235)
# Purpose : Makefile for lift_sim_B
# Date Modified : 10/05/2020

CC = gcc
EXEC = lift_sim_B

CFLAGS = -Wall -Werror -g -std=c99
T = -lrt -pthread
OBJ = lift_sim_B.o file.o



$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(T)

lift_sim_B.o: lift_sim_B.c file.h global.h
	$(CC) -c $(CFLAGS) lift_sim_B.c -o lift_sim_B.o $(T)

file.o : file.c file.h
	$(CC) -c $(CFLAGS) file.c -o file.o

clean : 
	rm -f $(EXEC) $(OBJ) sim_out
