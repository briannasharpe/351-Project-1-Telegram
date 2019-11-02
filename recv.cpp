#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */

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

void init(int& shmid, int& msqid, void*& sharedMemPtr) {
	
	// ====================================================================================================
	/* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	         2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		 3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V object (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all System V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	
	printf("RECEIVER\n");
	// ftok to generate unique key
	key_t key = ftok("keyfile.txt", 'a');
	
	// check success
	if (key == -1) {
		perror("ftok: ftok failed");
		exit(1);
	}
	
	// ====================================================================================================
	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
	
	printf("allocating shared memory\n");
	// shmget - to obtain access to a shared memory segment
	// IPC_CREAT - to create a new memory segment
	// 64 permissions (rw-r--r--)
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644);
	
	// check success
	if (shmid == -1) {
		perror("shmget: shmget failed");
		exit(1);
	}
	
	// ====================================================================================================
	/* TODO: Attach to the shared memory */
	
	printf("attaching to shared memory\n");
	// shmat - to attach to shared memory
	// (void *)0 used so OS can choose address for us
	// last arg is 0 since SHM_RDNONLY is for reading only, 0 otherwise
	sharedMemPtr = shmat(shmid, (void *)0, 0);
	
	//shmat returns -1 if it fails. Check for -1 failure w/ comparison to check for error
	// check success
	if (sharedMemPtr == (void *) -1) {
		perror("shmat: shmat failed");
		exit(1);
	}
	
	// ====================================================================================================
	/* TODO: Create a message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	
	printf("creating message queue\n");
	// msgget - returns the System V message queue identifier associated w/ value of the key argument
	// 0666 - usual access permissions 
	msqid = msgget(key, 0666);
	
	// check success
	if (msqid == -1) {
		perror("msgget: msgget failed");
		exit(1);
	}
	
}
 

/**
 * The main loop
 */
void mainLoop() {
	/* The size of the mesage */
	int msgSize = 0;
	
	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");
		
	/* Error checks */
	if(!fp) {
		perror("fopen");	
		exit(-1);
	}

	// ====================================================================================================
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

	printf("receiving message from sender\n");
	// send and receive message buffers
	message sndMsg;
	message rcvMsg;

	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */	
	
	msgSize = 1;

	while(msgSize != 0) {	
		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0) {
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0) {
				perror("fwrite");
			}
			
			// ====================================================================================================
			/* TODO: Tell the sender that we are ready for the next file chunk. 
 			 * I.e. send a message of type RECV_DONE_TYPE (the value of size field
 			 * does not matter in this case). 
 			 */
			 
			 // set size to 0
			sndMsg.size = 0;
			sndMsg.mtype = RECV_DONE_TYPE;
			
			//check success (sending message)
			if (msgsnd(msqid, &sndMsg, 0, 0) == -1) {
				perror("msgsnd: msgsnd failed");
			}
			 
		}
		/* We are done */
		else {
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

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr) {
	// ====================================================================================================
	/* TODO: Detach from shared memory */
	
	printf("detaching from shared memory\n");
	shmdt(sharedMemPtr);
	
	// ====================================================================================================
	/* TODO: Deallocate the shared memory chunk */
	
	printf("deallocating shared memory chunk\n");
	// shmctl - to perform control operation on the shared memory segment whose identifier is given in shmid
	// IPC_RMID - to mark the segment to be destroyed
	shmctl(shmid, IPC_RMID, NULL);
	
	// ====================================================================================================
	/* TODO: Deallocate the message queue */
	
	printf("deallocating message queue\n");
	// msgctl - to control message queue specified by msqid
	msgctl(msqid, IPC_RMID, NULL);
	
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal) {
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv) {
	
	// ====================================================================================================
	/* TODO: Install a signal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
	 
	 // signal - signal handling
	 // SIGINT - interrupt signal
	 signal(SIGINT, ctrlCSignal);
				
	/* Initialize */
	init(shmid, msqid, sharedMemPtr);
	
	/* Go to the main loop */
	mainLoop();

	// ====================================================================================================
	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
	
	printf("detaching from shared memory segment\n");
	cleanUp(shmid, msqid, sharedMemPtr);
	
	return 0;
}
