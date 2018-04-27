#pragma once
#include "KBEDefine.h"

namespace KBEngine
{
	class KBEDATATYPE_BASE;

	/*
	实体定义的方法模块
	抽象出一个def文件中定义的方法，改模块类提供了该方法的相关描述信息
	例如：方法的参数、方法的id、方法对应脚本的handler
	*/
	class KBENGINE_API Method
	{
	public:
		FString name;
		uint16 methodUtype = 0;
		int16 aliasID = -1;
		TArray<KBEDATATYPE_BASE *> args;
		MessageHandler handler = NULL;

		Method()
		{
		}
	};

}
