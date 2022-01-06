#pragma once

#include "Bundle.h"

namespace KBEngine
{
	class NetworkInterfaceBase;

	class KBENGINE_API Mailbox
	{
	public:
		enum class MAILBOX_TYPE : int8
		{
			MAILBOX_TYPE_CELL = 0,		// CELL_MAILBOX
			MAILBOX_TYPE_BASE = 1		// BASE_MAILBOX
		};

		Mailbox(int32 entityID, const FString& entityType, MAILBOX_TYPE mbType);
		~Mailbox();

		inline bool IsBase() const
		{
			return type_ == MAILBOX_TYPE::MAILBOX_TYPE_BASE;
		}

		inline bool IsCell() const
		{
			return type_ == MAILBOX_TYPE::MAILBOX_TYPE_CELL;
		}

		/*
		创建新的mail
		*/
		Bundle *NewMail();

		/*
		向服务端发送这个mail
		*/
		void PostMail(NetworkInterfaceBase *networkInterface);

		int32 ID() { return id_; }
		const FString& ClassName() { return className_; }
		MAILBOX_TYPE Type() { return type_; }

	private:
		int32 id_ = 0;
		FString className_;
		MAILBOX_TYPE type_ = MAILBOX_TYPE::MAILBOX_TYPE_CELL;
		Bundle *bundle_ = nullptr;
	};
}
