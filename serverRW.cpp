
#include <iostream>
#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#include<unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include <vector>
#include <signal.h> 
#include <sys/wait.h> 

#include <memory.h> 

#include "serverTypes.h"

//Initialize 
int initialize();

int initServer(int port);
int establishChannels(int socketFds[]);

void startThreads();

//Reader and Writer threads
void* listener_Thread (void* parameters) ;
void* writer_Thread (void* parameters) ;


int readDataFromCmdChannel(int fd,int);
int readDataFromCtrlChannel(int fd);
int writeDataToVideoChannel(int fd);
int writeDataToCmdChannel(u8 * ,int ,int);

void createUnixClient()
{
	int sockfd;
	int len;

	struct sockaddr_un address;
	int result;
	char ch = 'A';
	char buffer[256];
	int n = 0;
	
	/*  Create a socket for the client.  */
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	/*  Name the socket, as agreed with the server.  */

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, "server_socket");
	len = sizeof(address);

	/*  Now connect our socket to the server's socket.  */
	result = connect(sockfd, (struct sockaddr *)&address, len);

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}

	/*  We can now read/write via sockfd.  */

	printf("createUnixClient sockfd :: %d\n",sockfd);
	g_unixSocketFd = sockfd;
}

u32 getChannelType(u8 * data)
{
        u32 channelId = INT_VALUE_ZERO;
	u32 dataHH = INT_VALUE_ZERO;
	u32 dataHL = INT_VALUE_ZERO;
	u32 dataLH = INT_VALUE_ZERO;
	u32 dataLL = INT_VALUE_ZERO;

	dataHH = (data[0] & 0xff);
	dataHL = (data[1] & 0xff);
	dataLH = (data[2] & 0xff);
	dataLL = (data[3] & 0xff);

        channelId = ((dataHH << 24) & 0xff000000) | ((dataHL << 16) & 0x00ff0000)| ((dataLH << 8) & 0x0000ff00) | ((dataLL) & 0x000000ff);

	return channelId;
}

u32 getDataLength(u8 * data)
{
        u32 dataLength = INT_VALUE_ZERO;
	u32 dataHH = INT_VALUE_ZERO;
	u32 dataHL = INT_VALUE_ZERO;
	u32 dataLH = INT_VALUE_ZERO;
	u32 dataLL = INT_VALUE_ZERO;

	dataHH = (data[4] & 0xff);
	dataHL = (data[5] & 0xff);
	dataLH = (data[6] & 0xff);
	dataLL = (data[7] & 0xff);

        dataLength = ((dataHH << 24) & 0xff000000) | ((dataHL << 16) & 0x00ff0000)| ((dataLH << 8) & 0x0000ff00) | ((dataLL) & 0x000000ff);

	return dataLength;
}

void getCarlifeMsg(u8 * carLifeMsg ,u8 * data, u32 dataLength )
{
	memcpy(carLifeMsg,data+HEAD_LEN,dataLength);
}

int analyseHead(char * head)
{
	//
} 

void* writer_Thread (void* parameters) 
{
	struct thread_parms*  p = (struct thread_parms*) parameters;
	int cmdFd,videoFd;
	int channelFds[MAX_CHANNELS];
	int unixSocketFd;

	u8 * data = new u8[CMD_DATA_SIZE];

	unixSocketFd = p->unixSocketFd;
	for(int index= INT_VALUE_ZERO ;index < MAX_CHANNELS ; index++)
	{
		channelFds[index] = p->channelFds[index];
	}
	int end_server = FALSE;
	int close_conn;

	printf("In writing thread\n");
	printf("Unix unixSocketFd :: %d\n",unixSocketFd);
	//Waiting for 3 secs
	int number = 0;
	int ret = 0;
	int res;
	struct timeval mTimeout;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(unixSocketFd, &set);
	mTimeout.tv_sec = SERVER_READ_TIMEOUT_SEC;
	mTimeout.tv_usec = SERVER_READ_TIMEOUT_USEC;

do 
{
	        do {
		       ret = select((unixSocketFd + 1), &set, NULL, NULL, &mTimeout);
			std::cout<<"\033[1;34m::select done with ret :: \033[0m"<<ret<<std::endl;
		}while (ret <= 0);

	number = read(unixSocketFd,data,CMD_DATA_SIZE);
	printf("Number of bytes read :: %d\n",number);
	int k;
	for(k= INT_VALUE_ZERO; k<number;k++)				  
	{
		printf(" Buffer[%d] = %x\n",k,data[k] );
	}

	 printf("channel Id : %d\n",getChannelType(data));
	 printf("data size : %d\n",getDataLength(data));
	 u32 dataLength = getDataLength(data);
	 u8* carlifeMsg = new u8[dataLength];
	 getCarlifeMsg(carlifeMsg,data,dataLength);//Pass pointer ?
	 writeDataToCmdChannel(carlifeMsg,dataLength,channelFds[VIDEO_CHANNEL]);
}while(end_server == FALSE);


#if 0
	//TODO AnalyseHead module
	//Writing to command channel
	writeDataToCmdChannel(channelFds[CMD_CHANNEL]);
	do {
		sleep(2);
		close_conn = writeDataToVideoChannel(channelFds[VIDEO_CHANNEL]);

		printf("close_conn = %d\n",close_conn);
		if(close_conn == FALSE)
			end_server = TRUE;
	
	}while(end_server == FALSE);
#endif
}
void* listener_Thread (void* parameters) 
{
	int    len, rc;
	int    max_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;
	struct sockaddr_in   addr;
	struct timeval       timeout;
	int unixSocketFd;
	


	fd_set        master_set, working_set;
	int channelFds[MAX_CHANNELS];

	struct thread_parms*  p = (struct thread_parms*) parameters; 

	unixSocketFd = p->unixSocketFd;

	for(int index= INT_VALUE_ZERO ;index < MAX_CHANNELS ; index++)
	{
		channelFds[index] = p->channelFds[index];
		printf("listening socketFds[%d] = %d  ",index,channelFds[index]);		
	}

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	
	for(int index= INT_VALUE_ZERO; index < MAX_CHANNELS; index++ )
	{
		FD_SET(channelFds[index], &master_set);
	}

	max_sd = channelFds[MAX_CHANNELS-INT_VALUE_ONE];//use last

	/*************************************************************/
	/* Initialize the timeval struct to 3 minutes.  If no        */
	/* activity after 3 minutes this program will end.           */
	/*************************************************************/
	timeout.tv_sec  = SERVER_READ_TIMEOUT_SEC;
	timeout.tv_usec = SERVER_READ_TIMEOUT_USEC;

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
		rc = select(max_sd + INT_VALUE_ONE, &working_set, NULL, NULL, &timeout);

		/**********************************************************/
		/* Check to see if the select call failed.                */
		/**********************************************************/
		if (rc < INT_VALUE_ZERO)
		{
			perror("  select() failed");
			break;
		}

		/**********************************************************/
		/* Check to see if the 5 minute time out expired.         */
		/**********************************************************/
		if (rc == INT_VALUE_ZERO)
		{
			printf("  select() timed out.  End program.\n");
			break;
		}

		if (FD_ISSET(channelFds[CMD_CHANNEL], &working_set))
		{
			printf("calling readData for cmdFd\n");
		 	close_conn  = readDataFromCmdChannel(channelFds[CMD_CHANNEL],unixSocketFd); 

			//TODO in separate thread
		 	//writeDataToCMDChannel(cmdFd);
		}
		else if (FD_ISSET(channelFds[VIDEO_CHANNEL], &working_set))
		{
			printf("calling readData for videoFd\n");
			//close_conn  = readDataFromVideoChannel(channelFds[VIDEO_CHANNEL]);
		}
		else if (FD_ISSET(channelFds[MEDIA_CHANNEL], &working_set))
		{
			printf("calling readData for media\n");
		}
		else if (FD_ISSET(channelFds[TTS_CHANNEL], &working_set))
		{
			printf("calling readData for tts\n");
		}
		else if (FD_ISSET(channelFds[VR_CHANNEL], &working_set))
		{
			printf("calling readData for Vr\n");
		}
		else if (FD_ISSET(channelFds[CTRL_CHANNEL], &working_set))
		{
			printf("calling readData for cntr\n");
			close_conn  = readDataFromCtrlChannel(channelFds[CTRL_CHANNEL]); 
		}
		else
		{
			printf("Error :: invalid channel \n");
		}

		if(FALSE == close_conn)
			end_server = TRUE;
	} while (end_server == FALSE);
	
	//TODO	
	//Kill the thread

	/*************************************************************/
	/* Clean up all of the sockets that are open                  */
	/*************************************************************/
	for (int i=INT_VALUE_ZERO; i <= max_sd; ++i)
	{
		if (FD_ISSET(i, &master_set))
			 close(i);
	}
	
}
int readDataFromCtrlChannel(int fd)
{
	char   buffer[80];
	int rc,len,ret = TRUE;

	printf("Socket fd = %d",fd);

	/**********************************************/
	/* Receive data on this connection until the  */
	/* recv fails with EWOULDBLOCK.  If any other */
	/* failure occurs, we will close the          */
	/* connection.                                */
	/**********************************************/
	rc = recv(fd, buffer, sizeof(buffer), INT_VALUE_ZERO);
	if (rc < INT_VALUE_ZERO)
	{
		if (errno != EWOULDBLOCK)
		{
			perror("  recv() failed");
			ret = FALSE;
		}
	}

	/**********************************************/
	/* Check to see if the connection has been    */
	/* closed by the client                       */
	/**********************************************/
	if (rc == INT_VALUE_ZERO)
	{
		printf("  Connection closed\n");
		ret = FALSE;
	}

	/**********************************************/
	/* Data was received                          */
	/*********************************************/
	len = rc;
	printf("  %d bytes received in ctrl channel\n", len);

	int k;
	for(k= INT_VALUE_ZERO; k<len;k++)				  
	{
		printf(" Buffer[%d] = %x\n",k,buffer[k] );
	}

	return ret;
}
int readDataFromCmdChannel(int fd,int unixSocketFd)
{
	char   buffer[80];
	int rc,len,ret = TRUE;

	printf("Socket fd = %d",fd);

	/**********************************************/
	/* Receive data on this connection until the  */
	/* recv fails with EWOULDBLOCK.  If any other */
	/* failure occurs, we will close the          */
	/* connection.                                */
	/**********************************************/
	rc = recv(fd, buffer, sizeof(buffer), INT_VALUE_ZERO);
	if (rc < INT_VALUE_ZERO)
	{
		if (errno != EWOULDBLOCK)
		{
			perror("  recv() failed");
			ret = FALSE;
		}
	}

	/**********************************************/
	/* Check to see if the connection has been    */
	/* closed by the client                       */
	/**********************************************/
	if (rc == INT_VALUE_ZERO)
	{
		printf("  Connection closed\n");
		ret = FALSE;
	}

	/**********************************************/
	/* Data was received                          */
	/*********************************************/
	len = rc;
	printf("  %d bytes received\n", len);

	int k;
	for(k= INT_VALUE_ZERO; k<len;k++)				  
	{
		printf(" Buffer[%d] = %x\n",k,buffer[k] );
	}

    int  n = write(unixSocketFd,buffer,len);
    if (n < 0) 
         printf("ERROR writing to socket");

	return ret;
}

int writeDataToVideoChannel(int fd)
{

	int rc , ret = TRUE;
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

	if (rc < INT_VALUE_ZERO)
	{
		perror("  send() failed");
		ret = FALSE;
	}

	return ret;
}

int writeDataToCmdChannel(u8* buffer,int len,int fd)
{
	int rc;
#if 0
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
#endif
	for(int index= INT_VALUE_ZERO; index < len; index++ )
	{
		printf("Data to be sent :: buffer[%d] = %x\n",index,buffer[index]);
	}

	//rc = send(i, buffer, len, 0);
	rc = send(fd, buffer, len, 0);

	if (rc < INT_VALUE_ZERO)
	{
		perror("  send() failed");
	}
}

int initServer(int port)
{
	printf("xxxx - [initServer] port :: %d\n",port);

	int    i, len, rc, on = INT_VALUE_ONE;
	int    listen_sd, max_sd, new_sd,new_listen_sd;
	int    desc_ready, end_server = FALSE;
	int    close_conn;

	struct sockaddr_in   addr;
	struct timeval       timeout;
	fd_set        master_set, working_set;

	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming       */
	/* connections on                                            */
	/*************************************************************/
	listen_sd = socket(AF_INET, SOCK_STREAM, INT_VALUE_ZERO);
	if (listen_sd < INT_VALUE_ZERO)
	{
		perror("socket() failed");
		exit(NEGATIVE_VALUE_ONE);
	}

	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
		   (char *)&on, sizeof(on));
	if (rc < INT_VALUE_ZERO)
	{
		perror("setsockopt() failed");
		close(listen_sd);
		exit(NEGATIVE_VALUE_ONE);
	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for    */
	/* the incoming connections will also be nonblocking since  */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
	if (rc < INT_VALUE_ZERO)
	{
		perror("ioctl() failed");
		close(listen_sd);
		exit(NEGATIVE_VALUE_ONE);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	memset(&addr, INT_VALUE_ZERO, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port        = htons(port);
	rc = bind(listen_sd,
	     (struct sockaddr *)&addr, sizeof(addr));
	if (rc < INT_VALUE_ZERO)
	{
		perror("bind() failed");
		close(listen_sd);
		exit(NEGATIVE_VALUE_ONE);
	}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(listen_sd, PENDING_QUEUE_LENGTH);
	if (rc < INT_VALUE_ZERO)
	{
		perror("listen() failed");
		close(listen_sd);
		exit(NEGATIVE_VALUE_ONE);
	}

	printf("listen_sd is = %d\n",listen_sd);

	return listen_sd;
}


int establishChannels(int socketFds[])
{

	printf("xxxx - [establishChannels] \n");
	int index = INT_VALUE_ZERO;
	//Sockets fds are
	for(index= INT_VALUE_ZERO; index < MAX_CHANNELS; index++ )
	{
		printf("socketFds[%d] = %d  ",index,socketFds[index]);
	}
	printf("\n");

	int    rc , establishCount = NEGATIVE_VALUE_ONE;
	int    max_sd, new_sd;
	int    desc_ready, end_server = FALSE;


	struct timeval       timeout;
	fd_set        master_set, working_set;

	/*************************************************************/
	/* Initialize the master fd_set                              */
	/*************************************************************/
	FD_ZERO(&master_set);
	
	//for(index= 0; index < 2; index++ )
	for(index= INT_VALUE_ZERO; index < MAX_CHANNELS; index++ )
	{
		FD_SET(socketFds[index], &master_set);
	}
		
	//max_sd = socketFds[1];//use second
	max_sd = socketFds[MAX_CHANNELS - INT_VALUE_ONE];//use second


	/*************************************************************/
	/* Initialize the timeval struct to 3 minutes.  If no        */
	/* activity after 3 minutes this program will end.           */
	/*************************************************************/
	timeout.tv_sec  = SERVER_ACCEPT_TIMEOUT_SEC;
	timeout.tv_usec = SERVER_ACCEPT_TIMEOUT_USEC;

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
		rc = select(max_sd + INT_VALUE_ONE, &working_set, NULL, NULL, &timeout);

		/**********************************************************/
		/* Check to see if the select call failed.                */
		/**********************************************************/
		if (rc < INT_VALUE_ZERO)
		{
			perror("  select() failed");
			break;
		}

		/**********************************************************/
		/* Check to see if the 5 minute time out expired.         */
		/**********************************************************/
		if (rc == INT_VALUE_ZERO)
		{
			printf("  select() timed out.  End program.\n");
			break;
		}

		for(index= INT_VALUE_ZERO; index < MAX_CHANNELS; index++ )
		{
			if (FD_ISSET(socketFds[index], &working_set))
			{
				printf("Channel established\n");

				struct sockaddr_in   clientAddr;
				socklen_t cli_addr_size = sizeof(clientAddr);
				//new_sd = accept(socketFds[index], NULL, NULL);
				new_sd = accept(socketFds[index],(struct sockaddr *)&clientAddr, &cli_addr_size);

				if (new_sd < INT_VALUE_ZERO)
				{
					if (errno != EWOULDBLOCK)
					{
						perror("  accept() failed");
						end_server = TRUE;
					}
					break;//for loop
				}

				printf("  New incoming connection for this channel is : %d\n", new_sd);
				printf("  New incoming connection port : %d\n", clientAddr.sin_port);

				char str[INET_ADDRSTRLEN];						
				inet_ntop(AF_INET, &(clientAddr.sin_addr), str, INET_ADDRSTRLEN);
				printf("  New incoming connection Address : %s\n", str);
				
				//store cmd channel fd			
				g_channelFds[index] = new_sd;

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
		//if(1 == establishCount)
		if((MAX_CHANNELS- INT_VALUE_ONE) == establishCount)
		{
			printf("All channels are established !!!\n");
			return TRUE;	
		}
	} while (end_server == FALSE);

//Some error occured
return FALSE;
}

void initClientDetails()
{
	tMM_ClientDetails l_tClientDetails = {};
	l_tClientDetails.channel = CMD_CHANNEL;
	l_tClientDetails.portNumber = CMD_SOCKET_PORT_CLIENT;

}

int initialize()
{
	int ret = NEGATIVE_VALUE_ONE;
	int socketFds[MAX_CHANNELS];//Holds fds for six channels

	socketFds[CMD_CHANNEL] = initServer(CMD_SOCKET_PORT_CLIENT);
	socketFds[VIDEO_CHANNEL] = initServer(VIDEO_SOCKET_PORT_CLIENT);
	socketFds[MEDIA_CHANNEL] = initServer(MEDIA_SOCKET_PORT_CLIENT);
	socketFds[TTS_CHANNEL] = initServer(TTS_SOCKET_PORT_CLIENT);
	socketFds[VR_CHANNEL] = initServer(VR_SOCKET_PORT_CLIENT);
	socketFds[CTRL_CHANNEL] = initServer(TOUCH_SOCKET_PORT_CLIENT);

	ret = establishChannels(socketFds);
	printf("xxxx - [initialize] channel establish status :: %d\n",ret);

	for(int index= INT_VALUE_ZERO; index < MAX_CHANNELS; index++ )
	{
		printf("g_channelFds[[%d] = %d  ",index,g_channelFds[index]);
	}

	return ret;
}

void startThreads()
{

	createUnixClient();

	//Reader Thread
	pthread_t listener_thread_id; 
	struct thread_parms listener_thread_args; 

	listener_thread_args.unixSocketFd = g_unixSocketFd;
	for(int index = INT_VALUE_ZERO ; index <  MAX_CHANNELS ; index++)
	{
		listener_thread_args.channelFds[index] = g_channelFds[index];
	}
	pthread_create  (&listener_thread_id,  NULL,  &listener_Thread,  &listener_thread_args);

	//Writer thread
	pthread_t writer_thread_id; 
	struct thread_parms writer_thread_args; 
	
	writer_thread_args.unixSocketFd = g_unixSocketFd;
	for(int index = INT_VALUE_ZERO ; index <  MAX_CHANNELS ; index++)
	{
		writer_thread_args.channelFds[index] = g_channelFds[index];
	}

	pthread_create  (&writer_thread_id,  NULL,  &writer_Thread,  &writer_thread_args);

	pthread_join  (listener_thread_id,  NULL); 
	pthread_join  (writer_thread_id,  NULL);
}

// Signal handler to process terminated children
void mysig(int nsig) 
{ 
    signal(SIGCHLD, mysig); 
} 

int main (int argc, char *argv[])
{	  
	signal(SIGCHLD, mysig); // To clean up terminated children

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

