#include "PacketReceiverTCP.h"
#include "NetworkInterfaceBase.h"

namespace KBEngine 
{

void PacketReceiverTCP::BackgroundRecv() 
{
	// 必须有空间可写，否则我们阻塞在线程中直到有空间为止
	int retries = 0;
	int space = FreeWriteSpace();

	while (space == 0)
	{
		retries += 1;

		if (retries > 10)
		{
			KBE_ERROR(TEXT("PacketReceiverTCP::OnBackgroundRecv(): no space! disconnect to server!"));
			breakThread_ = true;
			if (!willClose_)
				networkInterface_->WillClose();
			return;
		}

		KBE_WARNING(TEXT("PacketReceiverTCP::OnBackgroundRecv(): waiting for space, Please adjust 'TCP_RECV_BUFFER_MAX'! retries = %d"), retries);
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
		KBE_ERROR(TEXT("PacketReceiverTCP::BackgroundRecv: Maybe lose connected from server!"));
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

}	// end namespace KBEngine