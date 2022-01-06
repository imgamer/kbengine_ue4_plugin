#pragma once

#include "KBEDebug.h"

namespace KBEngine
{
	class NetworkInterfaceBase;

	class PacketSenderBase
	{
	public:
		PacketSenderBase(NetworkInterfaceBase* networkInterface);
		virtual ~PacketSenderBase();

		virtual bool Send(uint8* datas, uint32 length) = 0;

		void WillClose() { willClose_ = true; }
		
	protected:
		NetworkInterfaceBase* networkInterface_ = NULL;

		// 由NetworkInterface关闭网络时通知，
		// 以避免在主动关闭网络时也发出错误信息
		bool willClose_ = false;
	};

}	// end namespace KBEngine
