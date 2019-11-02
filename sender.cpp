#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 */

// bucket
void init(int& shmid, int& msqid, void*& sharedMemPtr) {
	// ====================================================================================================
	/*  TODO: 
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	    2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		3. Use the key in the TODO's below. Use the same key for the queue
		   and the shared memory segment. This also serves to illustrate the difference
		   between the key and the id used in message queues and shared memory. The id
		   for any System V object (i.e. message queues, shared memory, and semaphores) 
		   is unique system-wide among all SYstem V objects. Two objects, on the other hand,
		   may have the same key.
	 */
	
	printf("SENDER\n");
	// ftok to generate unique key
	key_t key = ftok("keyfile.txt", 'a');
	
	// check success
	if (key == -1) {
		perror("ftok: ftok failed");
		exit(1);
	}
	
	// ====================================================================================================
	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	
	printf("getting id of shared memory segment\n");
	// shmget - to obtain access to a shared memory segment
	// IPC_CREAT - to create a new memory segment
	// 64 permissions (rw-r--r--)
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT);
	
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
	/* TODO: Attach to the message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	
	printf("attaching to message queue\n");
	// msgget - returns the System V message queue identifier associated w/ value of the key argument
	// 0666 - usual access permissions 
	msqid = msgget(key, 0666 | IPC_CREAT);
	
	// check success
	if (msqid == -1) {
		perror("msgget: msgget failed");
		exit(1);
	}
	
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr) {
	// ====================================================================================================
	/* TODO: Detach from shared memory */
	
	printf("detaching from shared memory\n");
	shmdt(sharedMemPtr);
}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName) {
	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");
	

	/* A buffer to store message we will send to the receiver. */
	message sndMsg; 
	
	/* A buffer to store message received from the receiver. */
	message rcvMsg;
	
	/* Was the file open? */
	if(!fp) {
		perror("fopen");
		exit(-1);
	}
	
	/* Read the whole file */
	while(!feof(fp)) {
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory. 
 		 * fread will return how many bytes it has actually read (since the last chunk may be less
 		 * than SHARED_MEMORY_CHUNK_SIZE).
 		 */
		if ((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0) {
			perror("fread");
			exit(-1);
		}
		
		// ====================================================================================================
		/* TODO: Send a message to the receiver telling him that the data is ready 
 		 * (message of type SENDER_DATA_TYPE) 
 		 */
		
		printf("sending message to receiver\n");
		// mtype from msg.h
		sndMsg.mtype = SENDER_DATA_TYPE;
		
		// msgsnd - to send message
		// check success
		if (msgsnd(msqid, &sndMsg, sizeof(sndMsg) - sizeof(long), 0) == -1) {
			perror("msgsnd: msgsnd failed");
		}
		
		// ====================================================================================================
		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us 
 		 * that he finished saving the memory chunk. 
 		 */
		 
		 printf("waiting for message from receiver\n");
		 // msgrcv - to receive message
		 // check success
		 if (msgrcv(msqid, &sndMsg, 0, RECV_DONE_TYPE, 0)) {
			 perror("msgrcv: msgrcv failed");
			 exit(1);
		 }
	}
	
	// ====================================================================================================
	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0. 	
	  */
	
	printf("finished sending file\n");
	// set size to 0
	sndMsg.size = 0;
	sndMsg.mtype = SENDER_DATA_TYPE;
	
	//check success (sending message)
	if (msgsnd(msqid, &sndMsg, sizeof(sndMsg) - sizeof(long), 0) == -1) {
		perror("msgsnd: msgsnd failed");
	}

		
	/* Close the file */
	fclose(fp);
	
}


int main(int argc, char** argv) {
	
	/* Check the command line arguments */
	if(argc < 2) {
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}
		
	/* Connect to shared memory and the message queue */
	init(shmid, msqid, sharedMemPtr);
	
	/* Send the file */
	send(argv[1]);
	
	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);
		
	return 0;
}
