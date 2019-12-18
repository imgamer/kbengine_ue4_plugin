#include "EntityDef.h"
#include "KBEnginePrivatePCH.h"
#include "ScriptModule.h"

namespace KBEngine
{

	TMap<FString, uint16> EntityDef::datatype2id_;
	TMap<FString, KBEDATATYPE_BASE *> EntityDef::datatypes_;
	TMap<uint16, KBEDATATYPE_BASE *> EntityDef::id2datatypes_;

	TMap<FString, int32> EntityDef::entityclass_;

	TMap<FString, ScriptModule *> EntityDef::moduledefs_;
	TMap<uint16, ScriptModule *> EntityDef::idmoduledefs_;

	bool EntityDef::entityDefImported_ = false;

	void EntityDef::Clear()
	{
		KBE_DEBUG(TEXT("EntityDef::Clear"));

		// 由于基础数据类型有可能会被二次引用，因此地址相同的实例只能保留一份
		TSet<KBEDATATYPE_BASE *> dataTypeSet;
		for (auto it : datatypes_)
			dataTypeSet.Add(it.Value);

		for (auto dt : dataTypeSet)
			delete dt;

		// 注册进来的ScriptModule由EntityDef来清理
		for (auto md : moduledefs_)
			delete md.Value;

		datatype2id_.Empty(0);
		datatypes_.Empty(0);
		id2datatypes_.Empty(0);
		entityclass_.Empty(0);
		moduledefs_.Empty(0);
		idmoduledefs_.Empty(0);
		entityDefImported_ = false;
	}

	void EntityDef::Init()
	{
		Clear();
		InitDataType();
		BindMessageDataType();
	}

	void EntityDef::BindAllDataType()
	{
		for (auto it : datatypes_)
		{
			if (it.Value)
				it.Value->Bind();
		}
	}

	KBEDATATYPE_BASE* EntityDef::GetDataType(uint16 typeID)
	{
		auto** p = id2datatypes_.Find(typeID);
		if (p)
			return *p;
		return nullptr;
	}

	KBEDATATYPE_BASE* EntityDef::GetDataType(const FString& typeName)
	{
		auto** p = datatypes_.Find(typeName);
		if (p)
			return *p;
		return nullptr;
	}

	void EntityDef::RegisterDataType(const FString& typeName, uint16 typeID, KBEDATATYPE_BASE* inst)
	{
		datatypes_.Add(typeName, inst);
		id2datatypes_.Add(typeID, inst);
		datatype2id_.Add(typeName, typeID);

		if (!inst)
			KBE_ERROR(TEXT("EntityDef::RegisterDataType: data type(%s:%d) has no match instance!"), *typeName, typeID);
	}

	ScriptModule* EntityDef::GetScriptModule(uint16 moduleID)
	{
		auto** p = idmoduledefs_.Find(moduleID);
		if (p)
			return *p;
		return nullptr;
	}

	ScriptModule* EntityDef::GetScriptModule(const FString& moduleName)
	{
		auto** p = moduledefs_.Find(moduleName);
		if (p)
			return *p;
		return nullptr;
	}

	void EntityDef::RegisterScriptModule(const FString& moduleName, uint16 moduleID, ScriptModule* inst)
	{
		moduledefs_.Add(moduleName, inst);
		idmoduledefs_.Add(moduleID, inst);
	}

	void EntityDef::InitDataType()
	{
		datatypes_.Add("UINT8", new KBEDATATYPE_UINT8());
		datatypes_.Add("UINT16", new KBEDATATYPE_UINT16());
		datatypes_.Add("UINT32", new KBEDATATYPE_UINT32());
		datatypes_.Add("UINT64", new KBEDATATYPE_UINT64());

		datatypes_.Add("INT8", new KBEDATATYPE_INT8());
		datatypes_.Add("INT16", new KBEDATATYPE_INT16());
		datatypes_.Add("INT32", new KBEDATATYPE_INT32());
		datatypes_.Add("INT64", new KBEDATATYPE_INT64());

		datatypes_.Add("FLOAT", new KBEDATATYPE_FLOAT());
		datatypes_.Add("DOUBLE", new KBEDATATYPE_DOUBLE());

		datatypes_.Add("STRING", new KBEDATATYPE_STRING());
		datatypes_.Add("VECTOR2", new KBEDATATYPE_VECTOR2());
		datatypes_.Add("VECTOR3", new KBEDATATYPE_VECTOR3());
		datatypes_.Add("VECTOR4", new KBEDATATYPE_VECTOR4());
		datatypes_.Add("PYTHON", new KBEDATATYPE_PYTHON());
		datatypes_.Add("PY_DICT", new KBEDATATYPE_PY_DICT());
		datatypes_.Add("PY_TUPLE", new KBEDATATYPE_PY_TUPLE());
		datatypes_.Add("PY_LIST", new KBEDATATYPE_PY_LIST());
		datatypes_.Add("UNICODE", new KBEDATATYPE_UNICODE());
		datatypes_.Add("ENTITYCALL", new KBEDATATYPE_MAILBOX());
		datatypes_.Add("BLOB", new KBEDATATYPE_BLOB());
	}

	void EntityDef::BindMessageDataType()
	{
		KBE_ASSERT(datatype2id_.Num() == 0);

		datatype2id_.Add("STRING", 1);
		datatype2id_.Add("STD::STRING", 1);

		id2datatypes_.Add(1, datatypes_["STRING"]);

		datatype2id_.Add("UINT8", 2);
		datatype2id_.Add("BOOL", 2);
		datatype2id_.Add("DATATYPE", 2);
		datatype2id_.Add("CHAR", 2);
		datatype2id_.Add("DETAIL_TYPE", 2);
		datatype2id_.Add("MAIL_TYPE", 2);

		id2datatypes_.Add(2, datatypes_["UINT8"]);

		datatype2id_.Add("UINT16", 3);
		datatype2id_.Add("UNSIGNED SHORT", 3);
		datatype2id_.Add("SERVER_ERROR_CODE", 3);
		datatype2id_.Add("ENTITY_TYPE", 3);
		datatype2id_.Add("ENTITY_PROPERTY_UID", 3);
		datatype2id_.Add("ENTITY_METHOD_UID", 3);
		datatype2id_.Add("ENTITY_SCRIPT_UID", 3);
		datatype2id_.Add("DATATYPE_UID", 3);

		id2datatypes_.Add(3, datatypes_["UINT16"]);

		datatype2id_.Add("UINT32", 4);
		datatype2id_.Add("UINT", 4);
		datatype2id_.Add("UNSIGNED INT", 4);
		datatype2id_.Add("ARRAYSIZE", 4);
		datatype2id_.Add("SPACE_ID", 4);
		datatype2id_.Add("GAME_TIME", 4);
		datatype2id_.Add("TIMER_ID", 4);

		id2datatypes_.Add(4, datatypes_["UINT32"]);

		datatype2id_.Add("UINT64", 5);
		datatype2id_.Add("DBID", 5);
		datatype2id_.Add("COMPONENT_ID", 5);

		id2datatypes_.Add(5, datatypes_["UINT64"]);

		datatype2id_.Add("INT8", 6);
		datatype2id_.Add("COMPONENT_ORDER", 6);

		id2datatypes_.Add(6, datatypes_["INT8"]);

		datatype2id_.Add("INT16", 7);
		datatype2id_.Add("SHORT", 7);

		id2datatypes_.Add(7, datatypes_["INT16"]);

		datatype2id_.Add("INT32", 8);
		datatype2id_.Add("INT", 8);
		datatype2id_.Add("ENTITY_ID", 8);
		datatype2id_.Add("CALLBACK_ID", 8);
		datatype2id_.Add("COMPONENT_TYPE", 8);

		id2datatypes_.Add(8, datatypes_["INT32"]);

		datatype2id_.Add("INT64", 9);

		id2datatypes_.Add(9, datatypes_["INT64"]);

		datatype2id_.Add("PYTHON", 10);
		datatype2id_.Add("PY_DICT", 10);
		datatype2id_.Add("PY_TUPLE", 10);
		datatype2id_.Add("PY_LIST", 10);

		id2datatypes_.Add(10, datatypes_["PYTHON"]);

		datatype2id_.Add("BLOB", 11);

		id2datatypes_.Add(11, datatypes_["BLOB"]);

		datatype2id_.Add("UNICODE", 12);

		id2datatypes_.Add(12, datatypes_["UNICODE"]);

		datatype2id_.Add("FLOAT", 13);

		id2datatypes_.Add(13, datatypes_["FLOAT"]);

		datatype2id_.Add("DOUBLE", 14);

		id2datatypes_.Add(14, datatypes_["DOUBLE"]);

		datatype2id_.Add("VECTOR2", 15);

		id2datatypes_.Add(15, datatypes_["VECTOR2"]);

		datatype2id_.Add("VECTOR3", 16);

		id2datatypes_.Add(16, datatypes_["VECTOR3"]);

		datatype2id_.Add("VECTOR4", 17);

		id2datatypes_.Add(17, datatypes_["VECTOR4"]);

		datatype2id_.Add("FIXED_DICT", 18);
		// 这里不需要绑定，FIXED_DICT需要根据不同类型实例化动态得到id
		//id2datatypes_[18] = datatypes_["FIXED_DICT"];

		datatype2id_.Add("ARRAY", 19);
		// 这里不需要绑定，ARRAY需要根据不同类型实例化动态得到id
		//id2datatypes_[19] = datatypes_["ARRAY"];

		datatype2id_.Add("ENTITYCALL", 20);

		id2datatypes_.Add(20, datatypes_["ENTITYCALL"]);
	}

	void EntityDef::CreateDataTypeFromStream(MemoryStream &stream)
	{
		uint16 utype = stream.ReadUint16();
		FString name = stream.ReadString();
		FString valname = stream.ReadString();

		/* 有一些匿名类型，我们需要提供一个唯一名称放到datatypes中
		如：
		<onRemoveAvatar>
		<Arg>	ARRAY <of> INT8 </of>		</Arg>
		</onRemoveAvatar>
		*/
		if (valname.Len() == 0)
			valname = FString::Printf(TEXT("Null_%d"), utype);

		KBE_DEBUG(TEXT("EntityDef::CreateDataTypeFromStream: importAlias(%s:%s:%d)!"), *name, *valname, utype);

		if (name == "FIXED_DICT")
		{
			uint8 keysize = stream.ReadUint8();
			FString implementedBy = stream.ReadString();
			KBEDATATYPE_FIXED_DICT* datatype = new KBEDATATYPE_FIXED_DICT(implementedBy);

			while (keysize > 0)
			{
				keysize--;

				FString keyname = stream.ReadString();
				uint16 keyutype = stream.ReadUint16();
				datatype->AddSubType(keyname, keyutype);
			};

			RegisterDataType(valname, utype, datatype);
		}
		else if (name == "ARRAY")
		{
			uint16 uitemtype = stream.ReadUint16();
			KBEDATATYPE_ARRAY* datatype = new KBEDATATYPE_ARRAY(uitemtype);
			RegisterDataType(valname, utype, datatype);
		}
		else
		{
			KBEDATATYPE_BASE* datatype = GetDataType(name);
			RegisterDataType(valname, utype, datatype);
		}
	}

	void EntityDef::CreateDataTypesFromStream(MemoryStream &stream)
	{
		uint16 aliassize = stream.ReadUint16();
		KBE_DEBUG(TEXT("EntityDef::CreateDataTypesFromStream: importAlias(size=%d)!"), aliassize);

		while (aliassize > 0)
		{
			aliassize--;
			CreateDataTypeFromStream(stream);
		};

		// 重新绑定各数据类型之前的引用关系
		BindAllDataType();
	}

	bool EntityDef::ImportEntityDefFromStream(MemoryStream &stream)
	{
		// @TODO(penghuawei): 这里当前没有对数据流的有效性进行较验，
		// 所以如果数据流来自本地且非法，就将有可能导致客户端出现未知的问题


		CreateDataTypesFromStream(stream);

		while (stream.Length() > 0)
		{
			FString scriptmethod_name = stream.ReadString();
			uint16 scriptUtype = stream.ReadUint16();
			uint16 propertysize = stream.ReadUint16();
			uint16 methodsize = stream.ReadUint16();
			uint16 base_methodsize = stream.ReadUint16();
			uint16 cell_methodsize = stream.ReadUint16();

			KBE_DEBUG(TEXT("EntityDef::ImportEntityDefFromStream: import(%s), propertys(%d), clientMethods(%d), baseMethods(%d), cellMethods(%d)!"),
				*scriptmethod_name, propertysize, methodsize, base_methodsize, cell_methodsize);

			ScriptModule* module = new ScriptModule(scriptmethod_name);
			EntityDef::RegisterScriptModule(scriptmethod_name, scriptUtype, module);

			if (propertysize > 255)
				module->UsePropertyDescrAlias(false);
			else
				module->UsePropertyDescrAlias(true);

			while (propertysize > 0)
			{
				propertysize--;
				module->MakeProperty(stream);
			};

			if (methodsize > 255)
				module->UseMethodDescrAlias(false);
			else
				module->UseMethodDescrAlias(true);

			while (methodsize > 0)
			{
				methodsize--;
				module->MakeMethod(stream);
			};

			while (base_methodsize > 0)
			{
				base_methodsize--;
				module->MakeBaseMethod(stream);
			};

			while (cell_methodsize > 0)
			{
				cell_methodsize--;
				module->MakeCellMethod(stream);
			};

			//if (module.script == null)
			//{
			//	Dbg.ERROR_MSG("EntityDef::ImportEntityDefFromStream: module(" + scriptmethod_name + ") not found!");
			//}

			//foreach(string name in module.methods.Keys)
			//{
			//	// Method infos = module.methods[name];

			//	if (module.script != null && module.script.GetMethod(name) == null)
			//	{
			//		Dbg.WARNING_MSG(scriptmethod_name + "(" + module.script + "):: method(" + name + ") no implement!");
			//	}
			//};
		}

		entityDefImported_ = true;
		return true;
	}
}
