
#define CMD_SOCKET_PORT_CLIENT	(7240)
#define VIDEO_SOCKET_PORT_CLIENT (8240)
#define MEDIA_SOCKET_PORT_CLIENT (9240)
#define TTS_SOCKET_PORT_CLIENT	(9241)
#define VR_SOCKET_PORT_CLIENT	(9242)
#define TOUCH_SOCKET_PORT_CLIENT (9340)
#define MAX_CHANNELS             (6)

#define TRUE             (1)
#define FALSE            (0)

#define INT_VALUE_ZERO	(0)             
#define INT_VALUE_ONE	(1)     
#define NEGATIVE_VALUE_ONE	(-1)        

#define SERVER_ACCEPT_TIMEOUT_SEC	(3*60)
#define SERVER_ACCEPT_TIMEOUT_USEC	(0)
#define PENDING_QUEUE_LENGTH	(32)
#define SERVER_READ_TIMEOUT_SEC	(3*60)
#define SERVER_READ_TIMEOUT_USEC	(0)


int g_channelFds[MAX_CHANNELS];

struct thread_parms 
{
	/* The file descriptor path.*/ 
	int cmdFd;
	int videoFd;
	int channelFds [6] ;
}; 

typedef enum EChannelType{
	CMD_CHANNEL=0,
	VIDEO_CHANNEL=1,
	MEDIA_CHANNEL=2,
	TTS_CHANNEL=3,
	VR_CHANNEL=4,
	CTRL_CHANNEL=5
}E_CHANNEL_TYPE;

struct tMM_ClientDetails{
	E_CHANNEL_TYPE channel;
	int portNumber;
	int listenFd;
	int socketFd;
	int remotePort;
	std::string remoteAddr;
};

std::vector<tMM_ClientDetails> m_vctClientDetails;
