/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>

void printError(char *msg)
{
    perror(msg);
    exit(0);
}

int main()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

/*  Remove any old socket and create an unnamed socket for the server.  */

    unlink("server_socket");
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( server_sockfd < 0) 
        printError("ERROR opening socket");
/*  Name the socket.  */

    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "server_socket");
    server_len = sizeof(server_address);
   if( bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
	  printError("ERROR on binding");

/*  Create a connection queue and wait for clients.  */

    listen(server_sockfd, 5);
    while(1) {
        char ch;
	char buffer[256];
        printf("server waiting\n");
	int n = 0;
/*  Accept a connection.  */

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, 
            (struct sockaddr *)&client_address, &client_len);
   if (client_sockfd < 0) 
          printError("ERROR on accept");

/*  We can now read/write to client on client_sockfd.  */
    
     bzero(buffer,256);
     n = read(client_sockfd,buffer,255);
     if (n < 0) printError("ERROR reading from socket");

     printf("Number of bytes read : %d\n",n);
//while(1)
//{
	typedef unsigned char u8;
	typedef unsigned int u32;

        u8 data [24];
        bzero(data,24);
        u32 dataSize = 4;

	data[0] = (u8) 0x0;
	data[1] = (u8) 0x0;
    	data[2] = (u8) 0x0;
    	data[3] = (u8) 0x2;

    	u32 carLifeMsgSize  = dataSize + 8 +4/*Timestamp in case of video*/ ;

    	printf("carLifeMsgSize = 0x%x\n",carLifeMsgSize);

	data[4] = (u8) ((carLifeMsgSize >> 24) & 0xff);
	data[5] = (u8) ((carLifeMsgSize >> 16) & 0xff);
    	data[6] = (u8) ((carLifeMsgSize >> 8) & 0xff);
    	data[7] = (u8) (carLifeMsgSize & 0xff);

	//Data length
	buffer[8] = 0x0;
	buffer[9] = 0x0;
	buffer[10] = 0x0;
	buffer[11] = 0x4;

	//Timestamp
	buffer[12] = 0x00;
	buffer[13] = 0x01;
	buffer[14] = 0x00;
	buffer[15] = 0x02;

        //MSG_VIDEO_DATA = 0x00020001
        //Service type
	buffer[16] = 0x0;
	buffer[17] = 0x02;
	buffer[18] = 0x0;
	buffer[19] = 0x01;

	//Data
	buffer[20] = 0x8;
	buffer[21] = 0x1;
	buffer[22] = 0x10;
	buffer[23] = 0x0;
	
	//MSG_VIDEO_DATA = 0x00020001, 8011000

     n = write(client_sockfd,data,24);
     if (n < 0) printError("ERROR writing to socket");
//}

/*
        read(client_sockfd, &ch, 1);
        ch++;
        write(client_sockfd, &ch, 1);
*/
       // close(client_sockfd);
    }
}
