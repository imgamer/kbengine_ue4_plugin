#pragma once

#include "KBEDefine.h"

namespace KBEngine
{
	typedef uint16 MessageID;
	
	class MemoryStream;
	class KBEDATATYPE_BASE;
	class Messages;
	class MessagesHandler;

	class KBENGINE_API Message
	{
		friend Messages;

	public:
		Message(MessageID msgid, FString msgname, int16 length, int8 argstype, const TArray<uint8> &msgargtypes, FString msghandler);
		~Message();

		inline MessageID ID() const { return id_; }
		inline int16 MsgLen() const { return msgLen_; }

		/*
		从二进制数据流中创建该消息的参数数据
		*/
		void CreateFromStream(MemoryStream *msgstream, TArray<FVariant> &out) const;

		/*
		将一个消息包反序列化后交给消息相关联的函数处理
		例如：KBEngineApp.Client_onRemoteMethodCall
		*/
		void HandleMessage(MemoryStream *msgstream, MessagesHandler* handler) const;

	private:
		MessageID id_ = 0;
		FString name_;
		int16 msgLen_ = -1;
		FString handler_;
		TArray<KBEDATATYPE_BASE *> argTypes_;
		int8 argsType_ = 0;
	};  // end of class Message






	class KBENGINE_API Messages
	{
	public:
		Messages();
		~Messages();

		void Reset();

		const Message* GetMessage(FString name);
		const Message* GetClientMessage(MessageID id);

		bool BaseappMessageImported() { return baseappMessageImported_; }
		void BaseappMessageImported(bool bValue) { baseappMessageImported_ = bValue; }
		bool LoginappMessageImported() { return loginappMessageImported_; }

		bool ImportMessagesFromStream(MemoryStream& stream, SERVER_APP_TYPE fromApp);

	private:
		void BindFixedMessage();
		void AddClientMessage(const Message& msg);
		void AddLoginappMessage(const Message& msg);
		void AddBaseappMessage(const Message& msg);

	private:
		TMap<MessageID, Message> loginappMessages_;
		TMap<MessageID, Message> baseappMessages_;
		TMap<MessageID, Message> clientMessages_;
		TMap<FString, Message> messages_;

		bool baseappMessageImported_ = false;
		bool loginappMessageImported_ = false;

	};  // end of class Messages
}
