#pragma once

#include "MemoryStream.h"
#include "KBEDefine.h"

namespace KBEngine
{
	class KBENGINE_API PersistentInofs
	{
	public:
		PersistentInofs(const FString& path, const FString& clientVersion, const FString& clientScriptVersion, const FString& loginappHost, uint16 loginappPort);

		void InitDigest();

		bool OnServerDigest(SERVER_APP_TYPE fromApp, const FString &serverProtocolMD5, const FString &serverEntitydefMD5);

		bool LoadServerErrorsDescr(MemoryStream& out);
		bool LoadEntityDef(MemoryStream& out);
		bool LoadBaseappMessages(MemoryStream& out);
		bool LoadLoginappMessages(MemoryStream& out);

		void WriteServerErrorsDescr(MemoryStream &stream);
		void WriteEntityDef(MemoryStream &stream);
		void WriteBaseappMessages(MemoryStream &stream);
		void WriteLoginappMessages(MemoryStream &stream);

		void ClearAllMessageFiles();
		void ClearLoginappMessageFiles();
		void ClearBaseappMessageFiles();

	private:
		const FString prefix_loginapp_digest = TEXT("kbengine.digest.loginapp.");
		const FString prefix_loginapp_messages = TEXT("loginapp_clientMessages.");
		const FString prefix_baseapp_digest = TEXT("kbengine.digest.baseapp.");
		const FString prefix_baseapp_messages = TEXT("baseapp_clientMessages.");
		const FString prefix_server_err_descr = TEXT("serverErrorsDescr.");
		const FString prefix_entity_def = TEXT("clientEntityDef.");


		FString GetSuffix();
		FString GetSuffixBaseapp() { return baseapp_digest_ + TEXT(".") + GetSuffix(); }
		FString GetSuffixLoginapp() { return loginapp_digest_ + TEXT(".") + GetSuffix(); }

		void WriteFile(const FString &name, MemoryStream &datas);
		bool LoadFile(MemoryStream& out, const FString &name);
		void DeleteFile(const FString &name);

	private:
		FString persistentDataPath_;
		FString clientVersion_;
		FString clientScriptVersion_;
		FString loginappHost_;
		uint16 loginappPort_ = 0;

		// 服务器与本地的digest是否匹配
		bool loginapp_is_match_ = false;
		bool baseapp_is_match_ = false;
		FString loginapp_digest_;
		FString baseapp_digest_;

	};
}
