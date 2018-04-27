#pragma once
#include "KBEnginePrivatePCH.h"
#include "MessageReader.h"

namespace KBEngine
{
	class MessageReader;
	class NetworkInterface;

	class PacketReceiver : public FRunnable
	{

	public:
		PacketReceiver(NetworkInterface* networkInterface, uint32 buffLength = 65535);
		~PacketReceiver();

		void Process(MessageReader& messageReader);
		void StartBackgroundRecv();
		void WillClose() { willClose_ = true; }

	public:
		// for FRunnable
		virtual uint32 Run() override;
		void DoThreadedWork();


	private:
		// 由于阻塞在socket中，所以这个接口可能会导致卡机，外部非测试理由别用
		void StopBackgroundRecv();

		// 子线程中调用：计算可写的缓冲区空间
		uint32 FreeWriteSpace();

		// 子线程中调用：开始从Socket中读取数据
		void BackgroundRecv();

	private:
		NetworkInterface* networkInterface_ = NULL;

		uint8* buffer_;
		uint32 bufferLength_ = 0;

		// socket向缓冲区写的起始位置
		uint32 wpos_ = 0;

		// 主线程读取数据的起始位置
		uint32 rpos_ = 0;

		FRunnableThread* thread_ = nullptr;
		bool breakThread_ = false;

		// 由NetworkInterface关闭网络时通知，
		// 以避免在主动关闭网络时也发出错误信息
		bool willClose_ = false;
	};

}