#include "KBEnginePrivatePCH.h"
#include "NetworkInterface.h"
#include "KBEDebug.h"
#include "KBEPersonality.h"
#include "PacketReceiver.h"
#include "PacketSender.h"
#include "KBEngineApp.h"

#if PLATFORM_WINDOWS
//#include "WinSock2.h"
#endif

namespace KBEngine
{
	class Status_Connected : public NetworkInterface::NetworkStatus
	{
	public:
		virtual void MainThreadProcess(NetworkInterface *networkInterface) override
		{
			if (networkInterface->Valid())
				networkInterface->ProcessMessage();
		}
	};

	class Status_Connecting : public NetworkInterface::NetworkStatus, public FRunnable
	{
	public:
		Status_Connecting(NetworkInterface::ConnectState& opt) :
			opt_(opt)
		{
			thread_ = FRunnableThread::Create(this, *FString::Printf(TEXT("KBEngineNetworkInterfaceStatus_Connecting:%p"), this));
		}

		virtual ~Status_Connecting()
		{
			if (thread_)
			{
				thread_->WaitForCompletion();
				delete thread_;
				thread_ = nullptr;
			}
		}

		// call by sub-trhead
		virtual uint32 Run() override
		{
			auto Addr = opt_.networkInterface->SocketSubsystem()->CreateInternetAddr(0, opt_.connectPort);
			bool bIsValid;

			Addr->SetIp(*opt_.connectIP, bIsValid);
			if (opt_.socket->Connect(*Addr) == false)
			{
				opt_.error = TEXT("Connect Error");
			}

			// 无论连接成功与否，都标识已进行过连接
			connected_ = true;
			return 0;
		}

		virtual void MainThreadProcess(NetworkInterface *networkInterface) override
		{
			if (connected_)
				networkInterface->OnConnected(opt_);
		}

	private:
		NetworkInterface::ConnectState opt_;
		FRunnableThread* thread_ = nullptr;
		bool connected_ = false;
	};

	NetworkInterface::NetworkInterface(MessageReader* messageReader)
	{
		messageReader_ = messageReader;
		KBE_ASSERT(messageReader);
	}


	NetworkInterface::~NetworkInterface()
	{
		KBE_DEBUG(TEXT("NetworkInterface::~NetworkInterface()"));
		Reset();
	}

	void NetworkInterface::Reset()
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

	bool NetworkInterface::InitSocket(uint32 receiveBufferSize, uint32 sendBufferSize)
	{
		socketSubsystem_ = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		if (socketSubsystem_ != nullptr)
		{
			FString descript = TEXT("KBEngine");
			socket_ = socketSubsystem_->CreateSocket(NAME_Stream, descript, true);

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

	bool NetworkInterface::Valid()
	{
		if (willClose_)
			return false;

		return ((socket_ != nullptr) && (socket_->GetConnectionState() == SCS_Connected));
	}

	void NetworkInterface::ChangeNetworkStatus(NetworkStatus* status)
	{
		if (networkStatus_)
			delete networkStatus_;
		networkStatus_ = status;
	}

	void NetworkInterface::OnConnected(ConnectState state)
	{
		bool success = (state.error == "" && Valid());
		if (success)
		{
			KBE_DEBUG(TEXT("NetworkInterface::OnConnected(), connect to '%s:%d' is success!"), *state.connectIP, state.connectPort);
			packetReceiver_ = new PacketReceiver(this, KBEngineApp::app->GetRecvBufferMax());
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
		
	void NetworkInterface::ConnectTo(const FString& host, uint16 port, ConnectCallbackFun callback)
	{
		KBE_ASSERT(!Valid());
		KBE_ASSERT(!networkStatus_);

		if (!InitSocket())
		{
			KBE_ERROR(TEXT("NetworkInterface::ConnectTo: init socket falut."));
		}

		ConnectState state;
		state.connectIP = host;
		state.connectPort = port;
		state.connectCB = callback;
		state.socket = socket_;
		state.networkInterface = this;

		networkStatus_ = new Status_Connecting(state);
	}

	void NetworkInterface::WillClose()
	{
		willClose_ = true;
	}

	void NetworkInterface::Close()
	{
		auto wc = willClose_;
		
		Reset();

		if (wc && KBEngineApp::app)
			KBEngineApp::app->OnLoseConnect();
	}

	bool NetworkInterface::Send(uint8* datas, int32 length)
	{
		if (!Valid())
		{
			//FError::Throwf(TEXT("invalid socket!"));
			KBE_ERROR(TEXT("NetworkInterface::Send: invalid socket!"));
		}

		if (!packetSender_)
		{
			packetSender_ = new PacketSender(this);
			packetSender_->StartBackgroundSend();
		}

		return packetSender_->Send(datas, length);
	}

	void NetworkInterface::Process()
	{
		if (willClose_)
		{
			Close();
			return;
		}

		if (networkStatus_)
			networkStatus_->MainThreadProcess(this);
	}

	void NetworkInterface::ProcessMessage()
	{
		if (packetReceiver_ && messageReader_)
		{
			packetReceiver_->Process(*messageReader_);
		}
	}

}