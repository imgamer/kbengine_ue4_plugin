#include "PersistentInfos.h"
#include "KBEnginePrivatePCH.h"
#include "KBEngineApp.h"

namespace KBEngine
{
	PersistentInofs::PersistentInofs(const FString& path, const FString& clientVersion, const FString& clientScriptVersion, const FString& loginappHost, uint16 loginappPort)
		: persistentDataPath_(path),
		clientVersion_(clientVersion),
		clientScriptVersion_(clientScriptVersion),
		loginappHost_(loginappHost),
		loginappPort_(loginappPort)
	{
		InitDigest();
	}

	FString PersistentInofs::GetSuffix()
	{
		return FString::Printf(TEXT("%s.%s.%s.%d"), *clientVersion_, *clientScriptVersion_, *loginappHost_, loginappPort_);
	}

	void PersistentInofs::InitDigest()
	{
		MemoryStream kbengine_digest;
		
		if (LoadFile(kbengine_digest, prefix_loginapp_digest + GetSuffix()))
			loginapp_digest_ = kbengine_digest.ReadString();

		kbengine_digest.Clear();
		if (LoadFile(kbengine_digest, prefix_baseapp_digest + GetSuffix()))
			baseapp_digest_ = kbengine_digest.ReadString();
	}

	bool PersistentInofs::LoadServerErrorsDescr(MemoryStream& out)
	{
		if (!loginapp_is_match_)
			return false;

		if (!LoadFile(out, prefix_server_err_descr + GetSuffixLoginapp()))
			return false;

		FString digest = out.ReadString();
		if (digest != loginapp_digest_)
			return false;
		return true;
	}

	bool PersistentInofs::LoadEntityDef(MemoryStream& out)
	{
		if (!baseapp_is_match_)
			return false;

		if (!LoadFile(out, prefix_entity_def + GetSuffixBaseapp()))
			return false;

		FString digest = out.ReadString();
		if (digest != baseapp_digest_)
			return false;
		return true;
	}

	bool PersistentInofs::LoadBaseappMessages(MemoryStream& out)
	{
		if (!baseapp_is_match_)
			return false;

		if (!LoadFile(out, prefix_baseapp_messages + GetSuffixBaseapp()))
			return false;

		FString digest = out.ReadString();
		if (digest != baseapp_digest_)
			return false;
		return true;
	}

	bool PersistentInofs::LoadLoginappMessages(MemoryStream& out)
	{
		if (!loginapp_is_match_)
			return false;

		if (!LoadFile(out, prefix_loginapp_messages + GetSuffixLoginapp()))
			return false;

		FString digest = out.ReadString();
		if (digest != loginapp_digest_)
			return false;
		return true;
	}



	void PersistentInofs::WriteLoginappMessages(MemoryStream &stream)
	{
		MemoryStream inStream;
		inStream.WriteString(loginapp_digest_);
		inStream.Append(&stream.Data()[stream.RPos()], stream.Length());
		WriteFile(prefix_loginapp_messages + GetSuffixLoginapp(), inStream);
	}

	void PersistentInofs::WriteBaseappMessages(MemoryStream &stream)
	{
		MemoryStream inStream;
		inStream.WriteString(baseapp_digest_);
		inStream.Append(&stream.Data()[stream.RPos()], stream.Length());
		WriteFile(prefix_baseapp_messages + GetSuffixBaseapp(), inStream);
	}

	void PersistentInofs::WriteServerErrorsDescr(MemoryStream &stream)
	{
		MemoryStream inStream;
		inStream.WriteString(loginapp_digest_);
		inStream.Append(&stream.Data()[stream.RPos()], stream.Length());
		WriteFile(prefix_server_err_descr + GetSuffixLoginapp(), inStream);
	}

	void PersistentInofs::WriteEntityDef(MemoryStream &stream)
	{
		MemoryStream inStream;
		inStream.WriteString(baseapp_digest_);
		inStream.Append(&stream.Data()[stream.RPos()], stream.Length());
		WriteFile(prefix_entity_def + GetSuffixBaseapp(), inStream);
	}

	bool PersistentInofs::OnServerDigest(SERVER_APP_TYPE fromApp, const FString &serverProtocolMD5, const FString &serverEntitydefMD5)
	{
		FString remoteDigest = serverProtocolMD5 + serverEntitydefMD5;
		FString prefix = TEXT("");
		FString localDigest = TEXT("");
		if (fromApp == SERVER_APP_TYPE::LoginApp)
		{
			if (loginapp_digest_ == remoteDigest)
			{
				loginapp_is_match_ = true;
				return loginapp_is_match_;
			}

			localDigest = loginapp_digest_;
			loginapp_digest_ = remoteDigest;
			prefix = prefix_loginapp_digest;
			ClearLoginappMessageFiles();
		}
		else
		{
			if (baseapp_digest_ == remoteDigest)
			{
				baseapp_is_match_ = true;
				return baseapp_is_match_;
			}

			localDigest = baseapp_digest_;
			baseapp_digest_ = remoteDigest;
			prefix = prefix_baseapp_digest;
			ClearBaseappMessageFiles();
		}

		KBE_DEBUG(TEXT("PersistentInofs::OnServerDigest: local(%s) digest(%s) not match remote(%s), will reimport message from remote."), *prefix, *localDigest, *remoteDigest);

		MemoryStream stream;
		stream.WriteString(remoteDigest);
		WriteFile(prefix + GetSuffix(), stream);
		return false;
	}

	void PersistentInofs::ClearAllMessageFiles()
	{
		ClearLoginappMessageFiles();
		ClearBaseappMessageFiles();
	}

	void PersistentInofs::ClearLoginappMessageFiles()
	{
		DeleteFile(prefix_loginapp_digest + GetSuffix());
		DeleteFile(prefix_loginapp_messages + GetSuffixLoginapp());
		DeleteFile(prefix_server_err_descr + GetSuffixLoginapp());
	}

	void PersistentInofs::ClearBaseappMessageFiles()
	{
		DeleteFile(prefix_baseapp_digest + GetSuffix());
		DeleteFile(prefix_baseapp_messages + GetSuffixBaseapp());
		DeleteFile(prefix_entity_def + GetSuffixBaseapp());
	}

	void PersistentInofs::WriteFile(const FString &name, MemoryStream &stream)
	{
		KBE_DEBUG(TEXT("PersistentInofs::WriteFile: %s"), *(persistentDataPath_ + "/" + name));
		DeleteFile(name);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.CreateDirectoryTree(*persistentDataPath_))
		{
			KBE_WARNING(TEXT("PersistentInofs::WriteFile: create directory(%s) fault!"), *persistentDataPath_);
			return;
		}

		IFileHandle *file = PlatformFile.OpenWrite(*(persistentDataPath_ + "/" + name));
		if (!file)
		{
			KBE_WARNING(TEXT("PersistentInofs::WriteFile: create file '%s' from '%s' fault!"), *name, *persistentDataPath_);
			return;
		}

		file->Write(&(stream.Data()[stream.RPos()]), stream.Length());
		delete file;
	}

	bool PersistentInofs::LoadFile(MemoryStream& out, const FString &name)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileHandle *file = PlatformFile.OpenRead(*(persistentDataPath_ + "/" + name));
		if (!file)
		{
			KBE_ERROR(TEXT("PersistentInofs::LoadFile: %s/%s, error!"), *persistentDataPath_, *name);
			return false;
		}

		file->SeekFromEnd();
		int32 len = (int32)file->Tell();
		file->Seek(0);

		TArray<uint8> datas;
		datas.SetNumUninitialized(len);
		file->Read(datas.GetData(), len);
		delete file;

		KBE_INFO(TEXT("PersistentInofs::LoadFile: %s/%s, datasize=%d"), *persistentDataPath_, *name, len);

		out.Append(datas.GetData(), datas.Num());
		return true;
	}

	void PersistentInofs::DeleteFile(const FString &name)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		KBE_INFO(TEXT("PersistentInofs::DeleteFile: %s/%s"), *persistentDataPath_, *name);
		PlatformFile.DeleteFile(*(persistentDataPath_ + "/" + name));
	}

}
