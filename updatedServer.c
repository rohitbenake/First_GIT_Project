#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>


#include <stdlib.h>
#include <pthread.h> 

#define CMD_SERVER_PORT  7240
#define VIDEO_SERVER_PORT  8240

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
	sleep(3);

	//TODO AnalyseHead module
	//Writing to command channel
	writeDataToCmdChannel(cmdFd);
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
	printf("In initServer port :: %d\n",port);

	int    i, len, rc, on = 1;
	int    listen_sd, max_sd, new_sd,new_listen_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	char   buffer[80];

	struct sockaddr_in   addr;
	struct timeval       timeout;
	//struct fd_set        master_set, working_set;
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


void establishChannels(int in_cmdFd,int in_videoFd)
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

	printf("In establishChannels\n");

	cmdFd = in_cmdFd;
	videoFd = in_videoFd;

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

		 if (FD_ISSET(cmdFd, &working_set))
		 {
			printf("Command Channel established\n");

			new_sd = accept(cmdFd, NULL, NULL);
			if (new_sd < 0)
			{
				if (errno != EWOULDBLOCK)
				{
					perror("  accept() failed");
					end_server = TRUE;
				}
				break;
			}

			printf("  New incoming connection for command channel: %d\n", new_sd);

			//store cmd channel fd			
			fds[0] = new_sd;

			FD_SET(new_sd, &master_set);
			if (new_sd > max_sd)
				max_sd = new_sd;
		 }
		else if (FD_ISSET(videoFd, &working_set))
		{
			printf("Video Channel established\n");

			new_sd = accept(videoFd, NULL, NULL);
			if (new_sd < 0)
			{
				if (errno != EWOULDBLOCK)
				{
					perror("  accept() failed");
					end_server = TRUE;
				}
				break;
			}
			
			printf("  New incoming connection for video channel : %d\n", new_sd);

			//store video channel fd			
			fds[1] = new_sd;

			FD_SET(new_sd, &master_set);
			if (new_sd > max_sd)
				max_sd = new_sd;   

			//ASSUMPTION :: All channels established when video channel establioshed(in case of two channels)
			//return to caller
			printf("All channels are established ...Ready for Read\n");
			return;
		}
		else
		{
			//printf("All channels are established ...Ready for Read\n");
			printf("Error :: Invalid connection or read not allowed\n");
			break;
		}
	} while (end_server == FALSE);


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

int establishChannel(int fd)
{
	int    rc;
	int    listen_sd, max_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	struct timeval       timeout;
	fd_set        master_set, working_set;
	int listenFd;
	int new_sd = -1;


	listenFd = fd;

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	FD_SET(listenFd, &master_set);
	max_sd = listenFd;

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

		 if (FD_ISSET(listenFd, &working_set))
		 {
			printf("Channel established for fd : %d\n",listenFd);

			new_sd = accept(listenFd, NULL, NULL);
			if (new_sd < 0)
			{
				if (errno != EWOULDBLOCK)
				{
					perror("  accept() failed");
					end_server = TRUE;
				}
				break;
			}

			printf("  New incoming connection for this channel: %d\n", new_sd);

		 }
		else
		{
			printf("Error :: could not establish channel for fd : %d\n",listenFd);
			break;
		}
	} while (end_server == FALSE);


	return new_sd;
}
int initialize()
{
	int retFd;
	int cmdFd,videoFd;
	cmdFd = initServer(CMD_SERVER_PORT);
	videoFd = initServer(VIDEO_SERVER_PORT);

	printf("In initialize\n");
	establishChannels(cmdFd,videoFd);
#if 0	
	//establish command channel
	retFd = establishChannel(cmdFd);
	if(retFd > 0)	
	{
		printf("Error :: could not establish command channel\n");
		return FALSE;
	}
	else
	{
		fds[0] = retFd;
		printf("Command Channel established\n");	
	}
	
	retFd = establishChannel(videoFd);
	if(retFd > 0)	
	{
		printf("Error :: could not establish video channel\n");
		return FALSE;
	}
	else
	{
		fds[1] = retFd;
		printf("Video Channel established\n");	
	}
#endif
	return TRUE;
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
