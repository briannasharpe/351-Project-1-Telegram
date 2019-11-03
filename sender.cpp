
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>//Allow for file i/o
#include "msg.h"//for the message struct

// The size of the shared memory chunk
#define SHARED_MEMORY_CHUNK_SIZE 1000

// For testing purposes
#define MESSAGE_COUNT_FOR_TESTING 1000

using namespace std;
#define ERROR_CODE -1
// The ids for the shared memory segment and the message queue
int shmid, msqid;

// The pointer to the shared memory
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	
	/* TODO: 
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).*/
	key_t key = ftok("keyfile.txt", 'a');

	//* TODO: Get the id of the shared memory segment
	//The size of the segment must be SHARED_MEMORY_CHUNK_SIZE
	//shmget() is used to obtain acces to a shared mem seg
	//IPC_CREAT = create a new memory segment
	// 64 permissions (rw-r--r--)
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT);
	
	/*3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V objest (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all SYstem V objects. Two objects, on the other hand,
		    may have the same key.
	 */

	/* TODO: Attach to the shared memory */
	// (void *)0 used so OS can choose address for us
	//last arg is 0 since SHM_RDNONLY is for reading only, 0 otherwise
	//
	//shmat attaches the shared memory segment identified by shmid
	//to the address space of the calling process
	sharedMemPtr = shmat(shmid, (void *) 0, 0);
	
	
	//Using sharedMemPtr, ptr provided by template
	//shmat returns -1 if it fails. Check for -1 failure w/ comparison to check for error
	//credit to csl.mtu.edu
	if (sharedMemPtr == (char *)(ERROR_CODE)) {
		perror("shmat: error shmat failed attaching to shared memory");
		exit(1);
	}
	
	/* TODO: Attach to the message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	msqid = msgget(key, 0666 | IPC_CREAT);
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr);

}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
	// Open the file for reading
	FILE* fp = fopen(fileName, "r");

	// Was the file open?
	if (!fp)
	{
		perror("fopen");
		exit(ERROR_CODE);
	}

	/* A buffer to store message we will send to the receiver. */
	message sndMsg;

	//Ensure correct Data type to avoid error
	sndMsg.mtype = SENDER_DATA_TYPE;
	
	/* A buffer to store message received from the receiver. */
	message rcvMsg;
	
	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory. 
		 * fread will return how many bytes it has actually read (since the last chunk may be less
		 * than SHARED_MEMORY_CHUNK_SIZE).
		 */

		sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp);
		
		//msgrcv() messages are retrieved from queue
		// msgctl() destroy message queue
		//msgsnd() Data is placed on to a message queue by calling msgsnd()
		/* Send message to queue alerting reciver message is ready */
		/* TODO: Send a message to the receiver telling him that the data is ready 
 		 * (message of type SENDER_DATA_TYPE) 
 		 */
		msgsnd(msqid, &sndMsg , sizeof(struct message) - sizeof(long), 0);
			
		/* Wait until the receiver sends a message of type RECV_DONE_TYPE telling the sender 
		 * that it finished saving the memory chunk. 
		 */
		msgrcv(msqid, &rcvMsg, sizeof(struct message) - sizeof(long), RECV_DONE_TYPE, 0);
	}

	/* Send message to queue to tell reciver we have no more data to send, size = 0
	 * Set the message size in sndMsg to 0; siganls no more data
	 */
	sndMsg.size = 0;

	msgsnd(msqid, &sndMsg , sizeof(struct message) - sizeof(long) , 0);
	// Close the file
	fclose(fp);
}

int main(int argc, char** argv)
{
	
	// Check the command line arguments
	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(ERROR_CODE);
	}
		
	// Connect to shared memory and the message queue
	init(shmid, msqid, sharedMemPtr);
	
	// Get the file name from the command line
	send(argv[1]);

	// Call Cleanup function to detach from shared mem
	cleanUp(shmid, msqid, sharedMemPtr);
		
	return 0;
}
