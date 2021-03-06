
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

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	/* TODO: 
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
 		    so manually or from the code).
	    2. Use ftok("keyfile.txt", 'a') in order to generate the key.
		3. Use the key in the TODO's below. Use the same key for the queue
		    and the shared memory segment. This also serves to illustrate the difference
		    between the key and the id used in message queues and shared memory. The id
		    for any System V objest (i.e. message queues, shared memory, and sempahores) 
		    is unique system-wide among all SYstem V objects. Two objects, on the other hand,
		    may have the same key.
	 */
	key_t key;
	key=ftok("keyfile.txt",'a');
	/*-------------------------------------------------------------------------------------
	This function has 2 arguments. It uses the file name and the least significant 8 bits of
	proj_id (whic must be nonzero) to generate a key_t type System V IPC key, suitable for use
	with msgget(), semget(), or shmget().
	----------------------------------------------------------------------------------------*/
	//check for error of ftok()
	if(key==-1){
		perror("ftok error from sender\n");
		exit(EXIT_FAILURE);
	}
	
	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	shmid=shmget(key, SHARED_MEMORY_CHUNK_SIZE,0666|IPC_CREAT);
	/*-------------------------------------------------------------------------------------
	This shmget() function is used to allocate a System V shared memory segment
	This function has 3 parameters: the key, the size of the segment, and the semflg.
	The semflg argument tell the shmget() which permission will be and specify if
	we want to create a new one or use the existing one. In this case we create a new one
	so we use IPC_CREAT with permission 666 meaning -rw-rw-rw
	--------------------------------------------------------------------------------------*/
	//check for error of shmid()
	if(shmid==-1){
		perror("shmget error from sender\n");
		exit(EXIT_FAILURE);
	}
	/* TODO: Attach to the shared memory */
	sharedMemPtr=shmat(shmid,(void *)0,0);
	/*----------------------------------------------------------------------------------------
	This shmat() is used to get the pointer from the shmid handle in order to
	attache to the shared memory. This function has 3 parameters: the 1st is the shared id we 
	from the call to shmget() above, the 2nd is shmaddr which is specific address to use but 
	we can just set it to 0 and let the OS choose the address for us, the 3rd parameter is 
	shmflg which can be set to SHM_RDONLY if you want to read from it, or set to 0 otherwise.
	-----------------------------------------------------------------------------------------*/
	//check for error of shmat().
	if(sharedMemPtr==(void *)(-1))
	{
		perror("shmat error from sender,\n");//shmat() return -1 on failure so
		//if it does we use perror to display a message error
		exit(EXIT_FAILURE);//and exit the program.
	}
	/* TODO: Attach to the message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	msqid=msgget(key,0666|IPC_CREAT);
	/*-------------------------------------------------------------------------------------------
	This msgget() system call returns the message queue ID on success, or -1 on failure.This one
	will help to connect to the queue or create one if it doesn't exist.It has 2 parameters: 1st
	one is the key which is a system-wide unique identifier describing the queue we want to 
	connect or create, 2nd one is msgflg tells msgget() what to do. In this case we use 0666 for 
	permission -rw-rw-rw and IPC_CREAT to create a queue and connect to it.0666 -rw-rw-rw because 
	we are going to use msgrcv() and msgsnd()later and these function must have write and read 
	permission on the message queue in order to send a message, and to receive a message freely.
	-------------------------------------------------------------------------------------------*/
	//check for error of msgget()
	if(msqid==-1){
		perror("msgget error from sender.\n ");
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
	//To detach from shared memory we only need to use
	// shmdt()with a single parameter
	int val1=shmdt(sharedMemPtr);
	/*---------------------------------------------------------------------------------------------
	When we are done with the shared memory segment,our program use this function to detach itself 
	from shared memory. This one only have one argument which is shmaddr, it is the address we got 
	from shmat() above, and it is sharedMemPtr. The functions returns -1 on error, 0 on success.
	----------------------------------------------------------------------------------------------*/
	//check for error of shmdt()
	if(val1==-1){
		perror("shmdt error from receiver.\n");
		exit(EXIT_FAILURE);
	}
	printf("Finished detaching from shared memory from sender.\n");
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
		
			
		/* TODO: Send a message to the receiver telling him that the data is ready 
 		 * (message of type SENDER_DATA_TYPE) 
 		 */
		sndMsg.mtype=SENDER_DATA_TYPE;//set the send message type to SENDER_DATA_TYPE
		printf("The data is ready\n");
		printf("Sending message in progress...\n");
		printf("The size of sending message is: ");
		printf("%d",sndMsg.size);
		printf("\n");

		printf("The type of sending message is: ");
		printf("%ld",sndMsg.mtype);
		printf("\n");

		printf("Receiving message in progress...\n");

		int returnValue=msgsnd(msqid,&sndMsg,sizeof(message)-sizeof(long),0);
		/*-----------------------------------------------------------------------------------------
		This msgsnd() will pass information of message to a message queue. It has 4 arguments: 1st 
		is the msqid returned by msgget(), 2nd one is the msgp is a pointer to the data we want to 
		put on the queue, and it is a sndMsg in this case, 3rd is the msgsz is the size in bytes of 
		the data(message) to add to the queue(not count the size of the mtype member).That is why
		we have them subtract the sizeof(long) which is the size of mtype. The last argument is the 
		msgflag which allows us to set some option flag parameters, but in this project we don't need
		to, so we just set it to 0.
		-------------------------------------------------------------------------------------------*/
		//check for error of msgsnd()
		if(returnValue==-1){
			perror("msgsnd error from sender.cpp");
			exit(EXIT_FAILURE);
		}
		else{
			printf("Message has sent.\n");
		}
		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us 
 		 * that he finished saving the memory chunk. 
 		 */
		 int retVal=msgrcv(msqid,&rcvMsg,sizeof(message)-sizeof(long),RECV_DONE_TYPE,0);
		 /*--------------------------------------------------------------------------------------------
		 This msgrcv() this will help us get the message out from the queue.This function has 5 arguments
		 the 1st,2nd and the 3rd are similar to the above msgsnd() function,the fourth one is type 
		 RECV_DONE_TYPE which is specified by professor.The last argument we just want to set it to 0
		 because this project does not require any special option for flag parameter.
		 --------------------------------------------------------------------------------------------*/
		 //check for error of msgrcv()
		 if(retVal==-1){
			perror("msgrcv error from sender.cpp\n");
			exit(EXIT_FAILURE);
		 }
		 else{
			 printf("Message received.\n");
		 }
	}
	

	/** TODO: once we are out of the above loop, we have finished sending the file.
 	  * Lets tell the receiver that we have nothing more to send. We will do this by
 	  * sending a message of type SENDER_DATA_TYPE with size field set to 0. 	
	  */
	sndMsg.mtype=SENDER_DATA_TYPE;//set the message type to SENDER_DATA_TYPE as required
	sndMsg.size=0;
	printf("We are out of the loop,We have nothing to send.\n");
	printf("This is an empty message.\n");
	printf("Sending message...\n");

	int val=msgsnd(msqid,&sndMsg,sizeof(message)-sizeof(long),0);
	/*-----------------------------------------------------------------------------------------
		This msgsnd() will pass information of message to a message queue. It has 4 arguments: 1st 
		is the msqid returned by msgget(), 2nd one is the msgp is a pointer to the data we want to 
		put on the queue, and it is a sndMsg in this case, 3rd is the msgsz is the size in bytes of 
		the data(message) to add to the queue(not count the size of the mtype member).That is why
		we have them subtract the sizeof(long) which is the size of mtype. The last argument is the 
		msgflag which allows us to set some option flag parameters, but in this project we don't need
		to, so we just set it to 0.
	-------------------------------------------------------------------------------------------*/
	printf("The size of sending message is: ");
	printf("%d",sndMsg.size);
	printf("\n");

	printf("The type of sending message is: ");
	printf("%ld",sndMsg.mtype);
	printf("\n");
	//check for error of msgsnd()
	if(val==-1){
		perror("msgsnd error from sender.cpp\n");
		exit(EXIT_FAILURE);
	}
	else{
		printf("Message has sent.\n");
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
