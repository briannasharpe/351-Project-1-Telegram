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
{
	key_t key = ftok("keyfile.txt", 'a');

	// Create message queue
	if ( (shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT)) == -1 ){
		perror("shmget");
		exit(1);
	}

	// Attach to the message queue
	if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
			perror("msgget");//Failed msgget
			exit(1);//exit
	}

	// Attach to the shared memory
	sharedMemPtr = shmat(shmid, (void *)0, 0);

	if (sharedMemPtr == (char *)(-1)){
		perror("shmat");
		exit(1);
	}

}
 

/**
 * The main loop
 */
void mainLoop()
{
	// The size of the mesage
	int msgSize;

	message myMessage;

	// Open the file for writing
	FILE* fp = fopen(recvFileName, "w");
		
	// Error checks
	if ( ! fp)
	{
		perror("Error opening the receiver file");	
		exit(-1);
	}

	/**
	 * ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
	 * @description:   Receive message from the queue
	 * @param msqid:   The message queue
	 * @param msgp:    Message buffer to send the message to
	 * @param msgsz:   Maximum size in bytes for the member mtext of the structure pointed to by the msgp argument
	 * @param msgtyp:  If msgtyp is greater than 0, then the first message in the queue of type msgtyp is read
	 * @param msgflag: 0 | IPC_NOWAIT | MSG_EXCEPT | MSG_NOERROR
	 * @return On failure return -1 with errno indicating the error, otherwise msgrcv() returns the number of bytes actually copied into the mtext array. 
	*/
	if ( msgrcv(msqid, &myMessage, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE , 0) == -1) {
		perror("msgrcv: Error receiving message");
		fclose(fp);
		exit(1);
	}

	// Testing for console
	printf("MsgSize\n");
	myMessage.printer();

	// Set message size from received message
	msgSize = myMessage.size;

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
	
	while (msgSize != 0)
	{

		// If the sender is not telling us that we are done, then get to work
		if (msgSize != 0)
		{		
			// Save the shared memory to file
			if ( fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("(fwrite) Error writing to file");
			}

			// Set the sent message type			
			myMessage.mtype=RECV_DONE_TYPE;

			// Send the meassage we are done with message
			if ( msgsnd(msqid, &myMessage, 0 , 0) == -1) {
				perror("(msgsnd) Error sending message");
				exit(1);
			}

			// Receive next message
			if ( msgrcv(msqid, &myMessage, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE , 0) == -1) {
				perror("(msgrcv) Error receiving message");		
				exit(1);
			}

			// Testing for console
			myMessage.printer();

			// Set message size from recived message, LCV
			msgSize = myMessage.size;
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
	// Detach from shared memory
	shmdt(sharedMemPtr); // detach shared memory
	
	// Deallocate the shared memory chunk
	shmctl(shmid, IPC_RMID, NULL); // delete shared memory
	
	// Deallocate the message queue
	msgctl(msqid, IPC_RMID, NULL); // deallocate message queue
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */
void ctrlCSignal(int signal)
{
	// Free system V resources
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
	
	
	/* Install a singnal handler (see signaldemo.cpp sample file).
	 * In a case user presses Ctrl-c your program should delete message
	 * queues and shared memory before exiting. You may add the cleaning functionality
	 * in ctrlCSignal().
	 */
	signal(SIGINT, ctrlCSignal); // signal handler call
	
	// Initialize
	init(shmid, msqid, sharedMemPtr);
	
	// Go to the main loop
	mainLoop();
	
	return 0;
}
