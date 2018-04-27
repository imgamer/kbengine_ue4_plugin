#pragma once
#include <map>
#include "Core.h"
#include "KBEDebug.h"

namespace KBEngine
{
	class Updatables;

	/*
	用来描述一个总是会被更新的对象， app每个tick都会调用所有的
	Updatable来更新状态， 需要实现不同的Updatable来完成不同的更新特性。
	*/
	class Updatable
	{
		friend Updatables;

	public:
		Updatable();
		virtual ~Updatable();

		virtual bool Update() = 0;
		const FString& Desc() { return desc_; }

	private:
		// 自身在Updatables容器中的唯一标识
		int id_;

		FString desc_;
	};

	class Updatables
	{
	public:
		Updatables();
		~Updatables();

		bool Add(Updatable* obj);
		bool Remove(Updatable* obj);

		void Update();

	private:
		std::map<uint32, Updatable*> objects_;
	};

}
