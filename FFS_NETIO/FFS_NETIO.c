#include "FFS_NETIO.h"
#include <SDL.h>
#include <stdlib.h>

			int32_t			FFS_Net_Initialize()
{
	int err = 0; //err will contain a socket error code. 0 = no error

	if(!FFS_NET_INIT_STATUS)
	{
		if(!FFS_NET_START())
		{
			err = (int32_t)FFS_NET_ERROR();
		}
		else
		{
			FFS_NET_INIT_STATUS = true;
		}
	}
	return err;
}

			socketFd_t		FFS_Net_TCP_ConnectToPeer(int Port, unsigned long Address)
{
	//-1 = net not init
	//-9 = 5x timeouts 
	uint32_t attempts;
	struct sockaddr_in myAddr;
	socketFd_t mySock;
	int32_t err;

	attempts = 0;	

	if(!FFS_NET_INIT_STATUS) //If the network has not been initialized
	{
		return -1; //return false (error)
	}
	while (1)
	{
		if(FFS_NET_SOCK_TCP(&mySock,&myAddr,Port,Address))
		{
			break;
		}		
		err = FFS_NET_ERROR();
		//log the error
		attempts++;
		if(attempts >= 5)
		{
			return -9; //shit timed out
		}
	}
	return mySock;
}
			//connects peer to peer to another user
			bool			FFS_Net_TCP_AConnectToPeer(int Port, unsigned long Address, void(*CallBack)(socketFd_t Socket, unsigned long Address))
{		
	struct AHelperData *myData;

	if(!FFS_NET_INIT_STATUS) //If the network has not been initialized
	{
		return false; //return false (error)
	}
	myData = (struct AHelperData*) malloc(sizeof(struct AHelperData));
	
	if(myData <= 0)
	{
		//malloc error
		return false;
	}

	myData->Address		=	Address;
	myData->Port		=	Port;
	myData->CallBack	=	CallBack;

	if (SDL_CreateThread(&FFS_Net_TCP_AConnectToPeerHelper,myData) <= 0)
	{
		free(myData);
		return false;
	}
	return true;
}

			int				FFS_Net_TCP_AConnectToPeerHelper(void* Data)
{
	//-1 = net not init
	//-9 = 5x timeouts 
	uint32_t attempts;
	struct sockaddr_in myAddr;
	socketFd_t mySock;
	int32_t err;
	struct AHelperData *HelperData; //using this to cast.

	HelperData = (struct AHelperData*) Data;
	attempts = 0;	

	if(!FFS_NET_INIT_STATUS) //If the network has not been initialized
	{		
		(*(HelperData->CallBack))(-1,0); //Callback using -1 for socket. bad socket id.
		free(Data); //free data
		return -1; //return false (error)
	}
	while (1)
	{
		if(FFS_NET_SOCK_TCP(&mySock,&myAddr,HelperData->Port,HelperData->Address))
		{
			break;
		}		
		err = FFS_NET_ERROR();
		//log the error
		attempts++;
		if(attempts >= 5)
		{
			(*(HelperData->CallBack))(-9,0); //Callback using -9 for socket. bad socket id.
			free(Data);
			return -9; //shit timed out
		}
	}
	(*(HelperData->CallBack))(mySock,HelperData->Address); 
	free(Data);
	return mySock;	
}
			bool			FFS_Net_TCP_AListenForPeer(int Port, void(*CallBack)(socketFd_t Socket, unsigned long Address))
{
	return FFS_Net_TCP_AListenForPeerFrom(Port,0,CallBack);		
}			
			bool			FFS_Net_TCP_AListenForPeerFrom(int Port, unsigned long Address, void(*CallBack)(socketFd_t Socket, unsigned long Address))
{
	struct AHelperData *myData;

	if(!FFS_NET_INIT_STATUS)
	{
		return false;
	}
	myData = (struct AHelperData*) malloc(sizeof(struct AHelperData));

	if(myData <= 0)
	{
		//malloc error
		return false;
	}

	myData->Address = Address;
	myData->Port = Port;
	myData->CallBack = CallBack;

	if(SDL_CreateThread(&FFS_Net_TCP_AListenForPeerHelper,myData) <= 0)
	{
		free(myData);
		return false;
	}
	return true;
}

			int				FFS_Net_TCP_AListenForPeerHelper(void* Data)
{
	//-1 = net not init
	//-9 = 5x timeouts 
	uint32_t attempts;
	struct sockaddr_in myAddr;
	struct sockaddr_in someAddr;
	socketFd_t mySock;
	socketFd_t newSock;
	int32_t err;
	struct AHelperData *HelperData; //using this to cast.
	int sockaddr_Size;

	HelperData = (struct AHelperData*) Data;
	attempts = 0;	

	if(!FFS_NET_INIT_STATUS) //If the network has not been initialized
	{		
		(*(HelperData->CallBack))(-1,0); //Callback using -1 for socket. bad socket id.
		free(Data); //free data
		return -1; //return false (error)
	}
	if(!FFS_NET_LIS_TCP(&mySock,&myAddr,HelperData->Port))
	{
		err = (int32_t)FFS_NET_ERROR();
		(*(HelperData->CallBack))(-1,0);
		free(Data);
		return -1;
	}
	
	sockaddr_Size = sizeof(struct sockaddr_in);

	newSock = (socketFd_t) accept(mySock,(SOCKADDR*)&someAddr,&sockaddr_Size);

	if(newSock <= 0)
	{
		err = (int32_t)FFS_NET_ERROR();
		closesocket(mySock);
		(*(HelperData->CallBack))(-1,0);
		free(Data);
		return -1;
	}

	if(HelperData->Address != 0 && HelperData->Address != someAddr.sin_addr.s_addr)
	{		
		closesocket(mySock);
		closesocket(newSock);
		(*(HelperData->CallBack))(-1,someAddr.sin_addr.s_addr);
		free(Data);
		return -1;
	}
	(*(HelperData->CallBack))(mySock,someAddr.sin_addr.s_addr); 
	free(Data);
	return mySock;	
}
			bool			FFS_Net_Cleanup()
{
	FFS_NET_INIT_STATUS = false;
	WSACleanup();
	return true;
}