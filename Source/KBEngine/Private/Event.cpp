#include "KBEnginePrivatePCH.h"
#include "Event.h"

namespace KBEngine
{
	TSharedPtr<Event> Event::s_event_ = nullptr;

	Event::Event()
	{
	}

	Event::~Event()
	{
		KBE_DEBUG(TEXT("Event::~Event()"));
		Clear();
	}

	void Event::Clear()
	{
		for (auto it : events_) 
		{
			EventFuncArray lst = *(it.Value);
			for (int i = 0; i < lst.Num(); i++)
			{
				delete lst[i];
			}

			delete it.Value;
		}

		events_.Empty();
		firedEvents_.Empty();

		isPauseOut_ = false;
	}

	Event::EventFuncArray* Event::GetEvent(const FString& eventName)
	{
		MonitorEnter(cs_events_);
		auto ent = events_.Find(eventName);
		MonitorExit(cs_events_);

		return ent ? *ent : nullptr;
	}

	void Event::CopyEvent(EventFuncArray& out, const FString& eventName)
	{
		MonitorEnter(cs_events_);
		auto ent = events_.Find(eventName);
		if (ent)
			out = **ent;
		MonitorExit(cs_events_);
	}

	void Event::AddEvent(const FString& eventName, EventPair* eventPair)
	{
		MonitorEnter(cs_events_);
		EventFuncArray** p = events_.Find(eventName);
		EventFuncArray* lst = p ? *p : nullptr;
		if (lst)
		{
			lst->Add(eventPair);
			MonitorExit(cs_events_);
			return;
		}

		EventFuncArray* funcArray = new EventFuncArray();
		funcArray->Add(eventPair);
		events_.Add(eventName, funcArray);
		MonitorExit(cs_events_);
	}

	bool Event::HasRegister(const FString& eventName)
	{
		return nullptr != GetEvent(eventName);
	}

	void Event::Fire(const FString& eventName, const FVariantArray &args)
	{
		EventFuncArray lst;
		// 复制一份函数列表出来，以避免回调过程中因为注销问题而产生列表不一致
		// 更重要的是为了避免在多线程下线程锁重复锁的问题
		CopyEvent(lst, eventName);
		if (lst.Num() == 0)
		{
			KBE_WARNING(TEXT("Event::AsyncFire: event(%s) not found!"), *eventName);
			return;
		}

		for (int j = 0; j < lst.Num(); j++)
		{
			auto pair = lst[j];
			if (pair->func)
				pair->func(args);
		}
	}

	void Event::AsyncFire(const FString& eventName, const FVariantArray &args)
	{
		EventObj eobj;
		eobj.name = eventName;
		eobj.args = args;

		MonitorEnter(cs_firedEvents_);
		firedEvents_.Add(eobj);
		MonitorExit(cs_firedEvents_);
	}

	void Event::ProcessAsyncEvents()
	{
		MonitorEnter(cs_firedEvents_);
		TArray<EventObj> doingEvents = firedEvents_;
		firedEvents_.Reset();
		MonitorExit(cs_firedEvents_);

		if (isPauseOut_)
			return;

		EventFuncArray lst;
		for (int i = 0; i < doingEvents.Num(); i++)
		{
			EventObj eobj = doingEvents[i];

			lst.Reset();
			// 复制一份函数列表出来，以避免回调过程中因为注销问题而产生列表不一致
			// 更重要的是为了避免在多线程下线程锁重复锁的问题
			CopyEvent(lst, eobj.name);
			if (lst.Num() == 0)
			{
				KBE_WARNING(TEXT("Event::AsyncFire: event(%s) not found!"), *eobj.name);
				continue;
			}

			for (int j = 0; j < lst.Num(); j++)
			{
				auto pair = lst[j];
				if (pair->func)
					pair->func(eobj.args);
			}
		}
	}

	bool Event::Register(const FString& eventName, EventFuncPtr func)
	{
		EventPair* pair = new EventPair();
		pair->objAddr = nullptr;
		pair->funcAddr = (void *)func;
		pair->func = EventFunc(func);

		AddEvent(eventName, pair);
		return true;
	}

	bool Event::Deregister(const FString& eventName, EventFuncPtr func)
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

		for (int i = 0; i < lst->Num(); i++)
		{
			auto& pair = (*lst)[i];
			if (pair->objAddr == nullptr && pair->funcAddr == (void *)func)
			{
				KBE_DEBUG(TEXT("Event::Deregister: 3 - event(%s:%p:)!"), *eventName, func);
				delete pair;
				lst->RemoveAt(i);
				MonitorExit(cs_events_);
				return true;
			}
		}

		MonitorExit(cs_events_);
		return false;
	}

}
