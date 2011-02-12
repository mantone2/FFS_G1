/////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Michael Antonelli			Four Factor Studios					2011
//
//		FFS_NETIO is intended to be a set of high level helper functions for
//		network communication. 
//
/////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef FFS_NETIO
#define FFS_NETIO

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

int NYI()
{
	return 0;
}

#ifndef FFS_TYPES
#define FFS_TYPES
//Because WIN32 does not have headers for these types
//we include them here for portability
#ifdef WIN32

	//Type definitions for compatable unix types
	typedef unsigned	__int8	uint8_t;
	typedef unsigned	__int16	uint16_t;
	typedef unsigned	__int32	uint32_t;
	typedef unsigned	__int64	uint64_t;
	typedef	signed		__int8	int8_t;
	typedef	signed		__int16	int16_t;
	typedef	signed		__int32	int32_t;
	typedef	signed		__int64	int64_t;
	typedef unsigned	__int8	bool;
#else //assume the other OS is unix based....
#include <sys/inttypes.h>
typedef uint8_t bool;
#endif

//The following section starts up boolean logic

#ifndef FFS_BOOLEAN
#define FFS_BOOLEAN
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef false
#undef false
#endif
#ifdef true
#undef true
#endif
#define FALSE 0
#define TRUE 1
#define false 0
#define true 1
#endif
#endif

bool FFS_NET_INIT_STATUS = false;

#ifdef WIN32

#include "WinSock2.h"
#include <Windows.h>
typedef SOCKET			socketFd_t;

bool	win32StartNet()
{
	WORD wVersionRequested;
	int err;	
	WSADATA wsaData;	
	wVersionRequested = MAKEWORD(2,2);
	err = WSAStartup(wVersionRequested,&wsaData);
	if (err != 0)
	{
		return false;
	}	
	return true;
}

bool	win32StopNet(void)
{
	WSACleanup();
	return true;
}

int		win32ErrorNet()
{
	return WSAGetLastError();
}

bool	win32StartTCP(SOCKET *someSock, struct sockaddr_in *someAddr, int somePort,unsigned long Address)
{
	int rv;
	
	*someSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	someAddr->sin_family = AF_INET;
	someAddr->sin_port = htons(somePort);
	someAddr->sin_addr.s_addr = Address;
	memset(someAddr->sin_zero,0,8);
	rv = connect(*someSock,(SOCKADDR*)someAddr,sizeof(*someAddr));
	if (rv < 0)
	{
		//Error
		closesocket(*someSock);
		*someSock = 0;
		return false;
	}
	return true;
}

#define FFS_NET_START()					win32StartNet()
#define FFS_NET_STOP()					win32StopNet()
#define FFS_NET_ERROR()					win32ErrorNet()
#define FFS_NET_SOCK_TCP(X,Y,Z,Q)		win32StartTCP(X,Y,Z,Q)
#define FFS_NET_SOCK_UDP()				NYI()
#define	FFS_NET_SLEEP()					Sleep(X)
#endif


/////////////////////////////////////////////////////////////
//
// Following is the header info
//
/////////////////////////////////////////////////////////////

			int32_t			FFS_Net_Initialize();													//initializes network connection
			socketFd_t		FFS_Net_ConnectToPeer(int Port, unsigned long Address);					//connects peer to peer to another user
			bool			FFS_Net_AConnectToPeer();												//connects peer to peer to another user (async)
			bool			FFS_Net_AListenForPeerConnection();										//waits for a peer connection (async)
			bool			FFS_Net_AListenForPeerConnectionFrom();									//waits for a peer connection from a specific ip (async)
			bool			FFS_Net_Cleanup();														//Cleans up network			


#ifdef __cplusplus
};
#endif

#endif