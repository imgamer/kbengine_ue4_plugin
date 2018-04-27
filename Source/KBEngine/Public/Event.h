#pragma once
#include <functional>

#include "Core.h"
#include "KBEDefine.h"

namespace KBEngine
{
	class KBENGINE_API Event
	{
	public:
		template<typename T>
		struct ClassMethon
		{
			typedef void(T::*FuncType)(const FVariantArray &);
		};

		typedef void(*EventFuncPtr)(const FVariantArray &);
		typedef std::function<void(const FVariantArray &)> EventFunc;

		typedef struct
		{
			void* objAddr = nullptr;
			void* funcAddr = nullptr;
			EventFunc func;
		}EventPair;

		typedef TArray<EventPair> EventFuncArray;

		typedef struct
		{
			FString name;
			FVariantArray args;
		}EventObj;

	public:
		Event();
		~Event();

		void Clear();

		void Pause()   { isPauseOut_ = true; }
		void Resume()  { isPauseOut_ = false; }
		bool IsPause() { return isPauseOut_; }

		template <class T>
		bool Register(const FString& eventName, T* obj, typename ClassMethon<T>::FuncType func);
		bool Register(const FString& eventName, EventFuncPtr func);

		template <class T>
		bool Deregister(const FString& eventName, const T* obj, typename ClassMethon<T>::FuncType func);
		bool Deregister(const FString& eventName, EventFuncPtr func);

		/*
		注销所有obj有关的注册函数；
		no efficiency
		*/
		template <class T>
		bool Deregister(const T *obj);

		void Fire(const FString& eventName, const FVariantArray &args);
		void AsyncFire(const FString& eventName, const FVariantArray &args);
		void ProcessAsyncEvents();

	public:
		static Event *Instance()
		{
			if (!s_event_.Get())
				s_event_ = TSharedPtr<Event>(new Event());
			return s_event_.Get(); 
		}

	private:
		EventFuncArray* GetEvent(const FString& eventName);
		// 线程安全的事件复制
		void CopyEvent(EventFuncArray& out, const FString& eventName);

		// 线程安全的插入事件到队列中
		void AddEvent(const FString& eventName, EventPair eventPair);

		bool HasRegister(const FString& eventName);

		void MonitorEnter(FCriticalSection& cs) { cs.Lock(); }
		void MonitorExit(FCriticalSection& cs) { cs.Unlock(); }

	private:
		static TSharedPtr<Event> s_event_;

	private:
		/* penghuawei: 这很可能是UE4的一个bug
		   当我们：TMap<FString, EventFuncArray> events_
		   这样声明的时候，使用Event::Instance()->Register("f1", obj, func)注册一个f1事件时，一切正常，
		   但是，当我们再次注册一个f1事件的处理函数时，
		   如果接着有代码构造了FVariant()实例，则events_["f1"][0].func的地址将变得无效，似乎被释放了，
		   所以，当Fire("f1")事件时，则会遇到内存访问无效的问题，导致crash。
		   因此，下面改为使用数组指针，而不是数组实例
		*/
		TMap<FString, EventFuncArray *> events_;
		TArray<EventObj> firedEvents_;

		FCriticalSection cs_events_;
		FCriticalSection cs_firedEvents_;

		bool isPauseOut_ = false;

	};





	template <class T>
	bool Event::Deregister(const FString& eventName, const T* obj, typename ClassMethon<T>::FuncType func)
	{
		MonitorEnter(cs_events_);

		// 不能用 GetEvent()，以避免重复锁
		EventFuncArray** p = events_.Find(eventName);
		EventFuncArray* lst = p ? *p : nullptr;
		if (!lst)
		{
			MonitorExit(cs_events_);
			return false;
		}

		union
		{
			void(T::*f)(const FVariantArray &);
			void* t;
		}ut;

		ut.f = func;

		for (int i = 0; i < lst->Num(); i++)
		{
			auto& pair = (*lst)[i];
			if ((void *)obj == pair.objAddr && pair.funcAddr == ut.t)
			{
				KBE_DEBUG(TEXT("Event::Deregister: 2 - event(%s:%p:%p)!"), *eventName, obj, ut.t);
				lst->RemoveAt(i);
				MonitorExit(cs_events_);
				return true;
			}
		}

		MonitorExit(cs_events_);
		return false;
	}

	template <class T>
	bool Event::Deregister(const T *obj)
	{
		int count = 0;
		MonitorEnter(cs_events_);

		for (auto it : events_)
		{
			EventFuncArray* lst = it.Value;
			// 从后往前遍历，以避免中途删除的问题
			for (int i = lst->Num() - 1; i >= 0; i--)
			{
				const auto& o = (*lst)[i];
				if ((void *)obj == o.objAddr)
				{
					KBE_DEBUG(TEXT("Event::Deregister: 1 - event(%s:%p)!"), *it.Key, o.objAddr);
					lst->RemoveAt(i);
					count++;
				}
			}

		}

		MonitorExit(cs_events_);
		return count > 0;
	}

	template <class T>
	bool Event::Register(const FString& eventName, T* obj, typename ClassMethon<T>::FuncType func)
	{
		union
		{
			void(T::*f)(const FVariantArray &);
			void* t;
		}ut;

		ut.f = func;

		EventPair pair;
		pair.objAddr = (void *)obj;
		pair.funcAddr = ut.t;
		pair.func = std::bind(func, obj, std::placeholders::_1);

		AddEvent(eventName, pair);
		return true;
	}

}
