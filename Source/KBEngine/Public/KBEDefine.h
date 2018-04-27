#pragma once

#include "Core.h"
#include "KBEDebug.h"

namespace KBEngine
{
	class Entity;
	class MemoryStream;

	typedef uint16 MessageID;
	typedef TArray<FVariant> FVariantArray;
	typedef TMap<FString, FVariant> FVariantMap;

	typedef void(*MessageHandler)(void *, TArray<FVariant> &);


	typedef void(*PropertyHandler)(Entity *, FVariant &);

#define SAFE_DELETE(obj) if (obj) { delete obj; obj = nullptr; }



	// 客户端的类别
	// http://www.kbengine.org/docs/programming/clientsdkprogramming.html
	// http://www.kbengine.org/cn/docs/programming/clientsdkprogramming.html
	enum class CLIENT_TYPE : int8
	{
		// Mobile(Phone, Pad)
		CLIENT_TYPE_MOBILE = 1,

		// Windows Application program
		CLIENT_TYPE_WIN = 2,

		// Linux Application program
		CLIENT_TYPE_LINUX = 3,

		// Mac Application program
		CLIENT_TYPE_MAC = 4,

		// Web，HTML5，Flash
		CLIENT_TYPE_BROWSER = 5,

		// bots
		CLIENT_TYPE_BOTS = 6,

		// Mini-Client
		CLIENT_TYPE_MINI = 7,
	};

	enum class SERVER_APP_TYPE : int8
	{
		LoginApp = 0,
		BaseApp,
	};

	namespace EKBEVariantTypes
	{
		const int VariantArray = 0x8000;
		const int VariantMap = 0x8001;
		const int EntityPtr = 0x8002;
		const int MemoryStreamPtr = 0x8003;
	}

	template<> struct TVariantTraits < FVariantArray >
	{
		static int32 GetType()
		{
			return EKBEVariantTypes::VariantArray;
		}
	};

	template<> struct TVariantTraits < FVariantMap >
	{
		static int32 GetType()
		{
			return EKBEVariantTypes::VariantMap;
		}
	};

	template<> struct TVariantTraits < MemoryStream * >
	{
		static int32 GetType()
		{
			return EKBEVariantTypes::MemoryStreamPtr;
		}
	};

	template<> struct TVariantTraits < Entity * >
	{
		static int32 GetType()
		{
			return EKBEVariantTypes::EntityPtr;
		}
	};

	inline FMemoryWriter &operator << (FMemoryWriter &mem, MemoryStream *pMemoryStream)
	{
		mem.Serialize(&pMemoryStream, sizeof(pMemoryStream));
		return mem;
	}

	inline FMemoryWriter &operator << (FMemoryWriter &mem, Entity *pEntity)
	{
		mem.Serialize(&pEntity, sizeof(pEntity));
		return mem;
	}

}
