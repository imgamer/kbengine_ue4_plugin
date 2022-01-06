#include "NetworkInterfaceBase.h"
#include "KBEnginePrivatePCH.h"
#include "KBEDebug.h"
#include "KBEPersonality.h"
#include "PacketReceiverBase.h"
#include "PacketReceiverTCP.h"
#include "PacketSenderBase.h"
#include "KBEngineApp.h"
#include "Containers/StringConv.h"
#include "NetworkStatus.h"

#if PLATFORM_WINDOWS
//#include "WinSock2.h"
#endif

namespace KBEngine
{

	NetworkInterfaceBase::NetworkInterfaceBase(MessageReader* messageReader)
	{
		messageReader_ = messageReader;
		KBE_ASSERT(messageReader);
	}


	NetworkInterfaceBase::~NetworkInterfaceBase()
	{
		KBE_DEBUG(TEXT("NetworkInterface::~NetworkInterface()"));
		Reset();
	}

	void NetworkInterfaceBase::Reset()
	{
		if (packetSender_)
			packetSender_->WillClose();

		if (packetReceiver_)
			packetReceiver_->WillClose();

		// 必须先销毁networkStatus_再关闭socket_，以保证多线程下的socket不会出现野指针
		SAFE_DELETE(networkStatus_);

		// 不把socket先关闭，后面PacketReceiver的阻塞将无法终止
		// 从而出现卡机现象
		if (socket_)
			socket_->Close();

		SAFE_DELETE(packetSender_);
		SAFE_DELETE(packetReceiver_);
		willClose_ = false;

		// 这个要放最后，因为在这过程中，PacketSender和PacketReceiver可能还在异步使用这些变量
		if (socket_)
		{
			socketSubsystem_->DestroySocket(socket_);
			socket_ = nullptr;
			socketSubsystem_ = nullptr;
		}
	}

	bool NetworkInterfaceBase::Valid()
	{
		if (willClose_)
			return false;

		return ((socket_ != nullptr) && (socket_->GetConnectionState() == SCS_Connected));
	}

	void NetworkInterfaceBase::ChangeNetworkStatus(NetworkStatus* status)
	{
		if (networkStatus_)
			delete networkStatus_;
		networkStatus_ = status;
	}

	void NetworkInterfaceBase::OnConnected(ConnectState state)
	{
		bool success = (state.error == "" && Valid());
		if (success)
		{
			KBE_DEBUG(TEXT("NetworkInterface::OnConnected(), connect to '%s:%d' is success!"), *state.connectIP, state.connectPort);
			packetReceiver_ = CreatePacketReceiver();
			packetReceiver_->StartBackgroundRecv();
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
		
	void NetworkInterfaceBase::ConnectTo(const FString& host, uint16 port, ConnectCallbackFun callback)
	{
		KBE_ASSERT(!Valid());
		KBE_ASSERT(!networkStatus_);

		if (!InitSocket())
		{
			KBE_ERROR(TEXT("NetworkInterface::ConnectTo: init socket falut."));
		}

		FString ipAddress = this->GetIPAddress(host);

		ConnectState state;
		state.connectIP = ipAddress;
		state.connectPort = port;
		state.connectCB = callback;
		state.socket = socket_;
		state.networkInterface = this;

		StartConnect(state);
	}

	void NetworkInterfaceBase::WillClose()
	{
		willClose_ = true;
	}

	void NetworkInterfaceBase::Close()
	{
		auto wc = willClose_;
		
		Reset();

		if (wc && KBEngineApp::app)
			KBEngineApp::app->OnLoseConnect();
	}

	bool NetworkInterfaceBase::Send(uint8* datas, int32 length)
	{
		if (!Valid())
		{
			//FError::Throwf(TEXT("invalid socket!"));
			KBE_ERROR(TEXT("NetworkInterface::Send: invalid socket!"));
		}

		if (!packetSender_)
		{
			InitPacketSender();
		}

		return packetSender_->Send(datas, length);
	}

	void NetworkInterfaceBase::Process()
	{
		if (willClose_)
		{
			Close();
			return;
		}

		if (networkStatus_)
			networkStatus_->MainThreadProcess(this);
	}

	void NetworkInterfaceBase::ProcessMessage()
	{
		if (packetReceiver_ && messageReader_)
		{
			packetReceiver_->Process(*messageReader_);
		}
	}

	FString NetworkInterfaceBase::GetIPAddress(const FString &ipAddress)
	{
		ISocketSubsystem* socketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (socketSubsystem == nullptr)
		{
			KBE_ERROR(TEXT("NetworkInterface::GetIPAddress:cant get SocketSubsystem."));
			return ipAddress;
		}

		TSharedPtr<FInternetAddr> remoteAddr = socketSubsystem->CreateInternetAddr();
		FString outIP = ipAddress;
		bool bIsValid = false;
		remoteAddr->SetIp(*outIP, bIsValid);	// 验证是否ip地址
		if (!bIsValid)							// 不是ip地址，作为域名解析
		{
			ESocketErrors hostResolveError = socketSubsystem->GetHostByName(TCHAR_TO_ANSI(*ipAddress), *remoteAddr);
			if (hostResolveError == SE_NO_ERROR || hostResolveError == SE_EWOULDBLOCK)
			{
				outIP = remoteAddr->ToString(false);
				KBE_DEBUG(TEXT("NetworkInterface::GetIPAddress:resolve host ---> %s to %s."), *ipAddress, *outIP);
			}
		}

		return outIP;
	}
}