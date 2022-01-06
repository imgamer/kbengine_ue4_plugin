
#include "NetworkInterfaceTCP.h"
#include "PacketReceiverTCP.h"
#include "PacketSenderTCP.h"
#include "KBEngineApp.h"

namespace KBEngine
{

PacketReceiverBase* NetworkInterfaceTCP::CreatePacketReceiver()
{
	return new PacketReceiverTCP(this, KBEngineApp::app->GetTcpRecvBufferMax());
}

void NetworkInterfaceTCP::InitPacketSender()
{
	packetSender_ = new PacketSenderTCP(this, KBEngineApp::app->GetTcpSendBufferMax());
}

void NetworkInterfaceTCP::StartConnect(ConnectState state)
{
	networkStatus_ = new Status_Connecting(state);
}

bool NetworkInterfaceTCP::InitSocket(uint32 receiveBufferSize, uint32 sendBufferSize)
{
	socketSubsystem_ = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	if (socketSubsystem_ != nullptr)
	{
		socket_ = socketSubsystem_->CreateSocket(NAME_Stream, TEXT("KBEngine"), true);

		if (socket_ != nullptr)
		{
			bool Error = !socket_->SetReuseAddr(true) ||
				!socket_->SetLinger(false, 0) ||
				!socket_->SetRecvErr();

#if PLATFORM_WINDOWS
			if (!Error)
			{
				//int Param = 1;
				//Error = setsockopt(((FSocketBSD*)socket_)->GetNativeSocket(), IPPROTO_TCP, TCP_NODELAY, (char*)&Param, sizeof(Param)) == 0;
			}
#endif

			if (!Error)
			{
				//Error = !socket_->SetNonBlocking(true);
				Error = !socket_->SetNonBlocking(false);
			}

			if (!Error)
			{
				int32 OutNewSize;

				if (receiveBufferSize > 0)
				{
					socket_->SetReceiveBufferSize(receiveBufferSize, OutNewSize);
				}

				if (sendBufferSize > 0)
				{
					socket_->SetSendBufferSize(sendBufferSize, OutNewSize);
				}
			}

			if (Error)
			{
				KBE_ERROR(TEXT("KBENetwork::InitSocket: Failed to create socket"));

				socketSubsystem_->DestroySocket(socket_);

				socket_ = nullptr;
				return false;
			}
		}
	}
	return true;
}

}	// end namespace KBEngine