#include "PacketReceiverBase.h"
#include "KBEnginePrivatePCH.h"
#include "Core.h"
#include "NetworkInterfaceBase.h"
#include "MessageReader.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"

namespace KBEngine
{
	PacketReceiverBase::PacketReceiverBase(NetworkInterfaceBase* networkInterface, uint32 buffLength)
	{
		networkInterface_ = networkInterface;
		buffer_ = new uint8[buffLength];
		bufferLength_ = buffLength;
	}

	PacketReceiverBase::~PacketReceiverBase()
	{
		KBE_DEBUG(TEXT("PacketReceiverBase::~PacketReceiverBase()"));
		StopBackgroundRecv();

		if (buffer_)
			delete buffer_;
	}

	void PacketReceiverBase::Process(MessageReader& messageReader)
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

	uint32 PacketReceiverBase::FreeWriteSpace()
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

	void PacketReceiverBase::StartBackgroundRecv()
	{
		KBE_ASSERT(!thread_);
		thread_ = FRunnableThread::Create(this, *FString::Printf(TEXT("KBEnginePacketReceiver:%p"), this));
	}

	void PacketReceiverBase::StopBackgroundRecv()
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
	
	uint32 PacketReceiverBase::Run()
	{
		DoThreadedWork();
		return 0;
	}

	void PacketReceiverBase::DoThreadedWork()
	{
		while (!breakThread_)
		{
			BackgroundRecv();
		}
	}
}