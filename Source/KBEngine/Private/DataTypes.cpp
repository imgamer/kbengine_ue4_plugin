#include "DataTypes.h"
#include "KBEnginePrivatePCH.h"
#include "EntityDef.h"

namespace KBEngine
{
	float KBEDATATYPE_ARRAY::KBE_FLT_MAX = 3.40282e+038f;

	bool KBEDATATYPE_BASE::IsNumeric(const FVariant &v)
	{
		switch (v.GetType())
		{
		case EVariantTypes::Int8:
		case EVariantTypes::UInt8:
		case EVariantTypes::Int16:
		case EVariantTypes::UInt16:
		case EVariantTypes::Int32:
		case EVariantTypes::UInt32:
		case EVariantTypes::Int64:
		case EVariantTypes::UInt64:
		case EVariantTypes::Float:
		case EVariantTypes::Double:
		case EVariantTypes::Ansichar:
		case EVariantTypes::Widechar:
		case EVariantTypes::Bool:
			return true;
		}
		return false;
	}

	double KBEDATATYPE_BASE::ToNumber(const FVariant &v)
	{
		switch (v.GetType())
		{
		case EVariantTypes::Int8:
			return v.GetValue<int8>();
		case EVariantTypes::UInt8:
			return v.GetValue<uint8>();
		case EVariantTypes::Int16:
			return v.GetValue<int16>();
		case EVariantTypes::UInt16:
			return v.GetValue<uint16>();
		case EVariantTypes::Int32:
			return v.GetValue<int32>();
		case EVariantTypes::UInt32:
			return v.GetValue<uint32>();
		case EVariantTypes::Int64:
			return v.GetValue<int64>();
		case EVariantTypes::UInt64:
			return v.GetValue<uint64>();
		case EVariantTypes::Float:
			return v.GetValue<float>();
		case EVariantTypes::Double:
			return v.GetValue<double>();
			//
		case EVariantTypes::Ansichar:
			return v.GetValue<ANSICHAR>();
		case EVariantTypes::Widechar:
			return v.GetValue<WIDECHAR>();
		case EVariantTypes::Bool:
			return v.GetValue<bool>();
		}
		return 0;
	}


	void KBEDATATYPE_ARRAY::Bind()
	{
		if (vtypeObject_)
		{
			vtypeObject_->Bind();
		}
		else
		{
			vtypeObject_ = EntityDef::GetDataType(vtype_);
		}
	}

	FVariant KBEDATATYPE_ARRAY::CreateFromStream(MemoryStream *stream)
	{
		KBE_ASSERT(vtypeObject_);

		uint32 size = stream->ReadUint32();
		FVariantArray data;

		while (size > 0)
		{
			size--;
			data.Add(vtypeObject_->CreateFromStream(stream));
		};

		return data;
	}

	void KBEDATATYPE_ARRAY::AddToStream(Bundle *stream, const FVariant &v)
	{
		KBE_ASSERT(vtypeObject_);

		const auto a = v.GetValue<FVariantArray>();
		stream->WriteUint32((uint32)a.Num());
		for (int i = 0; i<a.Num(); i++)
		{
			vtypeObject_->AddToStream(stream, a[i]);
		}
	}

	FVariant KBEDATATYPE_ARRAY::ParseDefaultValStr(const FString& s)
	{
		return FVariantArray();
	}

	bool KBEDATATYPE_ARRAY::IsSameType(const FVariant &v)
	{
		if (!vtypeObject_)
		{
			KBE_ERROR(TEXT("KBEDATATYPE_ARRAY::IsSameType: has not Bind! baseType=%d"), vtype_);
			return false;
		}

		if (v.GetType() != static_cast<EVariantTypes>(EKBEVariantTypes::VariantArray))
		{
			return false;
		}

		const auto a = v.GetValue<FVariantArray>();
		for (int i = 0; i<a.Num(); i++)
		{
			if (!vtypeObject_->IsSameType(a[i]))
			{
				return false;
			}
		}

		return true;
	}

	void KBEDATATYPE_FIXED_DICT::Bind()
	{
		for (auto it = dictType_.CreateIterator(); it; ++it)
		{
			FString itemkey = it.Key();

			auto** p = dictTypeObjects_.Find(itemkey);
			KBEDATATYPE_BASE *typeObject = p ? *p : nullptr;
			if (typeObject)
			{
				typeObject->Bind();
			}
			else
			{
				typeObject = EntityDef::GetDataType(it.Value());
				if (typeObject)
					dictTypeObjects_.Add(itemkey, typeObject);
			}
		}
	}

	FVariant KBEDATATYPE_FIXED_DICT::CreateFromStream(MemoryStream *stream)
	{
		FVariantMap data;

		KBE_ASSERT(dictTypeObjects_.Num());
		for (auto it = dictTypeObjects_.CreateIterator(); it; ++it)
		{
			FString itemkey = it.Key();
			KBEDATATYPE_BASE *typeObject = it.Value();
			check(typeObject);

			data.Add(itemkey, typeObject->CreateFromStream(stream));
		}

		return data;
	}

	void KBEDATATYPE_FIXED_DICT::AddToStream(Bundle *stream, const FVariant &v)
	{
		const auto data = v.GetValue<FVariantMap>();

		KBE_ASSERT(dictTypeObjects_.Num());
		for (auto it = dictTypeObjects_.CreateIterator(); it; ++it)
		{
			FString itemkey = it.Key();
			KBEDATATYPE_BASE *typeObject = it.Value();
			check(typeObject);

			typeObject->AddToStream(stream, data[itemkey]);
		}
	}

	FVariant KBEDATATYPE_FIXED_DICT::ParseDefaultValStr(const FString& s)
	{
		FVariantMap data;

		KBE_ASSERT(dictTypeObjects_.Num());
		for (auto it = dictTypeObjects_.CreateIterator(); it; ++it)
		{
			FString itemkey = it.Key();
			KBEDATATYPE_BASE *typeObject = it.Value();
			check(typeObject);

			data.Add(itemkey, typeObject->ParseDefaultValStr(FString()));
		}

		return data;
	}
	
	bool KBEDATATYPE_FIXED_DICT::IsSameType(const FVariant &v)
	{
		if (v.GetType() != static_cast<EVariantTypes>(EKBEVariantTypes::VariantMap))
			return false;

		const auto data = v.GetValue<FVariantMap>();

		KBE_ASSERT(dictTypeObjects_.Num());
		for (auto it = dictTypeObjects_.CreateIterator(); it; ++it)
		{
			FString itemkey = it.Key();
			KBEDATATYPE_BASE *typeObject = it.Value();
			check(typeObject);

			auto* value = data.Find(itemkey);
			if (value)
			{
				if (!typeObject->IsSameType(*value))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		return true;
	}

}
