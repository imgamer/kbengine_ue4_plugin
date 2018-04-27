#include "KBEnginePrivatePCH.h"
#include "Message.h"
#include "EntityDef.h"
#include "KBEngineApp.h"
#include "MessagesHandler.h"

namespace KBEngine
{
	// for Message ------------------------------------------------------------------------------------

	Message::Message(MessageID msgid, FString msgname, int16 length, int8 argstype, const TArray<uint8> &msgargtypes, FString msghandler)
		: id_(msgid),
		name_(msgname),
		msgLen_(length),
		handler_(msghandler),
		argsType_(argstype)
	{
		// 对该消息的所有参数绑定反序列化方法，改方法能够将二进制流转化为参数需要的值
		// 在服务端下发消息数据时会用到
		argTypes_.SetNumUninitialized(msgargtypes.Num());
		for (int i = 0; i<msgargtypes.Num(); i++)
		{
			argTypes_[i] = EntityDef::GetDataType((uint16)msgargtypes[i]);
			if (!argTypes_[i])
			{
				KBE_ERROR(TEXT("Message::Message(): message(%d:%s) arg(%d) type(%d) is not found!"), msgid, *msgname, i, msgargtypes[i]);
			}
		}

		KBE_DEBUG(TEXT("Message::Message(): (%s/%d/%d)!"), *msgname, msgid, msgLen_);
	}

	Message::~Message()
	{
	}

	void Message::CreateFromStream(MemoryStream *msgstream, TArray<FVariant> &out) const
	{
		if (argTypes_.Num() <= 0)
		{
			out.Add(FVariant(msgstream));
			return;
		}

		out.SetNum(argTypes_.Num());

		for (int i = 0; i<argTypes_.Num(); i++)
		{
			out[i] = argTypes_[i]->CreateFromStream(msgstream);
		}

		return;
	}

	void Message::HandleMessage(MemoryStream *msgstream, MessagesHandler* handler) const
	{
		KBE_ASSERT(handler);
		KBE_ASSERT(handler_.Len() > 0);

		if (argTypes_.Num() <= 0)
		{
			if (argsType_ < 0)
			{
				handler->HandleMessage(handler_, msgstream);
			}
			else
			{
				MemoryStream stream;
				handler->HandleMessage(handler_, &stream);
			}
		}
		else
		{
			TArray<FVariant> params;
			CreateFromStream(msgstream, params);
			handler->HandleMessage(handler_, params);
		}
	}


	// for Messages ------------------------------------------------------------------------------------
	Messages::Messages()
	{
		BindFixedMessage();
	}

	Messages::~Messages()
	{
		KBE_DEBUG(TEXT("Messages::~Messages()"));
	}

	void Messages::Reset()
	{
		messages_.Empty();
		loginappMessages_.Empty();
		baseappMessages_.Empty();
		clientMessages_.Empty();

		BindFixedMessage();
	}

	void Messages::BindFixedMessage()
	{
		// 引擎协议说明参见: http://www.kbengine.org/cn/docs/programming/clientsdkprogramming.html
		messages_.Add("Loginapp_importClientMessages", Message(5, "importClientMessages", 0, 0, TArray<uint8>(), ""));
		messages_.Add("Loginapp_hello", Message(4, "hello", -1, -1, TArray<uint8>(), ""));

		messages_.Add("Baseapp_importClientMessages", Message(207, "importClientMessages", 0, 0, TArray<uint8>(), ""));
		messages_.Add("Baseapp_importClientEntityDef", Message(208, "importClientMessages", 0, 0, TArray<uint8>(), ""));
		messages_.Add("Baseapp_hello", Message(200, "hello", -1, -1, TArray<uint8>(), ""));

		messages_.Add("Client_onHelloCB", Message(521, "Client_onHelloCB", -1, -1, TArray<uint8>(), "Client_onHelloCB"));
		clientMessages_.Add(messages_["Client_onHelloCB"].ID(), messages_["Client_onHelloCB"]);

		messages_.Add("Client_onScriptVersionNotMatch", Message(522, "Client_onScriptVersionNotMatch", -1, -1, TArray<uint8>(), "Client_onScriptVersionNotMatch"));
		clientMessages_.Add(messages_["Client_onScriptVersionNotMatch"].ID(), messages_["Client_onScriptVersionNotMatch"]);

		messages_.Add("Client_onVersionNotMatch", Message(523, "Client_onVersionNotMatch", -1, -1, TArray<uint8>(), "Client_onVersionNotMatch"));
		clientMessages_.Add(messages_["Client_onVersionNotMatch"].ID(), messages_["Client_onVersionNotMatch"]);

		messages_.Add("Client_onImportClientMessages", Message(518, "Client_onImportClientMessages", -1, -1, TArray<uint8>(), "Client_onImportClientMessages"));
		clientMessages_.Add(messages_["Client_onImportClientMessages"].ID(), messages_["Client_onImportClientMessages"]);
	}


	const Message* Messages::GetMessage(FString name)
	{
		return messages_.Find(name);
	}

	const Message* Messages::GetClientMessage(MessageID id)
	{
		return clientMessages_.Find(id);
	}

	void Messages::AddClientMessage(const Message& msg)
	{
		if (msg.name_.Len() > 0)
			messages_.Add(msg.name_, msg);
		clientMessages_.Add(msg.id_, msg);
	}

	void Messages::AddLoginappMessage(const Message& msg)
	{
		if (msg.name_.Len() > 0)
			messages_.Add(msg.name_, msg);
		loginappMessages_.Add(msg.id_, msg);
	}

	void Messages::AddBaseappMessage(const Message& msg)
	{
		if (msg.name_.Len() > 0)
			messages_.Add(msg.name_, msg);
		baseappMessages_.Add(msg.id_, msg);
	}

	bool Messages::ImportMessagesFromStream(MemoryStream& stream, SERVER_APP_TYPE fromApp)
	{
		// @TODO(penghuawei): 这里当前没有对数据流的有效性进行较验，
		// 所以如果数据流来自本地且非法，就将有可能导致客户端出现未知的问题


		uint16 msgcount = stream.ReadUint16();

		KBE_DEBUG(TEXT("Messages::ImportClientMessagesFromStream: msgsize=%d..."), msgcount);

		while (msgcount > 0)
		{
			msgcount--;

			MessageID msgid = stream.ReadUint16();
			int16 msglen = stream.ReadInt16();

			FString msgname = stream.ReadString();
			int8 argstype = stream.ReadInt8();
			uint8 argsize = stream.ReadUint8();
			TArray<uint8> argstypes;

			for (uint8 i = 0; i<argsize; i++)
			{
				argstypes.Add(stream.ReadUint8());
			}

			bool isClientMethod = msgname.Contains("Client_");

			//System.Reflection.MethodInfo handler = null;

			//if (isClientMethod)
			//{
			//	handler = typeof(KBEngineApp).GetMethod(msgname);
			//	if (handler == null)
			//	{
			//		Dbg.WARNING_MSG(string.Format("Messages::ImportClientMessagesFromStream[{0}]: interface({1}/{2}/{3}) no implement!",
			//			currserver, msgname, msgid, msglen));

			//		handler = null;
			//	}
			//	else
			//	{
			//		//Dbg.DEBUG_MSG(string.Format("Messages::ImportClientMessagesFromStream: imported({0}/{1}/{2}) successfully!", 
			//		//	msgname, msgid, msglen));
			//	}
			//}

			Message msg(msgid, msgname, msglen, argstype, argstypes, msgname);

			if (isClientMethod)
			{
				AddClientMessage(msg);
			}
			else
			{
				//KBE_DEBUG(TEXT("Messages::ImportClientMessagesFromStream[%s]: imported(%s/%d/%d) successfully!"), *currserver, *msgname, msgid, msglen);
				if (fromApp == SERVER_APP_TYPE::LoginApp)
				{
					AddLoginappMessage(msg);
				}
				else
				{
					AddBaseappMessage(msg);
				}
			}

			if (fromApp == SERVER_APP_TYPE::LoginApp)
				loginappMessageImported_ = true;
			else
				baseappMessageImported_ = true;
		}
		return true;
	}
}
