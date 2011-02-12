////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2011 by Four Factor Studios
//                         All Rights Reserved
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Michael Antonelli			Four Factor Studios				(C) 2011
//
//		FFS_NETIO is intended to be a set of high level helper functions for
//		network communication. 
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "FFS_NETIO.h"
#include <SDL.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_initialize
//
//	Starts up networking for OSes which require special network configs like Windows
//	Sets a global variable saying network is ready.
//	Call FFS_Net_Cleanup to cleanup networking.
//	Returns a signed 32int with error code, if any.
//	Returns 0 on success
//
/////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_ConnectToPeer
//
//	Creates a TCP socket then auto-connects to a peer location.
//	Takes a port number and unsigned long IP address.
//	IP Address is made using inet_ntoa on a string containing the IP
//	Function is synchronous, blocks, and will keep attempting up to 5 times.
//	returns -1 on not init network. -9 on timeouts.
//	returns the socket descriptor on success.
/////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_AConnectToPeer
//
//	This is an Async blocking function varient of FFS_Net_TCP_ConnectToPeer
//	This will spawn a thread which will attempt to connect to a client.
//	If it spawns correctly it will return true, and the specified callback function
//	will be called upon successful connection.
//	Returns false on error.
//
/////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_AConnectToPeerHelper
//
//	This is the threaded helper for FFS_Net_TCP_AConnectToPeer
//	This function runs in a thread made by SDL_CreateThread
//	It takes a pointer to an AHelperData struct as a parameter.
//	On success it will push a callback function to the stack and return the 
//	socket descriptor.
//	On failure it returns a value <= 0
/////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_AListenForPeer
//
//	An async function which creates a port, binds it, and listens for a connection.
//	This function is actually a special case of FFS_Net_TCP_AListenForPeerFrom
/////////////////////////////////////////////////////////////////////////////////////
			bool			FFS_Net_TCP_AListenForPeer(int Port, void(*CallBack)(socketFd_t Socket, unsigned long Address))
{
	return FFS_Net_TCP_AListenForPeerFrom(Port,0,CallBack);		
}
/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_AListenForPeerFrom
//
//	Begins the process of async listening for a connection from a remote peer.
//	This will spawn a thread which will wait for a single connection
//	It will return true of the thread spawns correctly, otherwise false.
/////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_TCP_AListenForPeerHelper
//
//	This is a helper function for FFS_Net_TCP_AListenForPeerFrom
//	It takes an AHelperData struct as a parameter
//	It will create and bind a TCP socket, and begin listening for the first connection
//	Even though it is async, it is not necessarily multiple re-entrant safe
//	This class of functions should be replaced with a true listener server later
//	This function will check the IP of the first client to connect,
//	If the caller specified no IP address, it will automatically allow the first client
//	otherwise it will check and if the IP does not match what the caller specified,
//	it will close the socket.
//	On successful connection this will push a callback function, and pass the 
//	socket descriptor along with the remote client IP address.
//	On failure it will send a <=0 value for socket.
/////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////
//
//	FFS_Net_Cleanup
//
//	Calls cleanup functions for network.
/////////////////////////////////////////////////////////////////////////////////////
			bool			FFS_Net_Cleanup()
{
	FFS_NET_INIT_STATUS = false;
	FFS_NET_STOP();
	return true;
}