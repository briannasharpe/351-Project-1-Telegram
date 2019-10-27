#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */

//Added libraries
#include <stdio.h>/* to use ftock()*/
#include <sys/ipc.h>
#include <iostream>
using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";


/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	
	/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key. */
	         key_t key = ftok("keyfile.txt", 'a');
	         //keyfile.txt created and saved into same directory
	         //2. ftock(Points to path upon which part of the key is formed, character upon which part of key is formed)
	        
	         /*
	         
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
	if((shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT)) == -1) {
		perror("shmget: Error with shmget reciever code");
		exit(1);
	}
	
	// will use the form: int shmget ( key_t key, int size, int shflg);
	
	
	if((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
		perror("msgget: Error with msgget receiver code");
		exit(1);
	}
	/* TODO: Attach to the shared memory */
	//I tried to attach it be assigning shmid to the the newly allocated piece of shared memory
	//for shflg I choce IPC_CREAT
	sharedMemPtr = shmat(shmid, (void *)0, 0);
	
	if (sharedMemPtr == (char *)(-1)) {
		perror("shmat: Error with Shmat in receiver code");
		exit(1);
	}
	
	
	
}
 

/**
 * The main loop
 */
void mainLoop()
{
	/* The size of the mesage */
	int msgSize = 0;
	
	message myMessage;
	
	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");
		
	/* Error checks */
	if(!fp)
	{
		perror("fopen");	
		exit(-1);
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
 	 
 	 if(msgrcv(msqid, &myMessage, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE, 0) == -1) {
 	 perror("msgrcv: Msgrcv error in receiver code");
 	 fclose(fp);
 	 exit(1);
 	 }
 	 	
	msgSize = myMessage.size;
	
	while(msgSize != 0)
	{	
		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0)
		{
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite");
			}
			
			//Use set messagae type from struct
			myMessage.mtype = RECV_DONE_TYPE;
			
			if(msgsnd(msqid, &myMessage, 0, 0) == -1) {
				perror("msqid: Error with msqid in receiver code");
				exit(1);
				}
			//get the next message
			if(msgrcv(msqid, &myMessage, sizeof(struct message)  - sizeof(long), SENDER_DATA_TYPE, 0) == -1) {
			perror("msgrcv: Error with msgrcv in receiver code");
			exit(1);
			}
			msgSize = myMessage.size;
		}
		/* We are done */
		else
		{
			/* Close the file */
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
	shmctl(shmid, IPC_RMID, NULL);
	/* TODO: Deallocate the message queue */
	msgctl(msqid, IPC_RMID, NULL);
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
	
	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
	//cntrlCSignal(SIGINT);
	/* Initialize */
	init(shmid, msqid, sharedMemPtr);
	
	/* Go to the main loop */
	mainLoop();

	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
		
	return 0;
}
