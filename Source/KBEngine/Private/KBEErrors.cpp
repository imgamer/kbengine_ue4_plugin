#pragma once

#include "KBEnginePrivatePCH.h"
#include "KBEErrors.h"

namespace KBEngine
{
	TMap<int32, KBEErrors::Error> KBEErrors::errors_;;
	bool KBEErrors::serverErrorsDescrImported_ = false;

	void KBEErrors::Clear()
	{
		errors_.Reset();
		serverErrorsDescrImported_ = false;
	}

	void KBEErrors::InitLocalErrors()
	{
		static KBEErrors::Error s_localErr_[] = {
			{ (int32)ERROR_TYPE::CONNECT_TO_LOGINAPP_FAULT, TEXT("CONNECT_TO_LOGINAPP_FAULT"), TEXT("无法连接到登录服务器") },
			{ (int32)ERROR_TYPE::CONNECT_TO_BASEAPP_FAULT,  TEXT("CONNECT_TO_BASEAPP_FAULT"), TEXT("无法连接到网关服务器") },
			{ (int32)ERROR_TYPE::VERSION_NOT_MATCH,         TEXT("VERSION_NOT_MATCH"), TEXT("客户端与服务器的版本不匹配") },
			{ (int32)ERROR_TYPE::SCRIPT_VERSION_NOT_MATCH,  TEXT("SCRIPT_VERSION_NOT_MATCH"), TEXT("客户端的与服务器的代码版本不匹配") },
			{ (int32)ERROR_TYPE::INVALID_NETWORK,           TEXT("INVALID_NETWORK"), TEXT("无网络连接") },
			{ (int32)ERROR_TYPE::LOSE_SERVER_CONNECTED ,    TEXT("LOSE_SERVER_CONNECTED"), TEXT("与服务器的连断被断开") },

			{ 0, TEXT(""), TEXT("") }  // end of value
		};

		int i = 0;
		while (true)
		{
			auto st = s_localErr_[i++];
			if (st.id == 0)
				break;

			errors_.Add(st.id, st);

			KBE_DEBUG(TEXT("KBEErrors::InitLocalErrors: Local: id=%d, name=%s, descr=%s"), st.id, *st.name, *st.descr);
		}
	}

	bool KBEErrors::ImportServerErrorsDescr(MemoryStream &stream)
	{
		// @TODO(penghuawei): 这里当前没有对数据流的有效性进行较验，
		// 所以如果数据流来自本地且非法，就将有可能导致客户端出现未知的问题

		uint16 size = stream.ReadUint16();
		while (size > 0)
		{
			size -= 1;

			KBEErrors::Error e;
			e.id = stream.ReadUint16();
			e.name = stream.ReadUTF8();
			e.descr = stream.ReadUTF8();

			errors_.Add(e.id, e);

			KBE_DEBUG(TEXT("KBEngineApp::OnImportServerErrorsDescr: Server: id=%d, name=%s, descr=%s"), e.id, *e.name, *e.descr);
		}

		serverErrorsDescrImported_ = true;
		return true;
	}

	FString KBEErrors::ErrorName(int32 id)
	{
		auto* e = errors_.Find(id);
		if (!e)
			return FString::Printf(TEXT("Unknown error code(%d)"), id);

		return e->name;
	}

	FString KBEErrors::ErrorDesc(int32 id)
	{
		auto* e = errors_.Find(id);
		if (!e)
			return FString::Printf(TEXT("Unknown error code(%d)"), id);

		return e->descr;
	}
}
