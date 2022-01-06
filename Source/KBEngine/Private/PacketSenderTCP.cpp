#include "PacketSenderTCP.h"
#include "Containers/UnrealString.h"
#include "KBEDebug.h"
#include "GenericPlatform/GenericPlatformProcess.h"	// WritePipe

#if PLATFORM_WINDOWS
	#include "WinSock2.h"	// GetLastError(), WSANOTINITIALISED
	#include "fileapi.h"	// ::ReadFile()
#endif	// PLATFORM_WINDOWS

namespace KBEngine
{

PacketSenderTCP::PacketSenderTCP(NetworkInterfaceBase* networkInterface, uint32 buffLength)
	: PacketSenderBase(networkInterface),
	buffer_(new uint8[buffLength]),
	bufferLength_(buffLength)
{
	InitPipe();
	StartBackgroundSend();
}

PacketSenderTCP::~PacketSenderTCP()
{
	StopBackgroundSend();

	if (buffer_)
		delete buffer_;

	ClosePipe();

	KBE_DEBUG(TEXT("PacketSenderTCP::~PacketSenderTCP()"));
}

bool PacketSenderTCP::Send(uint8* datas, uint32 length)
{
	KBE_ASSERT(length > 0);

	uint32 t_spos = spos_;

	//写下标大于等于发送下标，说明还没发完或者刚刚发完，并且还没有回环写，则写完尾部再往头部写
	if (wpos_ >= t_spos)
	{
		uint32 space1 = bufferLength_ - wpos_;
		uint32 space2 = t_spos - 1;		//写下标追上发送下标时保持1个空位，便于判断是发完还是写满
		if (length > space1 + space2)
		{
			KBE_ERROR(TEXT("PacketSenderTCP::Send() : no space, Please adjust 'TCP_SEND_BUFFER_MAX'!data(%d) > space(%d), wpos=%u, spos=%u"), length, space1 + space2, wpos_, t_spos);
			return false;
		}

		if (length <= space1)
		{
			memcpy(&(buffer_[wpos_]), datas, length);
			wpos_ += length;
		}
		else
		{
			if (space1 > 0)
			{
				memcpy(&(buffer_[wpos_]), datas, space1);
			}
			memcpy(&(buffer_[0]), datas + space1, length - space1);
			wpos_ = length - space1;
		}
	}
	//发送下标大于写下标，说明还没发完并且已经回环写，则写发送下标与写下标之间空位
	else
	{
		if (length > t_spos - wpos_ - 1)
		{
			KBE_ERROR(TEXT("PacketSenderTCP::Send() : no space, Please adjust 'TCP_SEND_BUFFER_MAX'!data(%d) > space(%d), wpos=%u, spos=%u"), length, t_spos - wpos_ - 1, wpos_, t_spos);
			return false;
		}

		memcpy(&(buffer_[wpos_]), datas, length);
		wpos_ += length;
	}

	KBE_DEBUG(TEXT("PacketSenderTCP::Send() : data(%d), wpos=%u, spos=%u"), length, wpos_, t_spos);

	WritePipe();

	return true;
}

uint32 PacketSenderTCP::Run()
{
	DoThreadedWork();
	return 0;
}

void PacketSenderTCP::StartBackgroundSend()
{
	KBE_ASSERT(!thread_);
	breakThread_ = false;
	thread_ = FRunnableThread::Create(this, *FString::Printf(TEXT("KBEnginePacketSender:%p"), this));
}

void PacketSenderTCP::DoThreadedWork()
{
	while (true)
	{
		FString data = ReadPipe();
		if (data.Len() == 0)
		{
			KBE_ERROR(TEXT("PacketSenderBase::DoThreadedWork: pipe closed!"));
			break;
		}
		//KBE_DEBUG(TEXT("PacketSenderBase::DoThreadedWork(), read pipe data '%s'"), *data);

		if (breakThread_)
			break;

		if (!networkInterface_ || !networkInterface_->Valid())
			break;
		BackgroundSend();
	}
	sending_ = false;
	breakThread_ = false;
}

uint32 PacketSenderTCP::SendSize() const
{
	uint32 t_wpos = wpos_;
	if (t_wpos >= spos_)
	{
		return t_wpos - spos_;
	}
	return bufferLength_ - spos_ + t_wpos;
}

void PacketSenderTCP::StopBackgroundSend()
{
	if (thread_)
	{
		breakThread_ = true;
		WritePipe(); // 激活阻塞

		// 阻塞等待线程结束
		thread_->WaitForCompletion();
		delete thread_;
		thread_ = nullptr;
	}
}

void PacketSenderTCP::BackgroundSend()
{
	uint32 sendSize = SendSize();
	if (sendSize == 0)
	{
		KBE_WARNING(TEXT("PacketSenderBase::BackgroundSend: require send data, but no data to send!!!"));
		return;
	}

	uint32 tailSize = bufferLength_ - spos_;
	int32 bytesSent = 0;

	if (sendSize <= tailSize)
	{
		RealBackgroundSend(sendSize, bytesSent);
		return;
	}

	uint32 remainSize = sendSize;
	if (tailSize > 0)
	{
		RealBackgroundSend(tailSize, bytesSent);
		if (tailSize != bytesSent)
			return;
		remainSize = sendSize - tailSize;
	}
	spos_ = 0;	//重置发送位置
	RealBackgroundSend(remainSize, bytesSent);
}

void PacketSenderTCP::RealBackgroundSend(uint32 sendSize, int32& bytesSent)
{
	networkInterface_->Socket()->Send(&(buffer_[spos_]), sendSize, bytesSent);

	if (bytesSent == -1)
	{
#if PLATFORM_WINDOWS
		int id = GetLastError();
		FString errStr;

		switch (id)
		{
		case WSANOTINITIALISED: errStr = TEXT("PacketReceiverTCP::BackgroundRecv: not initialized"); break;
		case WSASYSNOTREADY:    errStr = TEXT("PacketReceiverTCP::BackgroundRecv: sub sys not ready"); break;
		case WSAHOST_NOT_FOUND: errStr = TEXT("PacketReceiverTCP::BackgroundRecv: name server not found"); break;
		case WSATRY_AGAIN:      errStr = TEXT("PacketReceiverTCP::BackgroundRecv: server fail"); break;
		case WSANO_RECOVERY:    errStr = TEXT("PacketReceiverTCP::BackgroundRecv: no recovery"); break;
		case WSAEINPROGRESS:    errStr = TEXT("PacketReceiverTCP::BackgroundRecv: socket blocked by other prog"); break;
		case WSANO_DATA:        errStr = TEXT("PacketReceiverTCP::BackgroundRecv: no data record"); break;
		case WSAEINTR:          errStr = TEXT("PacketReceiverTCP::BackgroundRecv: blocking call canciled"); break;
		case WSAEPROCLIM:       errStr = TEXT("PacketReceiverTCP::BackgroundRecv: limit exceeded"); break;
		case WSAEFAULT:         errStr = TEXT("PacketReceiverTCP::BackgroundRecv: lpWSAData in startup not valid."); break;
		case WSAECONNABORTED:   errStr = TEXT("PacketReceiverTCP::BackgroundRecv: connect aborted!"); break;
		default:				errStr = FString::Printf(TEXT("PacketReceiverTCP::OnBackgroundRecv: unknown error id: %d"), id); break;
		};
		if (!willClose_)
			KBE_ERROR(TEXT("%s"), *errStr);
#endif				

		breakThread_ = true;
		if (!willClose_)
			networkInterface_->WillClose();
		return;
	}

	if (bytesSent > 0)
	{
		spos_ += bytesSent;
	}
}


void PacketSenderTCP::InitPipe()
{
	FPlatformProcess::CreatePipe(readPipe_, writePipe_);
}

void PacketSenderTCP::ClosePipe()
{
	if (readPipe_ || writePipe_)
	{
		FPlatformProcess::ClosePipe(readPipe_, writePipe_);
		readPipe_ = writePipe_ = nullptr;
	}
}

FString PacketSenderTCP::ReadPipe()
{
	FString Output;

#if PLATFORM_WINDOWS
	UTF8CHAR Buffer[255];
	uint32 BytesRead = 0;
	if (::ReadFile(readPipe_, Buffer, 254, (::DWORD*)&BytesRead, NULL))
	{
		if (BytesRead > 0)
		{
			Buffer[BytesRead] = '\0';
			Output += FUTF8ToTCHAR((const ANSICHAR*)Buffer).Get();
		}
	}
#else
	KBE_ASSERT(false);
#endif
	return Output;
}

void PacketSenderTCP::WritePipe()
{
	FPlatformProcess::WritePipe(writePipe_, TEXT("1"));
}

}	// end namespace KBEngine