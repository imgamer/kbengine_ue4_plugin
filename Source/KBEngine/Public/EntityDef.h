#pragma once

#include "DataTypes.h"
#include "MemoryStream.h"

namespace KBEngine
{

	class ScriptModule;

	/*
	EntityDef模块
	管理了所有的实体定义的描述以及所有的数据类型描述
	*/
	class KBENGINE_API EntityDef
	{
	public:
		static void Init();
		static void Clear();
		static void BindAllDataType();

		static KBEDATATYPE_BASE* GetDataType(uint16 typeID);
		static KBEDATATYPE_BASE* GetDataType(const FString& typeName);
		static int32 DataTypeNum() { return datatypes_.Num(); }

		static void RegisterDataType(const FString& typeName, uint16 typeID, KBEDATATYPE_BASE* inst);

		static ScriptModule* GetScriptModule(uint16 moduleID);
		static ScriptModule* GetScriptModule(const FString& moduleName);
		static int32 ScriptModuleNum() { return moduledefs_.Num(); }

		static void RegisterScriptModule(const FString& moduleName, uint16 moduleID, ScriptModule* inst);
		static bool ImportEntityDefFromStream(MemoryStream &stream);

		static bool EntityDefImported() { return entityDefImported_; }

	private:
		static void InitDataType();
		static void BindMessageDataType();
		static void CreateDataTypesFromStream(MemoryStream &stream);
		static void CreateDataTypeFromStream(MemoryStream &stream);

	private:
		// 所有的数据类型
		static TMap<FString, uint16> datatype2id_;
		static TMap<FString, KBEDATATYPE_BASE *> datatypes_;
		static TMap<uint16, KBEDATATYPE_BASE *> id2datatypes_;

		static TMap<FString, int32> entityclass_;

		static TMap<FString, ScriptModule *> moduledefs_;
		static TMap<uint16, ScriptModule *> idmoduledefs_;

		// 是否已导入数据
		static bool entityDefImported_;
	};
}
