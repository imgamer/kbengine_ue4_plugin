#include "PacketSenderKCP.h"
#include "NetworkInterfaceKCP.h"
#include "ikcp.h"
#include "KBEDebug.h"
#include "NetworkInterfaceBase.h"


namespace KBEngine
{

PacketSenderKCP::PacketSenderKCP(NetworkInterfaceBase* networkInterface)
	: PacketSenderBase(networkInterface)
{
}

PacketSenderKCP::~PacketSenderKCP()
{
}

bool PacketSenderKCP::Send(uint8* datas, uint32 length)
{
	NetworkInterfaceKCP* networkInterface = (NetworkInterfaceKCP*)networkInterface_;

	networkInterface->NextTickKcpUpdate();

	//KBE_ERROR(TEXT("PacketSenderKCP::Send:"));
	//hexlike(datas, 0, length);
	//KBE_ERROR(TEXT("shufeng-->>>PacketSenderKCP::Send data(len:%d) --->>> at time: %s:%d"), 
	//	length, *FDateTime::UtcNow().ToString(), FDateTime::UtcNow().GetMillisecond());

	if (ikcp_send(networkInterface->KCP(), (const char *)datas, length) < 0)
	{
		KBE_ERROR(TEXT("PacketSenderKCP::send: send error!currPacketSize %d, ikcp_waitsnd = %d"), length, ikcp_waitsnd(networkInterface->KCP()));

		return false;
	}

	return true;
}

}	// end namespace KBEngine