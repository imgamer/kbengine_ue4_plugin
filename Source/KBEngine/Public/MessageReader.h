#pragma once
#include <utility>

#include "Memorystream.h"
#include "Message.h"

namespace KBEngine
{
#define MessageID uint16
#define MessageLength uint16
#define MessageLengthEx uint32

	class MessagesHandler;

	class KBENGINE_API MessageReader
	{
	public:
		MessageReader(MessagesHandler* handler, Messages *messages, uint32 bufferLength = 65535);
		~MessageReader();

		enum READ_STATE
		{
			// 消息ID
			READ_STATE_MSGID = 0,

			// 消息的长度65535以内
			READ_STATE_MSGLEN = 1,

			// 当上面的消息长度都无法到达要求时使用扩展长度
			// uint32
			READ_STATE_MSGLEN_EX = 2,

			// 消息的内容
			READ_STATE_BODY = 3
		};

		// 用于主线程处理数据
		void Process();
		
		// 用于子线程写入数据
		uint32 Write(const uint8* datas, MessageLengthEx length);

		void Reset();

	private:
		uint32 FreeSpace();
		void Process_(const uint8* datas, MessageLengthEx length);

	private:
		MessagesHandler* messagesHandler_ = nullptr;
		Messages *messages_ = nullptr;

		uint32 rpos_ = 0;
		uint32 wpos_ = 0;
		uint8* buffer_ = nullptr;
		uint32 bufferLength_ = 0;

		MessageID msgid = 0;
		MessageLength msglen = 0;
		MessageLengthEx expectSize = 2;
		READ_STATE state = READ_STATE::READ_STATE_MSGID;
		MemoryStream stream;

	};

}
