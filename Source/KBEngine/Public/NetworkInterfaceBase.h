#pragma once
#include <functional>

#include "Networking.h"
#include "MessageReader.h"


namespace KBEngine
{
	class KBEngineApp;
	class PacketReceiverBase;
	class PacketSenderBase;
	class NetworkStatus;

	class KBENGINE_API NetworkInterfaceBase
	{
	public:
		typedef std::function<void(FString, int, bool)> ConnectCallbackFun;

		typedef struct
		{
			// for connect
			FString connectIP = "";
			uint16 connectPort = 0;
			ConnectCallbackFun connectCB;
			FSocket* socket = nullptr;
			NetworkInterfaceBase* networkInterface = nullptr;
			FString error = "";
		}ConnectState;

	public:
		NetworkInterfaceBase(MessageReader* messageReader);
		virtual ~NetworkInterfaceBase();

		static const int TCP_PACKET_MAX = 65535;

		virtual bool InitSocket(uint32 receiveBufferSize = 0, uint32 sendBufferSize = 0) = 0;
		void Reset();

		ISocketSubsystem* SocketSubsystem() { return socketSubsystem_; }
		FSocket* Socket() { return socket_; }

		virtual bool Valid();

		// 参数不能用引用模式，因为在内部会改变状态，删除原来的状态数据
		virtual void OnConnected(ConnectState state);

		void ConnectTo(const FString& ip, uint16 port, ConnectCallbackFun callback);

		virtual void Close();

		bool Send(uint8* datas, int32 length);

		virtual void Process();

		void ChangeNetworkStatus(NetworkStatus* status);

		PacketReceiverBase* GetReceiver() { return packetReceiver_; }

		void ProcessMessage();

		static FString GetIPAddress(const FString &ipAddress);

	public:
		// for internal

		// 用于PacketReceiver和PacketSender发现Socket有错误时，通知关闭使用
		// 此接口将被子线程调用
		void WillClose();

	protected:
		virtual PacketReceiverBase* CreatePacketReceiver() = 0;

		virtual void InitPacketSender() = 0;

		virtual void StartConnect(ConnectState state) = 0;

	protected:
		FSocket* socket_ = nullptr;
		ISocketSubsystem* socketSubsystem_ = nullptr;

		PacketReceiverBase* packetReceiver_ = nullptr;
		PacketSenderBase* packetSender_ = nullptr;

		bool willClose_ = false;

		NetworkStatus* networkStatus_ = nullptr;

		MessageReader* messageReader_ = nullptr;
	};

}