#include "NetworkInterfaceKCP.h"
#include "PacketReceiverKCP.h"
#include "KBEngineApp.h"
#include "ikcp.h"
#include "NetworkStatus.h"
#include "PacketSenderKCP.h"

namespace KBEngine
{

PacketReceiverBase* NetworkInterfaceKCP::CreatePacketReceiver()
{
	return new PacketReceiverKCP(this);
}

void NetworkInterfaceKCP::InitPacketSender()
{
	packetSender_ = new PacketSenderKCP(this);
}

void NetworkInterfaceKCP::Process()
{
	if (kcp_)
	{
		uint64 secs64 = FPlatformTime::Seconds() * 1000;	// ±äÎªºÁÃë£¬±ÜÃâÒç³ö
		uint32 current = secs64 & 0xfffffffful;

		if (current >= nextTickKcpUpdate_) 
		{
			ikcp_update(kcp_, current);
			nextTickKcpUpdate_ = ikcp_check(kcp_, current);
		}
	}

	NetworkInterfaceBase::Process();
}

void NetworkInterfaceKCP::Close()
{
	finiKCP();
	NetworkInterfaceBase::Close();
}

bool NetworkInterfaceKCP::Valid()
{
	if (willClose_)
		return false;

	return socket_ != nullptr;
}

void NetworkInterfaceKCP::StartConnect(ConnectState state)
{
	networkStatus_ = new NetworkStatusKCPConnecting(state);
}

void NetworkInterfaceKCP::OnConnected(ConnectState state)
{
	bool success = (state.error == "" && Valid());
	if (success)
	{
		connID_ = ((NetworkStatusKCPConnecting *)networkStatus_)->GetConnID();

		initKCP();

		bool bIsValid;
		addr_->SetIp(*state.connectIP, bIsValid);
		addr_->SetPort(state.connectPort);

		packetReceiver_ = CreatePacketReceiver();
		//packetReceiver_->StartBackgroundRecv();
		ChangeNetworkStatus(new Status_Connected);
	}
	else
	{
		KBE_DEBUG(TEXT("NetworkInterface::OnConnected(), connect to '%s:%d' fault! err: %s!"), *state.connectIP, state.connectPort, *state.error);
		socketSubsystem_->DestroySocket(socket_);
		socket_ = nullptr;
		socketSubsystem_ = nullptr;
		ChangeNetworkStatus(nullptr);
	}

	if (state.connectCB)
		state.connectCB(state.connectIP, state.connectPort, success);
}

bool NetworkInterfaceKCP::initKCP()
{
	kcp_ = ikcp_create((IUINT32)connID_, (void*)this);
	kcp_->output = &NetworkInterfaceKCP::kcp_output;
	kcp_->writelog = &NetworkInterfaceKCP::kcp_writeLog;
	/*
	kcp_->logmask |= (IKCP_LOG_OUTPUT | IKCP_LOG_INPUT | IKCP_LOG_SEND | IKCP_LOG_RECV | IKCP_LOG_IN_DATA | IKCP_LOG_IN_ACK |
		IKCP_LOG_IN_PROBE | IKCP_LOG_IN_WINS | IKCP_LOG_OUT_DATA | IKCP_LOG_OUT_ACK | IKCP_LOG_OUT_PROBE | IKCP_LOG_OUT_WINS);
	*/

	ikcp_setmtu(kcp_, 1400);

	ikcp_wndsize(kcp_, KBEngineApp::app->GetUdpSendBufferMax(), KBEngineApp::app->GetUdpRecvBufferMax());
	ikcp_nodelay(kcp_, 1, 10, 2, 1);
	kcp_->rx_minrto = 10;
	nextTickKcpUpdate_ = 0;
	return true;
}

bool NetworkInterfaceKCP::finiKCP()
{
	if (!kcp_)
		return true;

	ikcp_release(kcp_);
	kcp_ = NULL;

	return true;
}

int NetworkInterfaceKCP::kcp_output(const char* buf, int len, ikcpcb* kcp, void* user)
{
	NetworkInterfaceKCP* pNetworkInterfaceKCP = (NetworkInterfaceKCP*)user;
	//KBE_ERROR(TEXT("shufeng-->>>NetworkInterfaceKCP::kcp_output send --->>> at time: %d"), FDateTime::UtcNow().GetMillisecond());
	return pNetworkInterfaceKCP->SendTo(buf, len);
}

void NetworkInterfaceKCP::kcp_writeLog(const char* log, ikcpcb* kcp, void* user)
{
	KBE_DEBUG(TEXT("NetworkInterfaceKCP::kcp_writeLog: %s!"), *FString(log));
}

int NetworkInterfaceKCP::SendTo(const char* buf, int len)
{
	int32 sent = 0;
	socket_->SendTo((uint8*)buf, len, sent, *addr_);

	return 0;
}

bool NetworkInterfaceKCP::InitSocket(uint32 receiveBufferSize, uint32 sendBufferSize)
{
	socketSubsystem_ = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	if (socketSubsystem_ != nullptr)
	{
		socket_ = socketSubsystem_->CreateSocket(NAME_DGram, TEXT("KBEngine"), false);

		if (socket_ == nullptr)
		{
			KBE_ERROR(TEXT("NetworkInterfaceKCP::InitSocket(): socket could't be created!"));
			return false;
		}

		if (!socket_->SetNonBlocking(true))
		{
			KBE_ERROR(TEXT("NetworkInterfaceKCP::InitSocket: SetNonBlocking(true) error: %d"), 
				(int32)ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode());

			socketSubsystem_->DestroySocket(socket_);

			socket_ = nullptr;
			return false;
		}
	}
	return true;
}

}	// end namespace KBEngine