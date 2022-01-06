#pragma once

#include "PacketSenderBase.h"

namespace KBEngine
{

class PacketSenderKCP : public PacketSenderBase
{
public:
	PacketSenderKCP(class NetworkInterfaceBase* networkInterface);

	~PacketSenderKCP();

	bool Send(uint8* datas, uint32 length) override;

};

}	// end namespace KBEngine