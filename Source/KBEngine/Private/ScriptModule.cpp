#include "ScriptModule.h"
#include "KBEnginePrivatePCH.h"
#include "Entity.h"
#include "MemoryStream.h"
#include "EntityDef.h"

namespace KBEngine
{

	TMap<FString, std::shared_ptr<EntityClassDef>> EntityClassDef::name2entity_;

	ScriptModule::ScriptModule(FString modulename)
	{
		name_ = modulename;

		script_ = EntityClassDef::FindClass(modulename);

		if (!script_)
			KBE_ERROR(TEXT("ScriptModule::ScriptModule(), can't load script module(%s)!"), *modulename);
	}

	ScriptModule::~ScriptModule()
	{
		KBE_DEBUG(TEXT("ScriptModule::~ScriptModule, %s"), *name_);

		for (auto it : methods_)
		{
			if (it.Value)
				delete it.Value;
		}

		for (auto it : base_methods_)
		{
			if (it.Value)
				delete it.Value;
		}

		for (auto it : cell_methods_)
		{
			if (it.Value)
				delete it.Value;
		}

	}

	Entity* ScriptModule::CreateEntity(int32 eid)
	{
		Entity* entity = nullptr;
		if (script_)
			entity = script_->CreateEntity();

		if (!entity)
		{
			KBE_ERROR(TEXT("ScriptModule::CreateEntity: module '%s' has no entity script! entityID = %d"), *name_, eid);
			entity = new UnknownEntity();
		}

		entity->ID(eid);
		entity->ClassName(name_);
		entity->InitProperties(*this);

		return entity;
	}

	Property* ScriptModule::GetProperty(const FString& name)
	{
		Property** pp = propertys_.Find(name);
		return pp ? *pp : nullptr;
	}

	Property* ScriptModule::GetProperty(uint16 id)
	{
		Property** pp = idpropertys_.Find(id);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetMethod(const FString& name)
	{
		Method** pp = methods_.Find(name);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetMethod(uint16 id)
	{
		Method** pp = idmethods_.Find(id);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetBaseMethod(const FString& name)
	{
		Method** pp = base_methods_.Find(name);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetBaseMethod(uint16 id)
	{
		Method** pp = idbase_methods_.Find(id);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetCellMethod(const FString& name)
	{
		Method** pp = cell_methods_.Find(name);
		return pp ? *pp : nullptr;
	}

	Method* ScriptModule::GetCellMethod(uint16 id)
	{
		Method** pp = idcell_methods_.Find(id);
		return pp ? *pp : nullptr;
	}

	Property* ScriptModule::MakeProperty(MemoryStream &stream)
	{
		Property* savedata = new Property();

		savedata->properUtype = stream.ReadUint16();
		savedata->properFlags = stream.ReadUint32();
		savedata->aliasID = stream.ReadInt16();
		savedata->name = stream.ReadString();
		savedata->defaultValStr = stream.ReadString();
		savedata->utype = EntityDef::GetDataType(stream.ReadUint16());;

		savedata->val = savedata->utype->ParseDefaultValStr(savedata->defaultValStr);

		//Type Class = module.script;
		//PropertyHandler setmethod = null;

		//if (Class != NULL)
		//{
		//	try{
		//		setmethod = Class.GetMethod("set_" + name);
		//	}
		//	catch (Exception e)
		//	{
		//		string err = "ScriptModule::MakeProperty: " +
		//			scriptmethod_name + ".set_" + name + ", error=" + e.ToString();

		//		throw new Exception(err);
		//	}
		//}

		//oo//savedata->setmethod = setmethod;

		propertys_.Add(savedata->name, savedata);

		if (UsePropertyDescrAlias())
		{
			idpropertys_.Add((uint16)savedata->aliasID, savedata);
		}
		else
		{
			idpropertys_.Add(savedata->properUtype, savedata);
		}

		KBE_DEBUG(TEXT("ScriptModule::MakeProperty: add(%s), property(%s/%d)."), *name_, *savedata->name, savedata->properUtype);

		return savedata;
	}

	Method* ScriptModule::MakeMethod(MemoryStream &stream)
	{
		Method* method = new Method();
		method->methodUtype = stream.ReadUint16();
		method->aliasID = stream.ReadInt16();
		method->name = stream.ReadString();

		uint8 argssize = stream.ReadUint8();
		uint8 size = argssize;
		while (argssize > 0)
		{
			uint16 datatype = stream.ReadUint16();
			auto* arg = EntityDef::GetDataType(datatype);
			method->args.Add(arg);

			if (!arg)
				KBE_ERROR(TEXT("ScriptModule::MakeMethod: module method(%s:%s) arg(%d) has no data type(%d) match!"), *name_, *method->name, size - argssize, datatype);

			argssize--;
		};


		//Type Class = module.script;
		//if (Class != null)
		//{
		//	try{
		//		savedata.handler = Class.GetMethod(name);
		//	}
		//	catch (Exception e)
		//	{
		//		string err = "ScriptModule::MakeMethod: " + scriptmethod_name + "." + name + ", error=" + e.ToString();
		//		throw new Exception(err);
		//	}
		//}

		methods_.Add(method->name, method);

		if (UseMethodDescrAlias())
		{
			idmethods_.Add((uint16)method->aliasID, method);
		}
		else
		{
			idmethods_.Add(method->methodUtype, method);
		}

		KBE_DEBUG(TEXT("ScriptModule::MakeMethod: add(%s), method(%s)."), *name_, *method->name);

		return method;
	}

	Method* ScriptModule::MakeBaseMethod(MemoryStream &stream)
	{
		Method* method = new Method();
		method->methodUtype = stream.ReadUint16();
		method->aliasID = stream.ReadInt16();
		method->name = stream.ReadString();
		
		uint8 argssize = stream.ReadUint8();
		uint8 size = argssize;
		while (argssize > 0)
		{
			uint16 datatype = stream.ReadUint16();
			auto* arg = EntityDef::GetDataType(datatype);
			method->args.Add(arg);

			if (!arg)
				KBE_ERROR(TEXT("ScriptModule::MakeBaseMethod: module method(%s:%s) arg(%d) has no data type(%d) match!"), *name_, *method->name, size - argssize, datatype);

			argssize--;
		};

		base_methods_.Add(method->name, method);
		idbase_methods_.Add(method->methodUtype, method);
		
		KBE_DEBUG(TEXT("ScriptModule::MakeBaseMethod: add(%s), base_method(%s)."), *name_, *method->name);

		return method;
	}

	Method* ScriptModule::MakeCellMethod(MemoryStream &stream)
	{
		Method* method = new Method();
		method->methodUtype = stream.ReadUint16();
		method->aliasID = stream.ReadInt16();
		method->name = stream.ReadString();
		
		uint8 argssize = stream.ReadUint8();
		uint8 size = argssize;
		while (argssize > 0)
		{
			uint16 datatype = stream.ReadUint16();
			auto* arg = EntityDef::GetDataType(datatype);
			method->args.Add(arg);

			if (!arg)
				KBE_ERROR(TEXT("ScriptModule::MakeCellMethod: module method(%s:%s) arg(%d) has no data type(%d) match!"), *name_, *method->name, size - argssize, datatype);

			argssize--;
		};

		cell_methods_.Add(method->name, method);
		idcell_methods_.Add(method->methodUtype, method);

		KBE_DEBUG(TEXT("ScriptModule::MakeCellMethod: add(%s), cell_method(%s)."), *name_, *method->name);

		return method;
	}

	void ScriptModule::ClonePropertyTo(TMap<FString, Property *>& out1, TMap<uint16, Property *>& out2)
	{
		for (auto it : propertys_)
		{
			Property *e = it.Value;
			Property *newp = new Property();
			newp->name = e->name;
			newp->utype = e->utype;
			newp->properUtype = e->properUtype;
			newp->properFlags = e->properFlags;
			newp->aliasID = e->aliasID;
			newp->defaultValStr = e->defaultValStr;
			newp->setmethod = e->setmethod;
			newp->val = newp->utype->ParseDefaultValStr(newp->defaultValStr);

			out1.Add(e->name, newp);
			out2.Add(e->properUtype, newp);
		}
	}
}
