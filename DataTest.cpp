#include <iostream>
#include <stdio.h>
#include<string>
#include <cstring>
#include<memory>

using namespace std;
typedef unsigned char u8;
typedef unsigned int u32;

#define CMD_HEAD_LEN 8
#define CMD_DATA_SIZE 40*1024//MSG_CMD_MEDIA_INFO<32k

typedef enum EPackageHeadType{
	//--------------------Command channel-------------------------
       //command channel relative
	//--interactive message for initialization
	MSG_CMD_HU_PROTOCOL_VERSION = 0x00018001
}E_PACKAGE_HEAD_TYPE;

//struct for package(include head and data body)
typedef struct Package{
	u8* packageHead;
	u8* packageData;
}S_PACKAGE;

//package
S_PACKAGE sendPackage;

void setPackageHeadDataSize(u32 size){

		sendPackage.packageHead[0] = (u8) ((size >> 8) & 0xff);
		sendPackage.packageHead[1] = (u8) (size & 0xff);
	
}

void setPackageHeadType(u32 type){

		sendPackage.packageHead[4] = (u8) ((type >> 24) & 0xff);
		sendPackage.packageHead[5] = (u8) ((type >> 16) & 0xff);
		sendPackage.packageHead[6] = (u8) ((type >> 8) & 0xff);
		sendPackage.packageHead[7] = (u8) (type & 0xff);
}
void addAOAHead(u8* dest,u8* src)
{
       for(int i=0 ; i< 8 ; i++)
       {
           dest[i] = src[i];
        }
}
void addCarLifeMsgHead(u8* dest,u8* src)
{
        for(int i=0 ; i< 8 ; i++)
       {
           dest[i + 8] = src[i];
        }
}
void addCarLifeMsgData(u8* dest,u8* src,u32 dataSize)
{
    for(u8 i=0 ; i< dataSize ; i++)
       {
           dest[i + 8 + 8] = src[i];
        } 
}
int main() {
    		sendPackage.packageHead= new u8[CMD_HEAD_LEN];
			sendPackage.packageData= new u8[CMD_DATA_SIZE];
            
            for(int i = 0; i<4 ; i++)
            {
                sendPackage.packageData[i] = (u8) 7;
            }
            //AOA Head 
            u8 * aoaHead = new u8[8];
            
	//set package head type
	setPackageHeadType(MSG_CMD_HU_PROTOCOL_VERSION);

	u32 dataSize = 4;
	//set package data length
	setPackageHeadDataSize(dataSize);
	
	for(int i=0;i<8;i++)
	{
	    printf("packageHead[%d] = %x\n",i,sendPackage.packageHead[i]);
	}
	

	//Prepare AOA Head
	/* 0-3 channel ID
	   4-7 carlife msg length*/
	    aoaHead[0] = (u8) 0x0;
	    aoaHead[1] = (u8) 0x0 ;
    	aoaHead[2] = (u8) 0x0;
    	aoaHead[3] = (u8) 0x1;
    	
    	u32 carLifeMsgSize  = dataSize + 8;
    	printf("carLifeMsgSize = 0x%x\n",carLifeMsgSize);
	    aoaHead[4] = (u8) ((carLifeMsgSize >> 24) & 0xff);
	    aoaHead[5] = (u8) ((carLifeMsgSize >> 16) & 0xff);
    	aoaHead[6] = (u8) ((carLifeMsgSize >> 8) & 0xff);
    	aoaHead[7] = (u8) (carLifeMsgSize & 0xff);
	  
	   u8 * finalMsg = new u8[carLifeMsgSize+8+8];
	   
	   addAOAHead(finalMsg,aoaHead);
	   addCarLifeMsgHead(finalMsg,sendPackage.packageHead);
	   addCarLifeMsgData(finalMsg,sendPackage.packageData,dataSize);
	  
	  for(int i = 8+8 ;i<dataSize + 8+8 ; i++ )
	  {
	      printf("finalMsg[%d] = %x\n",i,finalMsg[i]);
	  }
	  
	return 0;
}
