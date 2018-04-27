#pragma once

#include <memory>

#include "KBEDebug.h"
#include "Method.h"
#include "Property.h"

namespace KBEngine
{
	class Entity;

	/*
	对应一个.def类型，以创建相应的实例
	*/
	class KBENGINE_API EntityClassDef
	{
	public:
		EntityClassDef() {};
		virtual ~EntityClassDef() {};

		virtual Entity* CreateEntity() { return nullptr; };

	public:
		inline static void RegisterClass(FString defName, std::shared_ptr<EntityClassDef> def)
		{
			name2entity_.Add(defName, def);
		}

		inline static EntityClassDef* FindClass(FString defName)
		{
			auto pValue = name2entity_.Find(defName);

			if (!pValue)
				return nullptr;
			return pValue->get();
		}

	private:
		static TMap<FString, std::shared_ptr<EntityClassDef>> name2entity_;

	};

	template<class T>
	class EntityClassDefT : public KBEngine::EntityClassDef
	{
	public:
		EntityClassDefT(const FString& name) : name_(name)
		{
			this->RegisterClass(name, std::shared_ptr<EntityClassDef>(this));
		}

		virtual ~EntityClassDefT()
		{}

		virtual KBEngine::Entity* CreateEntity() override { return new T(); }

	private:
		FString name_;
	};



	/*
	一个entitydef中定义的脚本模块的描述类
	包含了某个entity定义的属性与方法以及该entity脚本模块的名称与模块ID
	*/
	class ScriptModule
	{
	public:
		ScriptModule(FString modulename);
		~ScriptModule();

		Entity* CreateEntity(int32 eid);

		FORCEINLINE const FString& Name() { return name_; }
		FORCEINLINE void Name(const FString& name) { name_ = name; }
		
		FORCEINLINE bool UsePropertyDescrAlias() { return usePropertyDescrAlias_; }
		FORCEINLINE void UsePropertyDescrAlias(bool yes) { usePropertyDescrAlias_ = yes; }
		
		FORCEINLINE bool UseMethodDescrAlias() { return useMethodDescrAlias_; }
		FORCEINLINE void UseMethodDescrAlias(bool yes) { useMethodDescrAlias_ = yes; }

		void ClonePropertyTo(TMap<FString, Property *>& out1, TMap<uint16, Property *>& out2);
		Property* MakeProperty(MemoryStream &stream);
		Property* GetProperty(const FString& name);
		Property* GetProperty(uint16 id);


		Method* MakeMethod(MemoryStream &stream);
		Method* GetMethod(const FString& name);
		Method* GetMethod(uint16 id);

		Method* MakeBaseMethod(MemoryStream &stream);
		Method* GetBaseMethod(const FString& name);
		Method* GetBaseMethod(uint16 id);

		Method* MakeCellMethod(MemoryStream &stream);
		Method* GetCellMethod(const FString& name);
		Method* GetCellMethod(uint16 id);

	private:
		FString name_;
		bool usePropertyDescrAlias_ = false;
		bool useMethodDescrAlias_ = false;

		TMap<FString, Property *> propertys_;
		TMap<uint16, Property *> idpropertys_;

		TMap<FString, Method *> methods_;
		TMap<FString, Method *> base_methods_;
		TMap<FString, Method *> cell_methods_;

		TMap<uint16, Method *> idmethods_;
		TMap<uint16, Method *> idbase_methods_;
		TMap<uint16, Method *> idcell_methods_;

		EntityClassDef *script_ = nullptr;
		

	};

}
