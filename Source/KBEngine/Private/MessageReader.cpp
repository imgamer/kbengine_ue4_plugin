#include "MessageReader.h"
#include "KBEnginePrivatePCH.h"
#include "MessagesHandler.h"

namespace KBEngine
{
	MessageReader::MessageReader(MessagesHandler* handler, Messages *messages, uint32 bufferLength) :
		messagesHandler_(handler),
		messages_(messages),
		bufferLength_(bufferLength)
	{
		KBE_ASSERT(messages_);
		KBE_ASSERT(messagesHandler_);
		buffer_ = new uint8[bufferLength];
	}


	MessageReader::~MessageReader()
	{
		KBE_DEBUG(TEXT("MessageReader::~MessageReader()"));
		if (buffer_)
		{
			delete buffer_;
			buffer_ = nullptr;
		}
	}

	void MessageReader::Reset()
	{
		stream.Clear();
		rpos_ = 0;
		wpos_ = 0;
	}

	uint32 MessageReader::FreeSpace()
	{
		auto t_rpos = rpos_ % bufferLength_;
		auto t_wpos = wpos_ % bufferLength_;
		uint32 space = 0;

		if (t_wpos >= t_rpos)
			space = bufferLength_ - t_wpos;
		else
			// 减1是因为循环的时候写入的位置不能与读取的位置一致
			space = t_rpos - t_wpos - 1;
		return space;
	}

	uint32 MessageReader::Write(const uint8* datas, MessageLengthEx length)
	{
		KBE_ASSERT(length > 0);
		auto t_length = length;

		while (true)
		{
			auto space = FreeSpace();
			if (space == 0)
				break;

			if (space >= t_length)
			{
				memcpy(&buffer_[wpos_], &datas[length - t_length], t_length);
				wpos_ = (wpos_ + t_length) % bufferLength_;
				t_length = 0;
				break;
			}
			else
			{
				memcpy(&buffer_[wpos_], &datas[length - t_length], space);
				wpos_ = (wpos_ + space) % bufferLength_;
				t_length -= space;
			}
		}
		if (length - t_length == 0)
			KBE_ERROR(TEXT("MessageReader::Write: no space to write! rpos(%u), wpos(%u), message length(%u)"), rpos_, wpos_, length);
		return length - t_length;
	}

	void MessageReader::Process()
	{
		KBE_ASSERT(rpos_ <= bufferLength_ && wpos_ <= bufferLength_);
		while (true)
		{
			auto t_rpos = rpos_ % bufferLength_;
			auto t_wpos = wpos_ % bufferLength_;

			if (t_rpos == t_wpos)
				return;

			uint32 length = 0;

			if (t_wpos > t_rpos)
				length = t_wpos - t_rpos;
			else
				length = bufferLength_ - t_rpos;

			Process_(&buffer_[t_rpos], length);
			rpos_ = (rpos_ + length) % bufferLength_;
		}
	}

	void MessageReader::Process_(const uint8* datas, MessageLengthEx length)
	{
		MessageLengthEx totallen = 0;

		while (length > 0 && expectSize > 0)
		{
			if (state == READ_STATE::READ_STATE_MSGID)
			{
				if (length >= expectSize)
				{
					stream.Append(&(datas[totallen]), expectSize);
					totallen += expectSize;
					length -= expectSize;
					msgid = stream.ReadUint16();
					stream.Clear();

					auto* msg = messages_->GetClientMessage(msgid);
					if (!msg)
					{
						KBE_ERROR(TEXT("MessageReader::Process_: unknown message(%d)!"), msgid);
						KBE_ASSERT(msg);
					}

					if (msg->MsgLen() == -1)
					{
						state = READ_STATE::READ_STATE_MSGLEN;
						expectSize = 2;
					}
					else if (msg->MsgLen() == 0)
					{
						// 如果是0个参数的消息，那么没有后续内容可读了，处理本条消息并且直接跳到下一条消息
						msg->HandleMessage(&stream, messagesHandler_);
						state = READ_STATE::READ_STATE_MSGID;
						expectSize = 2;
					}
					else
					{
						expectSize = msg->MsgLen();
						state = READ_STATE::READ_STATE_BODY;
					}
				}
				else
				{
					stream.Append(&(datas[totallen]), length);
					expectSize -= length;
					break;
				}
			}
			else if (state == READ_STATE::READ_STATE_MSGLEN)
			{
				if (length >= expectSize)
				{
					stream.Append(&(datas[totallen]), expectSize);
					totallen += expectSize;
					length -= expectSize;

					msglen = stream.ReadUint16();
					stream.Clear();

					// 长度扩展
					if (msglen >= 65535)
					{
						state = READ_STATE::READ_STATE_MSGLEN_EX;
						expectSize = 4;
					}
					else
					{
						state = READ_STATE::READ_STATE_BODY;
						expectSize = msglen;
					}
				}
				else
				{
					stream.Append(&(datas[totallen]), length);
					expectSize -= length;
					break;
				}
			}
			else if (state == READ_STATE::READ_STATE_MSGLEN_EX)
			{
				if (length >= expectSize)
				{
					stream.Append(&(datas[totallen]), expectSize);
					totallen += expectSize;
					length -= expectSize;

					expectSize = stream.ReadUint32();
					stream.Clear();

					state = READ_STATE::READ_STATE_BODY;
				}
				else
				{
					stream.Append(&(datas[totallen]), length);
					expectSize -= length;
					break;
				}
			}
			else if (state == READ_STATE::READ_STATE_BODY)
			{
				if (length >= expectSize)
				{
					stream.Append(&(datas[totallen]), expectSize);
					totallen += expectSize;
					length -= expectSize;

					auto* msg = messages_->GetClientMessage(msgid);
					if (!msg)
					{
						KBE_ERROR(TEXT("MessageReader::Process_: unknown message(%d)!"), msgid);
						KBE_ASSERT(msg);
					}
					
					msg->HandleMessage(&stream, messagesHandler_);

					stream.Clear();

					state = READ_STATE::READ_STATE_MSGID;
					expectSize = 2;
				}
				else
				{
					stream.Append(&(datas[totallen]), length);
					expectSize -= length;
					break;
				}
			}
		}
	}

	void MessageReader::ProcessData(const uint8* datas, MessageLengthEx length)
	{
		Process_(datas, length);
	}
}
