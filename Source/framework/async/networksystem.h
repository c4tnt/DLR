// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __NETWORKSYSTEM_H__
#define __NETWORKSYSTEM_H__


/*
===============================================================================

  Network System.

===============================================================================
*/

class idNetworkSystem {
public:
	virtual					~idNetworkSystem( void ) {} //0

	virtual void			ServerSendReliableMessage( int clientNum, const idBitMsg &msg ); //1
	virtual void			ServerSendReliableMessageExcluding( int clientNum, const idBitMsg &msg ); //2

	// Some function is not exist in 1.0 APIv, use carefully from here
	virtual int				ServerGetClientPing( int clientNum ); //3
	virtual int				ServerGetClientPrediction( int clientNum ); //4
	virtual int				ServerGetClientTimeSinceLastPacket( int clientNum ); //5
#if API_VERS >= 3
	virtual int				ServerGetClientTimeSinceLastInput( int clientNum ); //6
#endif
	virtual int				ServerGetClientOutgoingRate( int clientNum ); //7
	virtual int				ServerGetClientIncomingRate( int clientNum ); //?

	// To here
	virtual float			ServerGetClientIncomingPacketLoss( int clientNum ); //8

	virtual void			ClientSendReliableMessage( const idBitMsg &msg ); //9
	virtual int				ClientGetPrediction( void ); //10
	virtual int				ClientGetTimeSinceLastPacket( void ); //11
	virtual int				ClientGetOutgoingRate( void ); //12
	virtual int				ClientGetIncomingRate( void ); //13
	virtual float			ClientGetIncomingPacketLoss( void ); //14
};

extern idNetworkSystem *	networkSystem;

#endif /* !__NETWORKSYSTEM_H__ */
