#include "Entity.h"
#include "KBEnginePrivatePCH.h"
#include "EntityDef.h"
#include "Property.h"
#include "Mailbox.h"
#include "Method.h"
#include "ScriptModule.h"
#include "KBEngineApp.h"
#include "BaseApp.h"


namespace KBEngine
{
	Entity::Entity()
	{
	}

	Entity::~Entity()
	{
		KBE_DEBUG(TEXT("Entity::~Entity(), %d"), id_);

		// delete properties
		for (auto it = defpropertys_.CreateIterator(); it; ++it) {
			delete it.Value();
		}
		defpropertys_.Empty(0);
		iddefpropertys_.Empty(0);

		SAFE_DELETE(baseMailbox_);
		SAFE_DELETE(cellMailbox_);

		if (actor_)
		{
			actor_->Destroy();
			actor_ = nullptr;
		}
	}

	void Entity::InitProperties(ScriptModule& scriptModule)
	{
		scriptModule.ClonePropertyTo(defpropertys_, iddefpropertys_);
	}

	void Entity::RemoteMethodCall(const FString &name, const TArray<FVariant> &args)
	{
		//KBE_ERROR(TEXT("Entity::RemoteMethodCall: Entity (id: %d) has no method '%s' to bound!"), id_, *name);
		OnRemoteMethodCall(name, args);
	}

	void Entity::OnRemoteMethodCall(const FString &name, const TArray<FVariant> &args)
	{
		//KBE_ERROR(TEXT("Entity::OnRemoteMethodCall: Entity (id: %d) has no method '%s' to bound!"), id_, *name);
		// wsf:在此函数进行远程方法调用的处理，以便外部使用者可以根据需求进行控制
		// 例如在某一时刻远程消息太多的情况下不立刻触发调用，而是缓存起来根据某种策略平滑处理
		auto map = GetMethodMap();
		if (!map)
			return;

		do
		{
			if (map->lpEntries)
			{
				auto pEntries = map->lpEntries;
				while (!(*pEntries).name.IsEmpty())
				{
					if ((*pEntries).name == name)
					{
						(*pEntries).pMethodProxy->Do(this, args);
						return;
					}

					pEntries++;
				}
			}

			map = map->pfnGetBaseMap();

		} while (map);
	}

	bool Entity::IsPlayer()
	{
		auto* p = KBEngineApp::app->Player();
		if (p)
			return id_ == p->ID();
		return false;
	}

	void Entity::AddDefinedProperty(FString name, const FVariant &v)
	{
		PrintString(name);
		Property *newp = new Property();
		newp->name = name;
		newp->properUtype = 0;
		newp->val = v;
		newp->setmethod = NULL;
		defpropertys_.Add(name, newp);
	}

	FVariant Entity::GetDefinedProperty(FString name)
	{
		auto** p = defpropertys_.Find(name);
		Property *obj = p ? *p : nullptr;

		KBE_ASSERT(obj);
		return obj->val;
	}

	void Entity::SetDefinedProperty(FString name, const FVariant &val)
	{
		auto** p = defpropertys_.Find(name);
		Property *obj = p ? *p : nullptr;
		
		KBE_ASSERT(obj);
		obj->val = val;
	}

	FVariant Entity::GetDefinedPropertyByUType(uint16 utype)
	{
		auto** p = iddefpropertys_.Find(utype);
		Property *obj = p ? *p : nullptr;

		KBE_ASSERT(obj);
		return obj->val;
	}

	void Entity::SetDefinedPropertyByUType(uint16 utype, const FVariant &val)
	{
		auto** p = iddefpropertys_.Find(utype);
		Property *obj = p ? *p : nullptr;

		KBE_ASSERT(obj);
		obj->val = val;
	}

	void Entity::CallPropertysSetMethods()
	{
		for (auto it = iddefpropertys_.CreateIterator(); it; ++it)
		{
			Property *prop = it.Value();
			FVariant oldval = GetDefinedPropertyByUType(prop->properUtype);
			PropertyHandler setmethod = prop->setmethod;

			//if (setmethod != NULL)
			{
				if (prop->IsBase())
				{
					if (inited_ && !inWorld_)
					{
						//Dbg.DEBUG_MSG(className + "::callPropertysSetMethods(" + prop.name + ")"); 
						//setmethod(this, oldval);
						OnUpdateProperty(prop->name, oldval, oldval);
					}
				}
				else
				{
					if (inWorld_)
					{
						//Dbg.DEBUG_MSG(className + "::callPropertysSetMethods(" + prop.name + ")"); 
						if (prop->IsOwnerOnly() && !IsPlayer())
							continue;
						//setmethod(this, oldval);
						OnUpdateProperty(prop->name, oldval, oldval);
					}
				}
			}
			//else
			//{
			//	//Dbg.DEBUG_MSG(className + "::callPropertysSetMethods(" + prop.name + ") not found set_*"); 
			//}
		}
	}

	void Entity::BaseCall(const FString &methodname, const FVariantArray &arguments)
	{
		if (!KBEngineApp::app->pBaseApp())
		{
			KBE_ERROR(TEXT("%s::BaseCall(%s), but no baseapp found!"), *className_, *methodname);
			return;
		}

		ScriptModule* scriptModule = EntityDef::GetScriptModule(className_);
		Method *method = scriptModule->GetBaseMethod(methodname);
		if (!method)
		{
			KBE_ERROR(TEXT("%s::BaseCall(%s), not found method!"), *className_, *methodname);
			return;
		}

		uint16 methodID = method->methodUtype;

		if (arguments.Num() != method->args.Num())
		{
			KBE_ERROR(TEXT("%s::BaseCall(%s): args(%d!=%d) size is error!"),
				*className_, *methodname, arguments.Num(), method->args.Num());
			return;
		}

		if (!baseMailbox_)
		{
			KBE_ERROR(TEXT("%s::BaseCall(%s): no cell!"), *className_, *methodname);
			return;
		}

		auto* bundle = baseMailbox_->NewMail();
		bundle->WriteUint16(methodID);

		for (int i = 0; i<method->args.Num(); i++)
		{
			if (method->args[i]->IsSameType(arguments[i]))
			{
				method->args[i]->AddToStream(bundle, arguments[i]);
			}
			else
			{
				throw FString::Printf(TEXT("arg%d: %s"), i, method->args[i]->TypeString());
			}
		}

		if (!EntityCallEnable())
		{
			KBE_ERROR(TEXT("%s::BaseCall(%s), but entity call is disable!"), *className_, *methodname);
			return;
		}

		baseMailbox_->PostMail(KBEngineApp::app->pBaseApp()->pNetworkInterface());
	}

	void Entity::CellCall(const FString &methodname, const FVariantArray &arguments)
	{
		if (!KBEngineApp::app->pBaseApp())
		{
			KBE_ERROR(TEXT("%s::CellCall(%s), but no baseapp found!"), *className_, *methodname);
			return;
		}

		ScriptModule* scriptModule = EntityDef::GetScriptModule(className_);
		Method *method = scriptModule->GetCellMethod(methodname);
		if (!method)
		{
			KBE_ERROR(TEXT("%s::CellCall(%s), not found method!"), *className_, *methodname);
			return;
		}

		uint16 methodID = method->methodUtype;

		if (arguments.Num() != method->args.Num())
		{
			KBE_ERROR(TEXT("%s::CellCall(%s): args(%d!=%d) size is error!"),
				*className_, *methodname, arguments.Num(), method->args.Num());
			return;
		}

		if (!cellMailbox_)
		{
			KBE_ERROR(TEXT("%s::CellCall(%s): no cell!"), *className_, *methodname);
			return;
		}

		if (!EntityCallEnable())
		{
			KBE_ERROR(TEXT("%s::CellCall(%s), but entity call is disable!"), *className_, *methodname);
			return;
		}

		auto* bundle = cellMailbox_->NewMail();
		bundle->WriteUint16(methodID);

		for (int i = 0; i<method->args.Num(); i++)
		{
			if (method->args[i]->IsSameType(arguments[i]))
			{
				method->args[i]->AddToStream(bundle, arguments[i]);
			}
			else
			{
				throw FString::Printf(TEXT("arg%d: %s"), i, method->args[i]->TypeString());
			}
		}

		cellMailbox_->PostMail(KBEngineApp::app->pBaseApp()->pNetworkInterface());
	}

	void Entity::EnterWorld()
	{
		inWorld_ = true;

		OnEnterWorld();
	}

	void Entity::LeaveWorld()
	{
		OnLeaveWorld();
		inWorld_ = false;
	}

	void Entity::EnterSpace()
	{
		inWorld_ = true;

		OnEnterSpace();
	}

	void Entity::LeaveSpace()
	{
		OnLeaveSpace();
		inWorld_ = false;
	}

	void Entity::OnPositionSet(const FVector &unrealPos)
	{
		FVector old = position_;
		position_ = lastSyncPos_ = unrealPos;

		if (IsPlayer())
			KBEngineApp::app->pBaseApp()->EntityServerPos(position_);

		if (parent_)
			localPosition_ = parent_->PositionWorldToLocal(position_);
		else
			localPosition_ = position_;
		SyncVolatileDataToChildren(true);

		Set_Position(old);
	}

	void Entity::OnDirectionSet(const FVector &unrealDir)
	{
		FVector old = direction_;
		direction_ = lastSyncDir_ = unrealDir;

		if (parent_)
			localDirection_ = parent_->DirectionWorldToLocal(direction_);
		else
			localDirection_ = direction_;
		SyncVolatileDataToChildren(false);

		Set_Direction(old);
	}


	void Entity::Set_Position(const FVector &oldVal)
	{
		//KBE_INFO(TEXT("%s::Set_Position: %s => %s"), *className_, *oldVal.ToString(), *position_.ToString());
	}

	void Entity::Set_Direction(const FVector &oldVal)
	{
		//KBE_INFO(TEXT("%s::set_direction: %s => %s"), *className_, *oldVal.ToString(), *direction_.ToString());
	}

	void Entity::OnUpdateProperty(const FString &name, const FVariant &newVal, const FVariant &oldVal)
	{
		FVariantArray args;
		args.Add(name);
		args.Add(newVal);
		args.Add(oldVal);

		auto map = GetPropertyMap();
		if (!map)
			return;

		do
		{
			if (map->lpEntries)
			{
				auto pEntries = map->lpEntries;
				while (!(*pEntries).name.IsEmpty())
				{
					if ((*pEntries).name == name)
					{
						(*pEntries).pPropertyProxy->Do(this, newVal, oldVal);
						return;
					}

					pEntries++;
				}
			}

			map = map->pfnGetBaseMap();

		} while (map);

		//KBE_ERROR(TEXT("Entity::onUpdateProperty: Entity (id: %d) has no property method '%s' to bound!"), id, *name);
	}

	void Entity::SetControlled(bool yes)
	{
		isControlled_ = yes;
		OnControlled(isControlled_);
	}

	void Entity::Destroy()
	{
		OnDestroy();

		// 销毁自身只代表自己不见了，不代表对方没有父了
		// 因此这里只改变父对象的指向，但parentID的值仍然保留
		for (auto iter : children_)
			iter.Value->parent_ = nullptr;
		children_.Reset();

		// 解引用
		if (parent_)
		{
			parent_->RemoveChild(this);
			parent_ = nullptr;
		}

		delete this;
	}

	void Entity::SetParent(Entity* ent)
	{
		if (ent == parent_)
			return;

		if (parent_)
		{
			parentID_ = 0;
			parent_->RemoveChild(this);
			localPosition_ = position_;
			localDirection_ = direction_;
			parent_ = nullptr;
			if (inWorld_)
				OnLoseParentEntity();
		}

		parent_ = ent;

		if (parent_)
		{
			parentID_ = ent->ID();
			parent_->AddChild(this);
			localPosition_ = parent_->PositionWorldToLocal(position_);
			localDirection_ = parent_->DirectionWorldToLocal(direction_);
			if (inWorld_)
				OnGotParentEntity();
		}
	}

	void Entity::SetParentOnEnterWorld(Entity* parent, const FVector& localpos, const FVector& localdir)
	{
		parent_ = parent;
		parentID_ = parent->ID();
		parent_->AddChild(this);
		localPosition_ = localpos;
		localDirection_ = localdir;
		if (InWorld())
			OnGotParentEntity();
	}

	void Entity::SyncVolatileDataToChildren(bool positionOnly)
	{
		for (auto iter : children_)
		{
			Entity* ent = iter.Value;

			ent->position_ = PositionLocalToWorld(ent->LocalPosition());

			// 设置最后更新值，以避免被控制者向服务器发送世界坐标或朝向
			ent->lastSyncPos_ = ent->position_;

			if (!positionOnly)
			{
				ent->direction_ = DirectionLocalToWorld(ent->LocalDirection());
			
				// 设置最后更新值，以避免被控制者向服务器发送世界坐标或朝向
				ent->lastSyncDir_ = ent->direction_;
			}

			// 对于玩家自已或被本机控制的entity而言，因父对象的移动而移动，
			// 新坐标不需要通知服务器，因为每个客户端都会做同样的处理，服务器也会自行计算。
			if (ent->IsPlayer() || ent->isControlled_)
			{
				ent->lastSyncPos_ = ent->position_;
				ent->lastSyncDir_ = ent->direction_;
			}
		}
	}

	void Entity::SyncAndNotifyVolatileDataToChildren(bool positionOnly)
	{
		SyncVolatileDataToChildren(positionOnly);
		for (auto iter : children_)
		{
			iter.Value->OnUpdateVolatileDataByParent();
		}
	}

	void Entity::UpdateVolatileDataToServer(const FVector& pos, const FVector& dir)
	{
		bool posChanged = false;
		bool dirChanged = false;

		if (FVector::Dist(position_, pos) > 0.001f)
		{
			position_ = pos;
			posChanged = true;

			if (parent_)
				localPosition_ = parent_->PositionWorldToLocal(position_);
			else
				localPosition_ = position_;
		}

		if (FVector::Dist(direction_, dir) > 0.001f)
		{
			direction_ = dir;
			dirChanged = true;

			if (parent_)
				localDirection_ = parent_->DirectionWorldToLocal(direction_);
			else
				localDirection_ = direction_;
		}

		if (dirChanged)
			SyncVolatileDataToChildren(false);  // 父的朝向改变会同时计算子对象的朝向和位置，所以需要先判断
		else if (posChanged)
			SyncVolatileDataToChildren(true);
	}

	FVector Entity::PositionLocalToWorld(const FVector& localPos)
	{
		if (Actor())
			return KBEMath::PositionLocalToWorld(Actor()->GetActorLocation(), Actor()->GetActorRotation().Euler(), localPos);
		else
			return KBEMath::PositionLocalToWorld(position_, direction_, localPos);
	}

	FVector Entity::PositionWorldToLocal(const FVector& worldPos)
	{
		if (Actor())
			return KBEMath::PositionWorldToLocal(Actor()->GetActorLocation(), Actor()->GetActorRotation().Euler(), worldPos);
		else
			return KBEMath::PositionWorldToLocal(position_, direction_, worldPos);
	}

	FVector Entity::DirectionLocalToWorld(const FVector& localDir)
	{
		if (Actor())
			return KBEMath::DirectionLocalToWorld(Actor()->GetActorRotation().Euler(), localDir);
		else
			return KBEMath::DirectionLocalToWorld(direction_, localDir);
	}
	
	FVector Entity::DirectionWorldToLocal(const FVector& worldDir)
	{
		if (Actor())
			return KBEMath::DirectionWorldToLocal(Actor()->GetActorRotation().Euler(), worldDir);
		else
			return KBEMath::DirectionWorldToLocal(direction_, worldDir);
	}

}
