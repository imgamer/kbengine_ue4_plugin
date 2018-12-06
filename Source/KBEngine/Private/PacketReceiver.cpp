#include "PacketReceiver.h"
#include "KBEnginePrivatePCH.h"
#include "Core.h"
#include "NetworkInterface.h"
#include "MessageReader.h"
#include "AllowWindowsPlatformTypes.h"
#include "HideWindowsPlatformTypes.h"

namespace KBEngine
{
	PacketReceiver::PacketReceiver(NetworkInterface* networkInterface, uint32 buffLength)
	{
		networkInterface_ = networkInterface;
		buffer_ = new uint8[buffLength];
		bufferLength_ = buffLength;
	}

	PacketReceiver::~PacketReceiver()
	{
		KBE_DEBUG(TEXT("PacketReceiver::~PacketReceiver()"));
		StopBackgroundRecv();

		if (buffer_)
			delete buffer_;
	}

	void PacketReceiver::Process(MessageReader& messageReader)
	{
		// @TODO(penghuawei): 将来要考虑直接在子线程就把数据写入到MessageReader中，以节省一次内存复制的消耗

		uint32 t_wpos = wpos_;
		uint32 r = 0;

		if (rpos_ < t_wpos)
		{
			messageReader.Write(&buffer_[rpos_], t_wpos - rpos_);
			rpos_ = t_wpos;
		}
		else if (t_wpos < rpos_)
		{
			messageReader.Write(&buffer_[rpos_], bufferLength_ - rpos_);
			if (t_wpos > 0)
				messageReader.Write(&buffer_[0], t_wpos);
			rpos_ = t_wpos;
		}
		else
		{
			// 没有可读数据
		}
	}

	uint32 PacketReceiver::FreeWriteSpace()
	{
		uint32 t_rpos = rpos_;

		if (wpos_ == bufferLength_)
		{
			if (t_rpos == 0)
			{
				return 0;
			}

			wpos_ = 0;
		}

		if (t_rpos <= wpos_)
		{
			return bufferLength_ - wpos_;
		}

		return t_rpos - wpos_ - 1;
	}

	void PacketReceiver::StartBackgroundRecv()
	{
		KBE_ASSERT(!thread_);
		thread_ = FRunnableThread::Create(this, *FString::Printf(TEXT("KBEnginePacketReceiver:%p"), this));
	}

	void PacketReceiver::StopBackgroundRecv()
	{
		if (thread_)
		{
			breakThread_ = true;
			// 阻塞等待线程结束
			thread_->WaitForCompletion();
			delete thread_;
			thread_ = nullptr;
		}
	}
	
	uint32 PacketReceiver::Run()
	{
		DoThreadedWork();
		return 0;
	}

	void PacketReceiver::DoThreadedWork()
	{
		while (!breakThread_)
		{
			BackgroundRecv();
		}
	}

	void PacketReceiver::BackgroundRecv()
	{
		// 必须有空间可写，否则我们阻塞在线程中直到有空间为止
		int retries = 0;
		int space = FreeWriteSpace();

		while (space == 0)
		{
			retries += 1;

			if (retries > 10)
			{
				KBE_ERROR(TEXT("PacketReceiver::OnBackgroundRecv(): no space! disconnect to server!"));
				breakThread_ = true;
				if (!willClose_)
					networkInterface_->WillClose();
				return;
			}

			KBE_WARNING(TEXT("PacketReceiver::OnBackgroundRecv(): waiting for space, Please adjust 'RECV_BUFFER_MAX'! retries = %d"), retries);
			FPlatformProcess::Sleep(0.1);

			space = FreeWriteSpace();
		}

		int32 bytesRead = 0;
		if (!networkInterface_ || !networkInterface_->Valid())
		{
			breakThread_ = true;
			return;
		}

		// wsf:注意，UE4 SocketsBSD Recv限制bytesRead不会小于0，用户无法得到网络错误原因
		bool recvSuccess = networkInterface_->Socket()->Recv(&(buffer_[wpos_]), space, bytesRead);
		if (!recvSuccess)
		{
			KBE_ERROR(TEXT("PacketReceiver::BackgroundRecv: Maybe lose connected from server!"));
			breakThread_ = true;
			if (!willClose_)
				networkInterface_->WillClose();
			return;
		}

		if (bytesRead > 0)
		{
			// 更新写位置
			wpos_ += bytesRead;
		}
	}
}