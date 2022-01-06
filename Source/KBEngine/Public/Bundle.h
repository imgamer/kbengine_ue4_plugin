#pragma once

#include "MemoryStream.h"
#include "Message.h"

namespace KBEngine
{
	class NetworkInterfaceBase;

	class KBENGINE_API Bundle
	{
	public:
		Bundle();
		~Bundle();

		void NewMessage(const Message *mt);

		void Fini(bool issend);

		void WriteMsgLength();

		void Send(NetworkInterfaceBase *networkInterface);

		void CheckStream(int v);

		//---------------------------------------------------------------------------------
		void WriteInt8(int8 v)
		{
			CheckStream(1);
			stream_->WriteInt8(v);
		}

		void WriteInt16(int16 v)
		{
			CheckStream(2);
			stream_->WriteInt16(v);
		}

		void WriteInt32(int32 v)
		{
			CheckStream(4);
			stream_->WriteInt32(v);
		}

		void WriteInt64(int64 v)
		{
			CheckStream(8);
			stream_->WriteInt64(v);
		}

		void WriteUint8(uint8 v)
		{
			CheckStream(1);
			stream_->WriteUint8(v);
		}

		void WriteUint16(uint16 v)
		{
			CheckStream(2);
			stream_->WriteUint16(v);
		}

		void WriteUint32(uint32 v)
		{
			CheckStream(4);
			stream_->WriteUint32(v);
		}

		void WriteUint64(uint64 v)
		{
			CheckStream(8);
			stream_->WriteUint64(v);
		}

		void WriteFloat(float v)
		{
			CheckStream(4);
			stream_->WriteFloat(v);
		}

		void WriteDouble(double v)
		{
			CheckStream(8);
			stream_->WriteDouble(v);
		}

		void WriteStdString(const std::string &v)
		{
			CheckStream(v.length() + 1);
			stream_->WriteStdString(v);
		}

		void WriteString(const FString &v)
		{
			auto s = StringCast<ANSICHAR>(*v);
			CheckStream(s.Length() + 1);
			stream_->Append(s.Get(), s.Length());
			stream_->WriteInt8(0);
		}

		void WriteUTF8(const FString &v)
		{
			FTCHARToUTF8 utf8(*v);
			
			//                 |- 长度  -| |- 数据流 ->
			// BLOB数据流格式：00 00 00 00 xx xx xx xx xx ...
			// 所以需要加4个字节长度
			CheckStream(utf8.Length() + 4);
			stream_->WriteBlob(utf8.Get(), utf8.Length());
		}

		void WriteBlob(const char *buf, uint32 len)
		{
			CheckStream(len + 4);
			stream_->WriteBlob(buf, len);
		}

		void WriteBlob(const TArray<uint8> &bytes)
		{
			CheckStream(bytes.Num() + 4);
			stream_->WriteBlob(bytes);
		}

	protected:
		MemoryStream *stream_;
		TArray<MemoryStream *> streamList_;
		int numMessage_ = 0;
		int messageLength_ = 0;
		const Message *msgType_ = nullptr;
		int curMsgStreamIndex_ = 0;
	};
}

