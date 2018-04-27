#pragma once

//#include "ScriptModule.h"

namespace KBEngine
{

#define ENTITY_DECLARE(DEFNAME, CLASS)                                                                      \
	class __kbe_module_##DEFNAME : public KBEngine::EntityClassDef                                          \
	{                                                                                                       \
	public:                                                                                                 \
		__kbe_module_##DEFNAME()                                                                            \
		{                                                                                                   \
			registerClass(#DEFNAME, this);                                                                  \
		}                                                                                                   \
		KBEngine::Entity* createEntity() override { return new CLASS(); };                                            \
	};                                                                                                      \
	__kbe_module_##DEFNAME *__kbe_module_##DEFNAME##_instance = new __kbe_module_##DEFNAME();        \


}
