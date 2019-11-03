#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream> //to allow file i/o
#include "msg.h"

// The size of the shared memory chunk
#define SHARED_MEMORY_CHUNK_SIZE 1000

#define ERROR_CODE -1
// The ids for the shared memory segment and the message queue
int shmid, msqid;

// The pointer to the shared memory
void *sharedMemPtr;

// The name of the received file
const char recvFileName[] = "recvfile";

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key. */
	         //keyfile.txt created and saved into same directory
	         //2. ftock(Points to path upon which part of the key is formed, character upon which part of key is formed) This should work since we're in the directory of keyfile.txt
{
	key_t key = ftok("keyfile.txt", 'a');

/*
		 3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V object (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	
	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	/* TODO: Create a message queue */
	//A new queue is created or an existing queue opened by msgget()
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT);
	
	msqid = msgget(key, 0666 | IPC_CREAT);

	// Attach to the shared memory
	sharedMemPtr = shmat(shmid, (void *)0, 0);

/* TODO: Attach to the shared memory */
	//I tried to attach it be assigning shmid to the the newly allocated piece of shared memory
	//for shflg I chose IPC_CREAT
	//If shared memory is ERROR_CODE, terminate because it's an error attaching to shared mem
	if(sharedMemPtr == (char *)(ERROR_CODE)){
		perror("shmat: Error in shmat receiver.cpp");
		exit(1);
	}
}
 
/**
 * The main loop
 */
void mainLoop()
{
	// The size of the mesage
	//int msgSize = 0;
	int msgSize;

	message recvMsg;

	// Open the file for writing
	FILE* fp = fopen(recvFileName, "w");
		
	// Error checks
	if (!fp)
	{
		perror("Error opening receiver file");	
		exit(ERROR_CODE);
	}

 /* TODO: Receive the message and get the message size. The message will 
     * contain regular information. The message will be of SENDER_DATA_TYPE
     * (the macro SENDER_DATA_TYPE is defined in msg.h).  If the size field
     * of the message is not 0, then we copy that many bytes from the shared
     * memory region to the file. Otherwise, if 0, then we close the file and
     * exit.
     *
     * NOTE: the received file will always be saved into the file called
     * "recvfile"
     */

	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */
 // Set message size from received message
	msgSize = recvMsg.size;
	 
	msgrcv(msqid, &recvMsg, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE , 0);
	
	while (msgSize != 0)
	{

		// If the sender is not telling us that we are done, then get to work
		if (msgSize != 0)
		{		
			// Save the shared memory to file
			if ( fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite");
			}
			// Set the sent message type			
			recvMsg.mtype=RECV_DONE_TYPE;

			// Send the message that we are done with message
			msgsnd(msqid, &recvMsg, 0 , 0);

			// Receive next message
			msgrcv(msqid, &recvMsg, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE , 0);

			// Set message size to be the message size of the received message
			msgSize = recvMsg.size;
		}
		else
		{
			// Close the file
			fclose(fp);
		}
	}
}

/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr);
	/* TODO: Deallocate the shared memory chunk */
	shmctl(shmid, IPC_RMID, 0);
	/* TODO: Deallocate the message queue */
	msgctl(msqid, IPC_RMID, 0);
}

void ctrlCSignal(int mySignal)
{
	// Free system V resources
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
	
	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
	signal(SIGINT, ctrlCSignal); // signal handler call
	
	// Initialize
	init(shmid, msqid, sharedMemPtr);
	
	// Go to the main loop
	mainLoop();
	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
		cleanUp(shmid, msqid, sharedMemPtr);
	
	return 0;
}
