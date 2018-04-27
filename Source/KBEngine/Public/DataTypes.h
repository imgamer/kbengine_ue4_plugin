#pragma once

#include "KBEDebug.h"
#include "MemoryStream.h"
#include "Bundle.h"

namespace KBEngine
{
	class KBENGINE_API KBEDATATYPE_BASE
	{
	public:
		static float KBE_FLT_MAX;

		static bool IsNumeric(const FVariant &v);
		static double ToNumber(const FVariant &v);

		virtual const TCHAR *TypeString() const = 0;

		virtual void Bind() {}
		virtual FVariant CreateFromStream(MemoryStream *stream) = 0;
		virtual void AddToStream(Bundle *stream, const FVariant &v) = 0;
		virtual FVariant ParseDefaultValStr(const FString& s) = 0;
		virtual bool IsSameType(const FVariant &v) = 0;
	};

	class KBENGINE_API KBEDATATYPE_INT8 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_INT8");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadInt8());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteInt8(v.GetValue<int8>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (int8)FCString::Atoi(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= INT8_MIN && n <= INT8_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_INT16 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_INT16");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadInt16());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteInt16(v.GetValue<int16>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (int16)FCString::Atoi(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= INT16_MIN && n <= INT16_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_INT32 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_INT32");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadInt32());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteInt32(v.GetValue<int32>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (int32)FCString::Atoi(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= INT32_MIN && n <= INT32_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_INT64 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_INT64");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadInt64());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteInt64(v.GetValue<int64>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (int64)FCString::Atoi64(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= INT64_MIN && n <= INT64_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_UINT8 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_UINT8");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadUint8());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteUint8(v.GetValue<uint8>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (uint8)FCString::Atoi(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= 0 && n <= UINT8_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_UINT16 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_UINT16");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadUint16());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteUint16(v.GetValue<uint16>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (uint16)FCString::Atoi(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= 0 && n <= UINT16_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_UINT32 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_UINT32");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadUint32());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteUint32(v.GetValue<uint32>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (uint32)FCString::Atoi64(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= 0 && n <= UINT32_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_UINT64 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_UINT64");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadUint64());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteUint64(v.GetValue<uint64>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			TCHAR *pEnd = nullptr;
			return (uint64)FCString::Strtoui64(*s, &pEnd, 10);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (!KBEDATATYPE_BASE::IsNumeric(v))
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= 0 && n <= UINT64_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_FLOAT : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_FLOAT");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadFloat());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteFloat(v.GetValue<float>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (float)FCString::Atof(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (v.GetType() != EVariantTypes::Float &&
				v.GetType() != EVariantTypes::Double)
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= FLT_MIN && n <= FLT_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_DOUBLE : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_DOUBLE");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadDouble());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteDouble(v.GetValue<double>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return (double)FCString::Atod(*s);
		}

		bool IsSameType(const FVariant &v) override
		{
			if (v.GetType()!= EVariantTypes::Float && 
				v.GetType()!= EVariantTypes::Double)
				return false;

			double n = KBEDATATYPE_BASE::ToNumber(v);
			return n >= DBL_MIN && n <= DBL_MAX;
		}
	};

	class KBENGINE_API KBEDATATYPE_STRING : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_STRING");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(FString(stream->ReadString()));
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteString(v.GetValue<FString>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return s;
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::String;
		}
	};

	class KBENGINE_API KBEDATATYPE_VECTOR2 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_VECTOR2");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			FVector2D vec;
			vec.X = stream->ReadFloat();
			vec.Y = stream->ReadFloat();
			return FVariant(vec);
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			FVector2D vec = v.GetValue<FVector2D>();
			stream->WriteFloat(vec.X);
			stream->WriteFloat(vec.Y);
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return FVector2D(0, 0);
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::Vector2d;
		}
	};

	class KBENGINE_API KBEDATATYPE_VECTOR3 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_VECTOR3");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			FVector vec;
			vec.X = stream->ReadFloat();
			vec.Y = stream->ReadFloat();
			vec.Z = stream->ReadFloat();
			return FVariant(vec);
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			FVector vec = v.GetValue<FVector>();
			stream->WriteFloat(vec.X);
			stream->WriteFloat(vec.Y);
			stream->WriteFloat(vec.Z);
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return FVariant(FVector(0, 0, 0));
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::Vector;
		}
	};

	class KBENGINE_API KBEDATATYPE_VECTOR4 : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_VECTOR4");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			FVector4 vec;
			vec.X = stream->ReadFloat();
			vec.Y = stream->ReadFloat();
			vec.Z = stream->ReadFloat();
			vec.W = stream->ReadFloat();
			return FVariant(vec);
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			FVector4 vec = v.GetValue<FVector4>();
			stream->WriteFloat(vec.X);
			stream->WriteFloat(vec.Y);
			stream->WriteFloat(vec.Z);
			stream->WriteFloat(vec.W);
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return FVector4(0, 0, 0, 0);
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::Vector4;
		}
	};

	class KBENGINE_API KBEDATATYPE_BYTEARRAY : public KBEDATATYPE_BASE
	{
	public:
		FVariant CreateFromStream(MemoryStream *stream) override
		{
			TArray<uint8> bytes;
			stream->ReadBlob(bytes);
			return bytes;
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteBlob(v.GetValue< TArray<uint8> >());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return TArray<uint8>();
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::ByteArray;
		}
	};

	class KBENGINE_API KBEDATATYPE_PYTHON : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_PYTHON");
		}
	};

	class KBENGINE_API KBEDATATYPE_PY_DICT : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_PY_DICT");
		}
	};

	class KBENGINE_API KBEDATATYPE_PY_TUPLE : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_PY_TUPLE");
		}
	};

	class KBENGINE_API KBEDATATYPE_PY_LIST : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_PY_LIST");
		}
	};

	class KBENGINE_API KBEDATATYPE_UNICODE : public KBEDATATYPE_BASE
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_UNICODE");
		}

		FVariant CreateFromStream(MemoryStream *stream) override
		{
			return FVariant(stream->ReadUTF8());
		}

		void AddToStream(Bundle *stream, const FVariant &v) override
		{
			stream->WriteUTF8(v.GetValue<FString>());
		}

		FVariant ParseDefaultValStr(const FString& s) override
		{
			return s;
		}

		bool IsSameType(const FVariant &v) override
		{
			return v.GetType() == EVariantTypes::String;
		}
	};

	class KBENGINE_API KBEDATATYPE_MAILBOX : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_MAILBOX");
		}
	};

	class KBENGINE_API KBEDATATYPE_BLOB : public KBEDATATYPE_BYTEARRAY
	{
	public:
		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_BLOB");
		}
	};

	class KBENGINE_API KBEDATATYPE_ARRAY : public KBEDATATYPE_BASE
	{
	public:
		KBEDATATYPE_ARRAY(uint16 vt)
			: vtype_(vt)
		{
		}

		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_ARRAY");
		}

		void Bind() override;
		FVariant CreateFromStream(MemoryStream *stream) override;
		void AddToStream(Bundle *stream, const FVariant &v) override;
		FVariant ParseDefaultValStr(const FString& s) override;
		bool IsSameType(const FVariant &v) override;

	private:
		uint16 vtype_;
		KBEDATATYPE_BASE *vtypeObject_ = nullptr;
	};

	class KBENGINE_API KBEDATATYPE_FIXED_DICT : public KBEDATATYPE_BASE
	{
	public:
		KBEDATATYPE_FIXED_DICT(const FString& implementedBy) :
			implementedBy_(implementedBy)
		{
		}

		virtual ~KBEDATATYPE_FIXED_DICT()
		{}

		virtual const TCHAR *TypeString() const override
		{
			return TEXT("KBEDATATYPE_FIXED_DICT");
		}

		void Bind() override;
		FVariant CreateFromStream(MemoryStream *stream) override;
		void AddToStream(Bundle *stream, const FVariant &v) override;
		FVariant ParseDefaultValStr(const FString& s) override;
		bool IsSameType(const FVariant &v) override;

		void AddSubType(const FString& key, uint16 type) { dictType_.Add(key, type); }

	private:
		FString implementedBy_;
		TMap<FString, uint16> dictType_;
		TMap<FString, KBEDATATYPE_BASE *> dictTypeObjects_;
	};
}
