#pragma once

#include "Core.h"
#include "MemoryStream.h"

namespace KBEngine
{

	enum class ERROR_TYPE
	{
		SUCCESS = 0,                     // 成功
		CONNECT_TO_LOGINAPP_FAULT = -1,  // 无法连接到登录服务器
		CONNECT_TO_BASEAPP_FAULT = -2,   // 无法连接到网关服务器
		VERSION_NOT_MATCH = -3,          // 客户端与服务器的版本不匹配
		SCRIPT_VERSION_NOT_MATCH = -4,   // 客户端的与服务器的代码版本不匹配
		INVALID_NETWORK = -5,            // 无网络连接
		LOSE_SERVER_CONNECTED = -6,      // 失去与服务器的连接
	};

	class KBENGINE_API KBEErrors
	{
		// 描述服务端返回或客户端本地的错误信息
		typedef struct
		{
			int32 id;
			FString name;
			FString descr;
		}Error;

	public:
		static bool ImportServerErrorsDescr(MemoryStream &stream);
		static FString ErrorName(int32 errcode);
		static FString ErrorDesc(int32 errcode);
		static bool ServerErrorsDescrImported() { return serverErrorsDescrImported_; }
		static void Clear();
		static void InitLocalErrors();

	private:

	private:
		/* 所有服务端以及客户端错误码对应的错误描述,
		服务器的错误请参考：kbe/src/lib/server/server_errors.h
		客户端错误请参考：ERROR_TYPE，
		由于客户端和服务器的错误都在一个列表中，因此不使用uint16，改为int32;
		小于0的则是客户端错误，大于0的为服务器错误。
		等于0表示成功；
		*/
		static TMap<int32, Error> errors_;

		// 是否已初始化过？
		static bool serverErrorsDescrImported_;
	};

}