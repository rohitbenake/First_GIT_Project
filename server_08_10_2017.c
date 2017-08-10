#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>


#include <stdlib.h>
#include <pthread.h> 

#define CMD_SOCKET_PORT_CLIENT	7240

#define VIDEO_SOCKET_PORT_CLIENT 8240

#define MEDIA_SOCKET_PORT_CLIENT 9240

#define TTS_SOCKET_PORT_CLIENT	9241

#define VR_SOCKET_PORT_CLIENT	9242

#define TOUCH_SOCKET_PORT_CLIENT 9340
#define MAX_CHANNELS             6

#define TRUE             1
#define FALSE            0


int fds[6];

struct thread_parms 
{
	/* The file descriptor path.*/ 
	int cmdFd;
	int videoFd;
}; 
 
void* writer_Thread (void* parameters) 
{
	struct thread_parms*  p = (struct thread_parms*) parameters;
	int cmdFd,videoFd;
	cmdFd = p->cmdFd;
	videoFd = p->videoFd;

	printf("In writing thread\n");
	
	//Waiting for 3 secs
	sleep(7);

	//TODO AnalyseHead module
	//Writing to command channel
	writeDataToCmdChannel(cmdFd);
	writeDataToVideoChannel(videoFd);
}
void* listener_Thread (void* parameters) 
{

	int    i, len, rc, on = 1;
	int    listen_sd, max_sd, new_sd,new_listen_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	char   buffer[80];
	struct sockaddr_in   addr;
	struct timeval       timeout;
	//struct fd_set        master_set, working_set;
	fd_set        master_set, working_set;
	int cmdFd,videoFd;


	struct thread_parms*  p = (struct thread_parms*) parameters; 

	cmdFd = p->cmdFd;
	videoFd = p->videoFd;

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	FD_SET(cmdFd, &master_set);
	FD_SET(videoFd, &master_set);
	max_sd = videoFd;//use second


	printf("listen_sd = %d , new_listen_sd = %d\n",listen_sd,new_listen_sd);

	/*************************************************************/
	/* Initialize the timeval struct to 3 minutes.  If no        */
	/* activity after 3 minutes this program will end.           */
	/*************************************************************/
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;

	/*************************************************************/
	/* Loop waiting for incoming connects or for incoming data   */
	/* on any of the connected sockets.                          */
	/*************************************************************/
	do
	{
		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
		memcpy(&working_set, &master_set, sizeof(master_set));

		/**********************************************************/
		/* Call select() and wait 5 minutes for it to complete.   */
		/**********************************************************/
		printf("Waiting on select() in Listening Thread...\n");
		rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

		/**********************************************************/
		/* Check to see if the select call failed.                */
		/**********************************************************/
		if (rc < 0)
		{
			perror("  select() failed");
			break;
		}

		/**********************************************************/
		/* Check to see if the 5 minute time out expired.         */
		/**********************************************************/
		if (rc == 0)
		{
			printf("  select() timed out.  End program.\n");
			break;
		}

		desc_ready = rc;
		if (FD_ISSET(cmdFd, &working_set))
		{
			printf("calling readData for cmdFd\n");
		 	close_conn  = readDataFromCmdChannel(cmdFd); 

			//TODO in separate thread
		 	//writeDataToCMDChannel(cmdFd);
		}
		else if (FD_ISSET(videoFd, &working_set))
		{
			printf("calling readData for videoFd\n");
			//close_conn  = readDataFromVideoChannel(videoFd);
		}
		else
		{
			printf("Error :: invalid channel \n");
		}

	} while (end_server == FALSE);
	
}

int readDataFromCmdChannel(int fd)
{
	char   buffer[80];
	int rc,len;

	printf("Socket fd = %d",fd);

	/**********************************************/
	/* Receive data on this connection until the  */
	/* recv fails with EWOULDBLOCK.  If any other */
	/* failure occurs, we will close the          */
	/* connection.                                */
	/**********************************************/
	rc = recv(fd, buffer, sizeof(buffer), 0);
	if (rc < 0)
	{
	if (errno != EWOULDBLOCK)
	{
		perror("  recv() failed");
		return TRUE;
	}
	}

	/**********************************************/
	/* Check to see if the connection has been    */
	/* closed by the client                       */
	/**********************************************/
	if (rc == 0)
	{
		printf("  Connection closed\n");
		return TRUE;
	}

	/**********************************************/
	/* Data was received                          */
	/**********************************************/
	len = rc;
	printf("  %d bytes received\n", len);

	int k;
	for(k=0; k<len;k++)				  
	{
		printf(" Buffer[%d] = %x\n",k,buffer[k] );
	}
}

int writeDataToVideoChannel(int fd)
{

	int rc;
	char   buffer[80];

	//Data length
	buffer[0] = 0x0;
	buffer[1] = 0x0;
	buffer[2] = 0x0;
	buffer[3] = 0x4;

        //MSG_VIDEO_DATA = 0x00020001
        //Service type
	buffer[8] = 0x0;
	buffer[9] = 0x02;
	buffer[10] = 0x0;
	buffer[11] = 0x01;

	//Data
	buffer[12] = 0x8;
	buffer[13] = 0x1;
	buffer[14] = 0x10;
	buffer[15] = 0x0;
	
	//MSG_VIDEO_DATA = 0x00020001, 8011000

	//Timestamp
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	buffer[6] = 0x00;
	buffer[7] = 0x02;
	
	printf("Socket fd used for send :: %d\n",fd);

	//rc = send(i, buffer, len, 0);
	rc = send(fd, buffer, 16, 0);

	if (rc < 0)
	{
		perror("  send() failed");
	}

}

int writeDataToCmdChannel(int fd)
{
	int rc;
	char   buffer[80];

	buffer[0] = 0x0;
	buffer[1] = 0x4;
	buffer[2] = 0x0;
	buffer[3] = 0x0;


	buffer[8] = 0x8;
	buffer[9] = 0x1;
	buffer[10] = 0x10;
	buffer[11] = 0x0;
	/*
	buffer[20] = 0x8;
	buffer[21] = 0x1;
	buffer[22] = 0x10;
	buffer[23] = 0x0;
	*/
	//MSG_VIDEO_DATA = 0x00020001, 8011000

	buffer[4] = 0x00;
	buffer[5] = 0x01;
	buffer[6] = 0x00;
	buffer[7] = 0x02;

	/*
	buffer[16] = 0x00;
	buffer[17] = 0x02;
	buffer[18] = 0x00;
	buffer[19] = 0x01;
	*/
	printf("Socket fd used for send :: %d\n",fd);

	//rc = send(i, buffer, len, 0);
	rc = send(fd, buffer, 12, 0);

	if (rc < 0)
	{
		perror("  send() failed");
	}
}

int initServer(int port)
{
	printf("xxxx - [initServer] port :: %d\n",port);

	int    i, len, rc, on = 1;
	int    listen_sd, max_sd, new_sd,new_listen_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	char   buffer[80];

	struct sockaddr_in   addr;
	struct timeval       timeout;
	fd_set        master_set, working_set;

	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming       */
	/* connections on                                            */
	/*************************************************************/
	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sd < 0)
	{
		perror("socket() failed");
		exit(-1);
	}

	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
		   (char *)&on, sizeof(on));
	if (rc < 0)
	{
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for    */
	/* the incoming connections will also be nonblocking since  */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
	if (rc < 0)
	{
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);
	rc = bind(listen_sd,
	     (struct sockaddr *)&addr, sizeof(addr));
	if (rc < 0)
	{
		perror("bind() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(listen_sd, 32);
	if (rc < 0)
	{
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	printf("listen_sd is = %d\n",listen_sd);

	return listen_sd;
}


int establishChannels(int socketFds[])
{

	printf("xxxx - [establishChannels] \n");
	int index = 0;
	//Sockets fds are
	for(index= 0; index < MAX_CHANNELS; index++ )
	{
		printf("socketFds[%d] = %d  ",index,socketFds[index]);
	}
	printf("\n");

	int    rc , establishCount = -1;
	int    max_sd, new_sd;
	int    desc_ready, end_server = FALSE;
	char   buffer[80];
	struct sockaddr_in   clientAddr;

	struct timeval       timeout;
	fd_set        master_set, working_set;

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	//for(index= 0; index < MAX_CHANNELS; index++ )
	for(index= 0; index < 2; index++ )
	{
		FD_SET(socketFds[index], &master_set);
	}
		
	max_sd = socketFds[1];//use second


	/*************************************************************/
	/* Initialize the timeval struct to 3 minutes.  If no        */
	/* activity after 3 minutes this program will end.           */
	/*************************************************************/
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;

	/*************************************************************/
	/* Loop waiting for incoming connects or for incoming data   */
	/* on any of the connected sockets.                          */
	/*************************************************************/
	do
	{ 
		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
		memcpy(&working_set, &master_set, sizeof(master_set));

		/**********************************************************/
		/* Call select() and wait 5 minutes for it to complete.   */
		/**********************************************************/
		printf("Waiting on select()...\n");
		rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

		/**********************************************************/
		/* Check to see if the select call failed.                */
		/**********************************************************/
		if (rc < 0)
		{
			perror("  select() failed");
			break;
		}

		/**********************************************************/
		/* Check to see if the 5 minute time out expired.         */
		/**********************************************************/
		if (rc == 0)
		{
			printf("  select() timed out.  End program.\n");
			break;
		}

		for(index= 0; index < MAX_CHANNELS; index++ )
		{
			if (FD_ISSET(socketFds[index], &working_set))
			{
				printf("Command Channel established\n");

				new_sd = accept(socketFds[index], NULL, NULL);
				if (new_sd < 0)
				{
					if (errno != EWOULDBLOCK)
					{
						perror("  accept() failed");
						end_server = TRUE;
					}
					break;//for loop
				}

				printf("  New incoming connection for command channel: %d\n", new_sd);
				
				//store cmd channel fd			
				fds[index] = new_sd;

				FD_SET(new_sd, &master_set);
				if (new_sd > max_sd)
					max_sd = new_sd;

				//Connection is established break the loop
				establishCount++;
				break;//for loop
			}
			else
			{
				;//Loop to next channel
			}
		}//End of for loop

		//Check all chennels established
		//if((MAX_CHANNELS-1) == establishCount)
		if(1 == establishCount)
		{
			printf("All channels are established !!!\n");
			return TRUE;	
		}
	} while (end_server == FALSE);

//Some error occured
return FALSE;
	//TODO
	   /*************************************************************/
	   /* Clean up all of the sockets that are open                  */
	   /*************************************************************/
	/*   for (i=0; i <= max_sd; ++i)
	   {
	      if (FD_ISSET(i, &master_set))
		 close(i);
	   }
	*/

}

int initialize()
{
	int ret = -1;
	int socketFds[6];//Holds fds for six channels
/*
#define CMD_SOCKET_PORT_CLIENT	7240

#define VIDEO_SOCKET_PORT_CLIENT 8240

#define MEDIA_SOCKET_PORT_CLIENT 9240

#define TTS_SOCKET_PORT_CLIENT	9241

#define VR_SOCKET_PORT_CLIENT	9242

#define TOUCH_SOCKET_PORT_CLIENT 9340
*/
	socketFds[0] = initServer(CMD_SOCKET_PORT_CLIENT);
	socketFds[1] = initServer(VIDEO_SOCKET_PORT_CLIENT);
	socketFds[2] = initServer(MEDIA_SOCKET_PORT_CLIENT);
	socketFds[3] = initServer(TTS_SOCKET_PORT_CLIENT);
	socketFds[4] = initServer(VR_SOCKET_PORT_CLIENT);
	socketFds[5] = initServer(TOUCH_SOCKET_PORT_CLIENT);

	ret = establishChannels(socketFds);
	printf("xxxx - [initialize] channel establish status :: %d\n",ret);

	return ret;
}

void startThreads()
{
	//Reader Thread
	pthread_t listener_thread_id; 
	struct thread_parms listener_thread_args; 

	listener_thread_args.cmdFd  = fds[0];
	listener_thread_args.videoFd  = fds[1];
	pthread_create  (&listener_thread_id,  NULL,  &listener_Thread,  &listener_thread_args);

	//Writer thread
	pthread_t writer_thread_id; 
	struct thread_parms writer_thread_args; 

	writer_thread_args.cmdFd  = fds[0];
	writer_thread_args.videoFd  = fds[1];
	pthread_create  (&writer_thread_id,  NULL,  &writer_Thread,  &writer_thread_args);

	pthread_join  (listener_thread_id,  NULL); 
	pthread_join  (writer_thread_id,  NULL);

}
int main (int argc, char *argv[])
{	  
	
	int ret = initialize();
	printf("Initialize :: %d\n",ret);
	if(ret == TRUE)
	{
		printf("Initialization done ... starting writer and listener threads\n");
		startThreads();
	}
	else
	{
		printf("Error :: Initialization failed\n");
	}

	return 0;
}
