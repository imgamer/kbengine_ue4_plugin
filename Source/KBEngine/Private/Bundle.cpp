#include "KBEnginePrivatePCH.h"
#include "Bundle.h"
#include "NetworkInterface.h"

namespace KBEngine
{
	Bundle::Bundle()
	{
		stream_ = new MemoryStream();
	}

	Bundle::~Bundle()
	{
		SAFE_DELETE(stream_);
		for (auto s : streamList_)
			delete s;
	}

	void Bundle::NewMessage(const Message *mt)
	{
		KBE_ASSERT(mt);
		Fini(false);

		msgType_ = mt;
		numMessage_ += 1;

		WriteUint16(msgType_->ID());

		if (msgType_->MsgLen() == -1)
		{
			WriteUint16(0);
			messageLength_ = 0;
		}

		curMsgStreamIndex_ = 0;
	}

	void Bundle::Fini(bool issend)
	{
		if (numMessage_ > 0)
		{
			WriteMsgLength();

			streamList_.Add(stream_);
			stream_ = new MemoryStream();
		}

		if (issend)
		{
			numMessage_ = 0;
			msgType_ = nullptr;
		}

		curMsgStreamIndex_ = 0;
	}

	void Bundle::WriteMsgLength()
	{
		if (msgType_->MsgLen() != -1)
			return;

		MemoryStream *mstream = this->stream_;
		if (curMsgStreamIndex_ > 0)
		{
			mstream = streamList_[streamList_.Num() - curMsgStreamIndex_];
		}
		mstream->Data()[2] = (uint8)(messageLength_ & 0xff);
		mstream->Data()[3] = (uint8)(messageLength_ >> 8 & 0xff);
	}

	void Bundle::CheckStream(int v)
	{
		if (v > stream_->Space())
		{
			streamList_.Add(stream_);
			stream_ = new MemoryStream();
			++curMsgStreamIndex_;
		}

		messageLength_ += v;
	}

	void Bundle::Send(NetworkInterface *networkInterface)
	{
		Fini(true);

		if (networkInterface->Valid())
		{
			for (int i = 0; i<streamList_.Num(); i++)
			{
				auto mstream = streamList_[i];
				networkInterface->Send(mstream->Data() + mstream->RPos(), mstream->Length());
			}
		}
		else
		{
			KBE_ERROR(TEXT("Bundle::send: networkInterface invalid!"));
		}

		for (auto s : streamList_) {
			delete s;
		}
		streamList_.Empty(0);
		stream_->Clear();
	}

}
