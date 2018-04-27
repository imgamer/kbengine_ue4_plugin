#pragma once
#include <memory>
#include <type_traits>

#include "KBEDefine.h"
#include "KBEDebug.h"

namespace KBEngine
{
	class Entity;

	template<size_t N>
	struct MethodInvokerE {
		template<typename T>
		struct MethodInvoker {
			template<typename F, typename... A>
			static inline void Invoke(KBEngine::Entity *entity, F && f, const FVariantArray &args, A &&... a)
			{
				typedef std::tuple_element<N - 1, T>::type VALUE_TYPE;
				VALUE_TYPE value;
				MethodArg<VALUE_TYPE>::Convert(value, std::forward<const FVariant>(args[N - 1]));
				MethodInvokerE<N - 1>::MethodInvoker<T>::Invoke(entity, std::forward<F>(f),
					args, value,
					std::forward<A>(a)...);
			}
		};
	};

	template<>
	struct MethodInvokerE<0> {
		template<typename T>
		struct MethodInvoker {
			template<typename F, typename... A>
			static inline void Invoke(KBEngine::Entity *entity, F && f, const FVariantArray &args, A &&... a)
			{
				(entity->*std::forward<F>(f))(std::forward<A>(a)...);
			}
		};

	};

	template <class T>
	class MethodArg
	{
	public:
		FORCEINLINE static void Convert(T& out, const FVariant&& v) { out = v.GetValue<T>(); }
	};

	template <>
	class MethodArg<FVariant>
	{
	public:
		FORCEINLINE static void Convert(FVariant& out, const FVariant&& v) { out = std::forward<const FVariant>(v); }
	};

	class KBENGINE_API EntityMethodProxy
	{
	public:
		virtual void Do(KBEngine::Entity *entity, const FVariantArray &args) = 0;
	};

	typedef std::shared_ptr<EntityMethodProxy> EntityMethodProxyPtr;

	template <class ... TYPES>
	class EntityMethodProxyV : public EntityMethodProxy
	{
	public:
		typedef void (KBEngine::Entity::*EMETHOD)(TYPES...);
		typedef std::tuple<typename std::decay<TYPES>::type...> TUPLE;

	private:
		static const size_t _Mysize = sizeof...(TYPES);
		EMETHOD func;

	public:
		EntityMethodProxyV(EMETHOD pfn) : func(pfn) {}

		virtual void Do(KBEngine::Entity *entity, const FVariantArray &args) override
		{
			if (args.Num() != _Mysize)
			{
				KBE_ERROR(TEXT("EntityMethodProxy: args must takes %d arguments (%d given)"), _Mysize, args.Num());
				return;
			}

			MethodInvokerE<_Mysize>::MethodInvoker<TUPLE>::Invoke(entity, func, args);
		}
	};








	struct KBE_ENTITY_METHOD_MAP_ENTRY
	{
		FString name;                       // method name
		EntityMethodProxyPtr pMethodProxy;    // method proxy instance
	};

	struct KBE_ENTITY_METHOD_MAP
	{
		const KBE_ENTITY_METHOD_MAP* (*pfnGetBaseMap)();
		const KBE_ENTITY_METHOD_MAP_ENTRY* lpEntries;
	};


#define KBE_DECLARE_ENTITY_MAP() \
protected: \
	static const KBEngine::KBE_ENTITY_METHOD_MAP* GetThisMethodMap(); \
	virtual const KBEngine::KBE_ENTITY_METHOD_MAP* GetMethodMap() const override; \
	static const KBEngine::KBE_ENTITY_PROPERTY_MAP* GetThisPropertyMap(); \
	virtual const KBEngine::KBE_ENTITY_PROPERTY_MAP* GetPropertyMap() const override; \


#define KBE_BEGIN_ENTITY_METHOD_MAP(theClass, baseClass) \
	const KBEngine::KBE_ENTITY_METHOD_MAP* theClass::GetMethodMap() const \
		{ return GetThisMethodMap(); } \
	const KBEngine::KBE_ENTITY_METHOD_MAP* theClass::GetThisMethodMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass SuperClass;					   \
		static const KBEngine::KBE_ENTITY_METHOD_MAP_ENTRY _methodEntries[] =  \
		{

#define KBE_END_ENTITY_METHOD_MAP() \
		{TEXT(""), nullptr } \
	}; \
		static const KBEngine::KBE_ENTITY_METHOD_MAP methodMap = \
		{ &SuperClass::GetThisMethodMap, &_methodEntries[0] }; \
		return &methodMap; \
	}								  \

#define DECLARE_REMOTE_METHOD(name, func, ...) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<__VA_ARGS__>(static_cast<KBEngine::EntityMethodProxyV<__VA_ARGS__>::EMETHOD>(func)))}, \



// @TODO(penghuawei): 使用c++11特性后，
// 下面的DECLARE_REMOTE_METHOD_0 - DECLARE_REMOTE_METHOD_9已经不再使用，
// 之所以保留是为了兼容旧代码，一定时间后将被删除
#define DECLARE_REMOTE_METHOD_0(name, func) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<>(static_cast<KBEngine::EntityMethodProxyV<>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_1(name, func, T1) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1>(static_cast<KBEngine::EntityMethodProxyV<T1>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_2(name, func, T1, T2) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2>(static_cast<KBEngine::EntityMethodProxyV<T1, T2>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_3(name, func, T1, T2, T3) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_4(name, func, T1, T2, T3, T4) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_5(name, func, T1, T2, T3, T4, T5) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_6(name, func, T1, T2, T3, T4, T5, T6) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_7(name, func, T1, T2, T3, T4, T5, T6, T7) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_8(name, func, T1, T2, T3, T4, T5, T6, T7, T8) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7, T8>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7, T8>::EMETHOD>(func)))}, \

#define DECLARE_REMOTE_METHOD_9(name, func, T1, T2, T3, T4, T5, T6, T7, T8, T9) \
	{TEXT(#name), KBEngine::EntityMethodProxyPtr(new KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7, T8, T9>(static_cast<KBEngine::EntityMethodProxyV<T1, T2, T3, T4, T5, T6, T7, T8, T9>::EMETHOD>(func)))}, \






	class KBENGINE_API EntityPropertyProxy
	{
	public:
		virtual void Do(KBEngine::Entity *entity, const FVariant &newVal, const FVariant &oldVal) = 0;
	};

	typedef std::shared_ptr<EntityPropertyProxy> EntityPropertyProxyPtr;

	template<class T>
	class EntityPropertyProxyT : public EntityPropertyProxy
	{
	public:
		typedef void (KBEngine::Entity::*PMETHOD)(const T &newVal, const T &oldVal);

	private:
		PMETHOD func;

	public:
		EntityPropertyProxyT(PMETHOD pfn) : func(pfn) {}

		virtual void Do(KBEngine::Entity *entity, const FVariant &newVal, const FVariant &oldVal) override
		{
			T nval = newVal.GetValue<T>();
			T oval = oldVal.GetValue<T>();
			(entity->*func)(nval, oval);
		}
	};




	struct KBE_ENTITY_PROPERTY_MAP_ENTRY
	{
		FString name;                       // property name
		EntityPropertyProxyPtr pPropertyProxy;    // property proxy instance
	};

	struct KBE_ENTITY_PROPERTY_MAP
	{
		const KBE_ENTITY_PROPERTY_MAP* (*pfnGetBaseMap)();
		const KBE_ENTITY_PROPERTY_MAP_ENTRY* lpEntries;
	};

#define KBE_BEGIN_ENTITY_PROPERTY_MAP(theClass, baseClass) \
	const KBEngine::KBE_ENTITY_PROPERTY_MAP* theClass::GetPropertyMap() const \
		{ return GetThisPropertyMap(); } \
	const KBEngine::KBE_ENTITY_PROPERTY_MAP* theClass::GetThisPropertyMap() \
	{ \
		typedef theClass ThisClass;						   \
		typedef baseClass SuperClass;					   \
		static const KBEngine::KBE_ENTITY_PROPERTY_MAP_ENTRY _propertyEntries[] =  \
		{

#define KBE_END_ENTITY_PROPERTY_MAP() \
		{TEXT(""), nullptr } \
	}; \
		static const KBEngine::KBE_ENTITY_PROPERTY_MAP propertyMap = \
		{ &SuperClass::GetThisPropertyMap, &_propertyEntries[0] }; \
		return &propertyMap; \
	}								  \

#define DECLARE_PROPERTY_CHANGED_NOTIFY(name, func, T) \
	{TEXT(#name), KBEngine::EntityPropertyProxyPtr(new KBEngine::EntityPropertyProxyT<T>(static_cast<KBEngine::EntityPropertyProxyT<T>::PMETHOD>(func)))}, \















#define ENTITY_DECLARE(DEFNAME, CLASS) \
	new KBEngine::EntityClassDefT<CLASS>((DEFNAME));


}

