// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

#include "Entity.h"
#include "EntityDef.h"
#include "Event.h"
#include "KBEMain.h"
#include "KBEMath.h"
#include "KBEngineApp.h"
#include "KBEngineArgs.h"
#include "KBEDefine.h"
#include "MemoryStream.h"
#include "NetworkInterface.h"
#include "KBEPersonality.h"
#include "Property.h"
#include "ScriptModule.h"
#include "EntityDeclareDefine.h"


class FKBEngineModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

