#pragma once
#include "PacketSenderBase.h"
#include "NetworkInterfaceBase.h"
#include "HAL/RunnableThread.h"

namespace KBEngine
{

class PacketSenderTCP : public PacketSenderBase, FRunnable
{
public:
	PacketSenderTCP(NetworkInterfaceBase* networkInterface, uint32 buffLength = 65535);

	~PacketSenderTCP();

	bool Send(uint8* datas, uint32 length) override;

	void StartBackgroundSend();

	uint32 Run() override;

protected:
	void BackgroundSend();

	void DoThreadedWork();

	void RealBackgroundSend(uint32 sendSize, int32& bytesSent);

	uint32 SendSize() const;

	// 由于有可能阻塞在socket中，所以这个接口可能会导致卡机，外部非测试理由别用
	void StopBackgroundSend();

	void InitPipe();
	void ClosePipe();
	FString ReadPipe();
	void WritePipe();

protected:
	uint8* buffer_;
	uint32	bufferLength_ = 0;

	uint32 wpos_ = 0;				// 写入的数据位置
	uint32 spos_ = 0;				// 发送完毕的数据位置
	bool sending_ = false;

	FRunnableThread* thread_ = nullptr;
	bool breakThread_ = false;

	void* readPipe_;
	void* writePipe_;

};

}	// end namespace KBEngine
