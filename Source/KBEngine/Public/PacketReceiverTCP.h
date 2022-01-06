#pragma once

#include "PacketReceiverBase.h"

namespace KBEngine
{

class PacketReceiverTCP : public PacketReceiverBase
{
public:
	PacketReceiverTCP(NetworkInterfaceBase* networkInterface, uint32 buffLength = 65535)
		: PacketReceiverBase(networkInterface, buffLength)
	{}

	void BackgroundRecv() override;

};

}	// end namespace KBEngine