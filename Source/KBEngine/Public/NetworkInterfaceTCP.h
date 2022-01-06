#pragma once

#include "NetworkInterfaceBase.h"
#include "MessageReader.h"

namespace KBEngine
{

class NetworkInterfaceTCP : public NetworkInterfaceBase
{
public:
	NetworkInterfaceTCP(MessageReader* messageReader)
		: NetworkInterfaceBase(messageReader)
	{}

protected:
	PacketReceiverBase* CreatePacketReceiver() override;

	bool InitSocket(uint32 receiveBufferSize = 0, uint32 sendBufferSize = 0) override;

	void InitPacketSender() override;

	void StartConnect(ConnectState state) override;

};


}	// end namespace KBEngine