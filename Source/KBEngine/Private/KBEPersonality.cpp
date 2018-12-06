#include "KBEPersonality.h"
#include "KBEDebug.h"
#include "KBEnginePrivatePCH.h"


namespace KBEngine
{
	KBEPersonality* KBEPersonality::instance_ = nullptr;

	void KBEPersonality::Register(KBEPersonality * inst)
	{
		KBE_ASSERT(instance_ == nullptr);
		instance_ = inst;
	}

	void KBEPersonality::Deregister()
	{
		if (instance_)
			instance_ = nullptr;
	}
}
