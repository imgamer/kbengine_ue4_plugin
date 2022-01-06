#pragma once

#include "PacketReceiverBase.h"
#include "Templates/SharedPointer.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"

namespace KBEngine
{

class NetworkInterfaceBase;

class PacketReceiverKCP : public PacketReceiverBase
{

const int UDP_PACKET_LENTH = 1472*4;

public:
	PacketReceiverKCP(NetworkInterfaceBase* networkInterface)
		: PacketReceiverBase(networkInterface),
		remoteAddr_(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr()),
		udpBuffer_(new uint8[UDP_PACKET_LENTH])
	{}

	~PacketReceiverKCP();

protected:
	void BackgroundRecv() override;

	uint32 CheckForSpace();

protected:
	TSharedRef<FInternetAddr> remoteAddr_;

	uint8* udpBuffer_;

};

}	// end namespace KBEngine