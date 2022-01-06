#include "PacketSenderBase.h"
#include "KBEnginePrivatePCH.h"
#include "NetworkInterfaceBase.h"


namespace KBEngine
{
	PacketSenderBase::PacketSenderBase(NetworkInterfaceBase* networkInterface) :
		networkInterface_(networkInterface)
	{
	}

	PacketSenderBase::~PacketSenderBase()
	{
	}

}

