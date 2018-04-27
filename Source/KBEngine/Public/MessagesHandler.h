#pragma once

#include "Core.h"
#include "MemoryStream.h"

namespace KBEngine
{
	class KBENGINE_API MessagesHandler
	{
	public:
		virtual void HandleMessage(const FString &name, MemoryStream *stream) = 0;
		virtual void HandleMessage(const FString &name, const TArray<FVariant> &args) = 0;
	};
}
