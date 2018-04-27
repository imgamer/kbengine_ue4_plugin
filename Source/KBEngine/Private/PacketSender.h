#pragma once

namespace KBEngine
{
	class NetworkInterface;

	class PacketSender : public FRunnable
	{
	public:
		PacketSender(NetworkInterface* networkInterface, uint32 buffLength = 65535);
		~PacketSender();

		bool Send(uint8* datas, uint32 length);

		void StartBackgroundSend();
		void WillClose() { willClose_ = true; }

	public:
		// for FRunnable
		virtual uint32 Run() override;
		void DoThreadedWork();

	private:
		// 由于有可能阻塞在socket中，所以这个接口可能会导致卡机，外部非测试理由别用
		void StopBackgroundSend();

		uint32 SendSize() const;
		void BackgroundSend();
		FString ReadPipe();
		void WritePipe();
		void InitPipe();
		void ClosePipe();

	private:
		uint8* buffer_;
		uint32	bufferLength_ = 0;

		uint32 wpos_ = 0;				// 写入的数据位置
		uint32 spos_ = 0;				// 发送完毕的数据位置
		bool sending_ = false;

		NetworkInterface* networkInterface_ = NULL;

		FRunnableThread* thread_ = nullptr;
		bool breakThread_ = false;

		void* readPipe_;
		void* writePipe_;

		// 由NetworkInterface关闭网络时通知，
		// 以避免在主动关闭网络时也发出错误信息
		bool willClose_ = false;
	};

}