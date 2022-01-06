#include "Mailbox.h"
#include "KBEnginePrivatePCH.h"
#include "KBEngineApp.h"

namespace KBEngine
{

	Mailbox::Mailbox(int32 entityID, const FString& entityType, MAILBOX_TYPE mbType) :
		id_(entityID),
		className_(entityType),
		type_(mbType)
	{
	}

	Mailbox::~Mailbox()
	{
		SAFE_DELETE(bundle_);
	}

	Bundle *Mailbox::NewMail()
	{
		if (bundle_ == NULL)
			bundle_ = new Bundle();

		if (type_ == MAILBOX_TYPE::MAILBOX_TYPE_CELL)
			bundle_->NewMessage(KBEngineApp::app->pBaseApp()->pMessages()->GetMessage("Baseapp_onRemoteCallCellMethodFromClient"));
		else
			bundle_->NewMessage(KBEngineApp::app->pBaseApp()->pMessages()->GetMessage("Entity_onRemoteMethodCall"));

		bundle_->WriteInt32(id_);

		return bundle_;
	}

	void Mailbox::PostMail(NetworkInterfaceBase *networkInterface)
	{
		KBE_ASSERT(bundle_);

		bundle_->Send(networkInterface);

		SAFE_DELETE(bundle_);
	}

}
