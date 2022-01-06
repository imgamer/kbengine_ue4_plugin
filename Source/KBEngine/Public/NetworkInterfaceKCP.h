#pragma once
#include "NetworkInterfaceBase.h"
#include "MessageReader.h"
#include "PacketReceiverBase.h"
#include "ikcp.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"

namespace KBEngine
{

class NetworkInterfaceKCP : public NetworkInterfaceBase
{
public:
	NetworkInterfaceKCP(MessageReader* messageReader)
		: NetworkInterfaceBase(messageReader),
		kcp_(NULL),
		connID_(0),
		nextTickKcpUpdate_(0),
		addr_(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
	{}

	void Process() override;

	void Close() override;

	bool Valid() override;
	
	// 参数不能用引用模式，因为在内部会改变状态，删除原来的状态数据
	void OnConnected(ConnectState state) override;

	int SendTo(const char* buf, int len);

	void NextTickKcpUpdate() {
		nextTickKcpUpdate_ = 0;
	}

	ikcpcb* KCP()
	{
		return kcp_;
	}

protected:
	PacketReceiverBase* CreatePacketReceiver() override;

	bool InitSocket(uint32 receiveBufferSize = 0, uint32 sendBufferSize = 0) override;

	void InitPacketSender() override;

	void StartConnect(ConnectState state) override;

	bool initKCP();
	bool finiKCP();

	static int kcp_output(const char* buf, int len, ikcpcb* kcp, void* user);

	static void kcp_writeLog(const char* log, ikcpcb* kcp, void* user);

private:
	ikcpcb* kcp_;
	uint32 connID_;
	uint32 nextTickKcpUpdate_;

	TSharedRef<FInternetAddr> addr_;
};

}	// end namespace KBEngine
