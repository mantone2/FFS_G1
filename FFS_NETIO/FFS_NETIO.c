#include "FFS_NETIO.h"

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

			socketFd_t		FFS_Net_ConnectToPeer(int Port, unsigned long Address)
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
			bool			FFS_Net_AConnectToPeer(){}
			bool			FFS_Net_AListenForPeerConnection(){}			//waits for a peer connection
			bool			FFS_Net_AListenForPeerConnectionFrom(){}		//waits for a peer connection from a specific ip
			bool			FFS_Net_Cleanup()
{
	FFS_NET_INIT_STATUS = false;
	WSACleanup();
}