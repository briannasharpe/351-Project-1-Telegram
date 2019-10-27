
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include "msg.h"    /* For the message struct */

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

using namespace std;

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
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
 		    
 	/*ofstream myFile;
 	string fileName = argv[1];
 	fout.open(fileName);
 	
 	if(!myFile.good()) {
 		perror("(ofstream) failed!");
 		exit(1);//exit status 1 for error
 	}
 	for(int i = 0; i < SHARED_MEMORY_CHUNK; i++) {
 		myFile << i << "Hello World!\n";
 	}
 	
 	myFile.close();*/
 	
//   2. Use ftok("keyfile.txt", 'a') in order to generate the key.
	key_t key = ftok("keyfile.txt", 'a');
	
	
/*3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V objest (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all SYstem V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	
	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	//shmget() is used to obtain acces to a shared mem seg
	//IPC_CREAT = create a new memory segment
	// 64 permissions (rw-r--r--)
	if((shmid = shmget (key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT)) == -1) {
		perror("shmget: shmget failed");
		exit(1);
	}
	
	/* TODO: Attach to the shared memory */
	// (void *)0 used so OS can choose address for us
	//last arg is 0 since SHM_RDNONLY is for reading only, 0 otherwise
	//Using sharedMemPtr, ptr provided by template
	sharedMemPtr = shmat(shmid, (void *)0, 0);
	
	//shmat returns -1 if it fails. Check for -1 failure w/ comparison to check for error
	if(sharedMemPtr == (char *) (-1)) {
		perror("shmat: shmat failed attaching to shared memory");
		exit(1);//exit 1 for failure
	}
	/* TODO: Attach to the message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	if((msqid = msgget(key, 0644 | IPC_CREAT) == -1)) {
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

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	if(shmdt(sharedMemPtr) == -1) {
		perror("shmdt: shmdt failed detaching from memory");
		exit(1);
	}
	
	
}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");
	

	/* A buffer to store message we will send to the receiver. */
	message sndMsg; 
	
	/* A buffer to store message received from the receiver. */
	message rcvMsg;
	
	/* Was the file open? */
	if(!fp)
	{
		perror("fopen");
		exit(-1);
	}
	
	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory. 
 		 * fread will return how many bytes it has actually read (since the last chunk may be less
 		 * than SHARED_MEMORY_CHUNK_SIZE).
 		 */
		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
		{
			perror("fread");
			exit(-1);
		}
		
		
		
		//msgrcv() messages are retrieved from queue
		// msgctl() destroy message queue
		//msgsnd() Data is placed on to a message queue by calling msgsnd()
			
		/* TODO: Send a message to the receiver telling him that the data is ready 
 		 * (message of type SENDER_DATA_TYPE) 
 		 */
		//msgsnd() Data is placed on to a message queue by calling msgsnd()
		if(msgsnd(msqid, &sndMsg, sizeof(struct message) - sizeof(long), 0) == -1) {
			perror("msgsnd: Failed msgsnd");
			//exit(-1);
		}
		
		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us 
 		 * that he finished saving the memory chunk. 
 		 */
 		 //Retrieve message from queue
 		 if(msgrcv(msqid, &rcvMsg, sizeof(struct message) - sizeof(long), RECV_DONE_TYPE, 0) == -1) {
 		 perror("msgrcv: mvsgrcv error");
 		 exit(1);
		}
	}
	

	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0. 	
	  */
	sndMsg.size = 0;
	if(msgsnd(msqid, &sndMsg, sizeof(struct message) - sizeof(long), 0) == -1) {
		perror("msgsnd error");
	}
		
	/* Close the file */
	fclose(fp);
	
}


int main(int argc, char** argv)
{
	
	/* Check the command line arguments */
	if(argc < 2)
	{
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
