#include "KBEnginePrivatePCH.h"
#include "KBEUpdatable.h"

namespace KBEngine
{

	// for Updatable ---------------------------------------------------------------------
	Updatable::Updatable() :
		id_(-1),
		desc_(TEXT("Updatable"))
	{
	}

	Updatable::~Updatable()
	{
		id_ = -1;
	}






	// for Updatables ---------------------------------------------------------------------
	Updatables::Updatables()
	{
	}

	Updatables::~Updatables()
	{
	}

	bool Updatables::Add(Updatable* obj)
	{
		static uint32 idx = 1;
		objects_[idx] = obj;

		// 记录存储位置
		obj->id_ = idx++;

		return true;
	}

	bool Updatables::Remove(Updatable* obj)
	{
		auto num = objects_.erase(obj->id_);
		obj->id_ = -1;
		return num > 0;
	}

	void Updatables::Update()
	{
		auto iter = objects_.begin();
		for (; iter != objects_.end(); )
		{
			if (!iter->second->Update())
			{
				objects_.erase(iter++);
			}
			else
			{
				++iter;
			}
		}
	}

}
