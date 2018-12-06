#include "LoginApp.h"
#include "KBEnginePrivatePCH.h"
#include "KBEngineApp.h"
#include "NetworkInterface.h"
#include "KBEDebug.h"
#include "Bundle.h"
#include "KBEPersonality.h"
#include "KBEErrors.h"

namespace KBEngine
{
	LoginApp::LoginApp(KBEngineApp* app)
		: app_(app)
	{
		KBE_ASSERT(app_);
		messages_ = app->pMessages();
		messageReader_ = new MessageReader(this, messages_, KBEngineApp::app->GetRecvBufferMax());

	}

	LoginApp::~LoginApp()
	{
		KBE_DEBUG(TEXT("LoginApp::~LoginApp()"));
		ClearNetwork();
		SAFE_DELETE(messageReader_);
	}

	bool LoginApp::NetworkIsValid()
	{
		return networkInterface_ && networkInterface_->Valid();
	}

	void LoginApp::SendTick()
	{
		if (!messages_->LoginappMessageImported() || app_->TickInterval() == 0)
			return;

		auto span = FDateTime::UtcNow() - lastTicktime_;

		if (span.GetTotalSeconds() >= app_->TickInterval())
		{
			// phw: 心跳判断代码需要kbe 0.8.10以上版本的服务器支持
			span = lastTickCBTime_ - lastTicktime_;

			// 如果心跳回调接收时间小于心跳发送时间，说明没有收到回调
			// 此时应该通知客户端掉线了
			if (span.GetTotalSeconds() < 0)
			{
				KBE_ERROR(TEXT("LoginApp::SendTick: Receive appTick timeout!"));
				networkInterface_->Close();
				return;
			}

			const Message* Loginapp_onClientActiveTickMsg = NULL;

			Loginapp_onClientActiveTickMsg = messages_->GetMessage("Loginapp_onClientActiveTick");

			if (Loginapp_onClientActiveTickMsg != NULL)
			{
				Bundle* bundle = new Bundle();
				bundle->NewMessage(messages_->GetMessage("Loginapp_onClientActiveTick"));
				bundle->Send(networkInterface_);
				delete bundle;
			}

			lastTicktime_ = FDateTime::UtcNow();
		}
	}

	void LoginApp::OnLoseConnect()
	{
	}

	void LoginApp::Connect(const FString& host, uint16 port, ConnectCallbackFunc func)
	{
		KBE_ASSERT(!networkInterface_);
		host_ = host;
		port_ = port;
		connectedCallbackFunc_ = func;
		networkInterface_ = new NetworkInterface(messageReader_);
		networkInterface_->ConnectTo(host, port, std::bind(&LoginApp::OnConnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void LoginApp::Disconnect()
	{
		host_ = TEXT("");
		port_ = 0;
		connectedCallbackFunc_ = nullptr;

		ClearNetwork();
	}

	void LoginApp::ClearNetwork()
	{
		if (networkInterface_)
		{
			networkInterface_->Close();
			delete networkInterface_;
			networkInterface_ = nullptr;
		}
	}

	void LoginApp::OnConnected(const FString& host, uint16 port, bool success)
	{
		if (!success)
		{
			KBE_ERROR(TEXT("LoginApp::OnConnected(): connect %s:%u is error!"), *host, port);
			if (connectedCallbackFunc_)
				connectedCallbackFunc_((int)ERROR_TYPE::CONNECT_TO_LOGINAPP_FAULT);
			Disconnect();
			return;
		}

		KBE_DEBUG(TEXT("LoginApp::OnConnected(): connect %s:%u is success!"), *host, port);

		CmdHello();
	}

	void LoginApp::CmdHello()
	{
		KBE_DEBUG(TEXT("LoginApp::CmdHello: send Loginapp_hello ..."));
		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_hello"));
		bundle->WriteString(app_->ClientVersion());
		bundle->WriteString(app_->ClientScriptVersion());
		bundle->WriteBlob(app_->EncryptedKey());
		bundle->Send(networkInterface_);
		delete bundle;
	}

	void LoginApp::Client_onHelloCB(MemoryStream &stream)
	{
		FString serverVersion = stream.ReadString();
		FString serverScriptVersion = stream.ReadString();
		FString serverProtocolMD5 = stream.ReadString();
		FString serverEntitydefMD5 = stream.ReadString();
		int32 ctype = stream.ReadInt32();

		KBE_DEBUG(TEXT("LoginApp::Client_onHelloCB: verInfo(%s), scriptVersion(%s), srvProtocolMD5(%s), srvEntitydefMD5(%s), + ctype(%d)!"),
			*serverVersion, *serverScriptVersion, *serverProtocolMD5, *serverEntitydefMD5, ctype);

		messages_->Reset();
		KBEErrors::Clear();

		bool digestMatch = false;
		if (app_->pPersistentInofs())
			digestMatch = app_->pPersistentInofs()->OnServerDigest(SERVER_APP_TYPE::LoginApp, serverProtocolMD5, serverEntitydefMD5);

		KBE_DEBUG(TEXT("LoginApp::Client_onHelloCB: digest match(%s)"), digestMatch ? TEXT("true") : TEXT("false"));
		bool success = digestMatch;
		if (success)
		{
			// 尝试加载本地消息定义
			MemoryStream out;
			success = app_->pPersistentInofs()->LoadLoginappMessages(out);
			if (success)
				success = messages_->ImportMessagesFromStream(out, SERVER_APP_TYPE::LoginApp);
	
			// 尝试加载本地错误定义
			out.Clear();
			if (success)
				success = app_->pPersistentInofs()->LoadServerErrorsDescr(out);

			if (success)
				success = KBEErrors::ImportServerErrorsDescr(out);
		}

		// 加载失败则向服务器请求
		if (!digestMatch || !success)
		{
			CmdImportClientMessages();
		}
		else
		{
			// 如果两种数据都加载完成，则可以结束登录流程
			connectedCallbackFunc_((int)ERROR_TYPE::SUCCESS);
		}
	}

	void LoginApp::Client_onVersionNotMatch(MemoryStream &stream)
	{
		auto serverVersion = stream.ReadString();

		KBE_ERROR(TEXT("LoginApp::Client_onVersionNotMatch: verInfo=%s, server=%s"), *app_->ClientVersion(), *serverVersion);
		if (app_->pPersistentInofs())
			app_->pPersistentInofs()->ClearAllMessageFiles();

		if (connectedCallbackFunc_)
			connectedCallbackFunc_((int)ERROR_TYPE::VERSION_NOT_MATCH);
		Disconnect();
	}

	void LoginApp::Client_onScriptVersionNotMatch(MemoryStream &stream)
	{
		auto serverScriptVersion = stream.ReadString();

		KBE_ERROR(TEXT("LoginApp::Client_onScriptVersionNotMatch: verInfo=%s, server=%s"), *app_->ClientScriptVersion(), *serverScriptVersion);
		if (app_->pPersistentInofs())
			app_->pPersistentInofs()->ClearAllMessageFiles();

		if (connectedCallbackFunc_)
			connectedCallbackFunc_((int)ERROR_TYPE::SCRIPT_VERSION_NOT_MATCH);
		Disconnect();
	}


	void LoginApp::CmdImportClientMessages()
	{
		KBE_DEBUG(TEXT("LoginApp::CmdImportClientMessages: send Loginapp_importClientMessages ..."));
		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_importClientMessages"));
		bundle->Send(networkInterface_);
		delete bundle;
	}

	void LoginApp::Client_onImportClientMessages(MemoryStream &stream)
	{
		KBE_DEBUG(TEXT("LoginApp::Client_onImportClientMessages: stream size: %d"), stream.Length());

		// 先复制一份出来，以为写入作准备
		MemoryStream datas(stream);

		messages_->ImportMessagesFromStream(stream, SERVER_APP_TYPE::LoginApp);

		if (app_->pPersistentInofs())
			app_->pPersistentInofs()->WriteLoginappMessages(datas);
	
		CmdImportServerErrorsDescr();
	}

	void LoginApp::CmdImportServerErrorsDescr()
	{
		KBE_DEBUG(TEXT("LoginApp::CmdImportServerErrorsDescr: send Loginapp_importServerErrorsDescr ..."));
		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_importServerErrorsDescr"));
		bundle->Send(networkInterface_);
		delete bundle;
	}

	void LoginApp::Client_onImportServerErrorsDescr(MemoryStream &stream)
	{
		KBE_DEBUG(TEXT("LoginApp::Client_onImportServerErrorsDescr: stream size: %d"), stream.Length());

		// 先复制一份出来，以为写入作准备
		MemoryStream datas(stream);

		KBEErrors::ImportServerErrorsDescr(stream);

		if (app_->pPersistentInofs())
			app_->pPersistentInofs()->WriteServerErrorsDescr(datas);

		if (connectedCallbackFunc_)
			connectedCallbackFunc_((int)ERROR_TYPE::SUCCESS);
	}

	void LoginApp::Login(const FString& account, const FString& password, const TArray<uint8>& datas, CLIENT_TYPE clientType, ConnectCallbackFunc func)
	{
		if (!networkInterface_)
		{
			if (func)
				func((int)ERROR_TYPE::INVALID_NETWORK);
			return;
		}
		
		//KBE_DEBUG(TEXT("LoginApp::Login(): send login! username=%s"), *account);
		
		account_ = account;
		password_ = password;
		connectedCallbackFunc_ = func;

		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_login"));
		bundle->WriteInt8((int8)clientType);
		bundle->WriteBlob(datas);
		bundle->WriteString(account);
		bundle->WriteString(password);
		bundle->Send(networkInterface_);
		delete bundle;

	}

	void LoginApp::Client_onLoginFailed(MemoryStream &stream)
	{
		uint16 failedcode = stream.ReadUint16();
		TArray<uint8> serverDatas;
		stream.ReadBlob(serverDatas);

		KBE_ERROR(TEXT("LoginApp::Client_onLoginFailed: failedcode(%d), datas(%d)!"), failedcode, serverDatas.Num());
		if (connectedCallbackFunc_)
			connectedCallbackFunc_(failedcode);
	}

	void LoginApp::Client_onLoginSuccessfully(MemoryStream &stream)
	{
		baseappAccount_ = stream.ReadString();;
		baseappHost_ = stream.ReadString();
		baseappPort_ = stream.ReadUint16();
		TArray<uint8> serverDatas;
		stream.ReadBlob(serverDatas);

		KBE_DEBUG(TEXT("LoginApp::Client_onLoginSuccessfully: accountName(%s), addr(%s:%d), datas(%d)!"),
			*baseappAccount_, *baseappHost_, baseappPort_, serverDatas.Num());

		if (connectedCallbackFunc_)
			connectedCallbackFunc_((int)ERROR_TYPE::SUCCESS);
	}

	void LoginApp::ResetPassword(const FString& account, ConnectCallbackFunc func)
	{
		KBE_ASSERT(NetworkIsValid());


		account_ = account;
		password_ = TEXT("");
		connectedCallbackFunc_ = func;

		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_reqAccountResetPassword"));
		bundle->WriteString(account_);
		bundle->Send(networkInterface_);
		delete bundle;
	}

	void LoginApp::Client_onReqAccountResetPasswordCB(uint16 failcode)
	{
		if (failcode != 0)
		{
			KBE_ERROR(TEXT("LoginApp::Client_onReqAccountResetPasswordCB: '%s' is failed! code=%d!"), *account_, failcode);
		}
		else
		{
			KBE_DEBUG(TEXT("LoginApp::Client_onReqAccountResetPasswordCB: %s password reset success!"), *account_);
		}

		if (connectedCallbackFunc_)
			connectedCallbackFunc_(failcode);
	}

	void LoginApp::CreateAccount(const FString& username, const FString& password, const TArray<uint8>& datas, ConnectCallbackFunc func)
	{
		KBE_ASSERT(NetworkIsValid());

		connectedCallbackFunc_ = func;
		account_ = username;
		password_ = password;

		Bundle* bundle = new Bundle();
		bundle->NewMessage(messages_->GetMessage("Loginapp_reqCreateAccount"));
		bundle->WriteString(account_);
		bundle->WriteString(password_);
		bundle->WriteBlob(datas);
		bundle->Send(networkInterface_);
		delete bundle;
	}

	void LoginApp::Client_onCreateAccountResult(MemoryStream &stream)
	{
		uint16 retcode = stream.ReadUint16();
		TArray<uint8> datas;
		stream.ReadBlob(datas);

		if (retcode != (int)ERROR_TYPE::SUCCESS)
		{
			KBE_WARNING(TEXT("LoginApp::Client_onCreateAccountResult: %s create is failed! code=%d!"), *account_, retcode);
		}
		else
		{
			KBE_DEBUG(TEXT("LoginApp::Client_onCreateAccountResult: %s create is successfully!"), *account_);
		}

		if (connectedCallbackFunc_)
			connectedCallbackFunc_(retcode);
	}

	void LoginApp::Client_onAppActiveTickCB()
	{
		lastTickCBTime_ = FDateTime::UtcNow();
	}

	void LoginApp::Process()
	{
		if (networkInterface_)
		{
			networkInterface_->Process();
			messageReader_->Process();
		}

		if (networkInterface_ && networkInterface_->Valid())
		{
			SendTick();
		}
	}

	void LoginApp::HandleMessage(const FString &name, MemoryStream *stream)
	{
		if (name == "Client_onHelloCB") {
			Client_onHelloCB(*stream);
		}
		else if (name == "Client_onScriptVersionNotMatch") {
			Client_onScriptVersionNotMatch(*stream);
		}
		else if (name == "Client_onVersionNotMatch") {
			Client_onVersionNotMatch(*stream);
		}
		else if (name == "Client_onImportClientMessages") {
			Client_onImportClientMessages(*stream);
		}
		else if (name == "Client_onImportServerErrorsDescr") {

			Client_onImportServerErrorsDescr(*stream);
		}
		else if (name == "Client_onLoginFailed") {

			Client_onLoginFailed(*stream);
		}
		else if (name == "Client_onLoginSuccessfully") {
			Client_onLoginSuccessfully(*stream);
		}
		else if (name == "Client_onCreateAccountResult") {
			Client_onCreateAccountResult(*stream);
		}
		else if (name == "Client_onAppActiveTickCB") {
			Client_onAppActiveTickCB();
		}
		else
		{
			KBE_ERROR(TEXT("LoginApp::HandleMessage: 1 - unknown message '%s'"), *name);
		}
	}

	void LoginApp::HandleMessage(const FString &name, const TArray<FVariant> &args)
	{
		if (name == "Client_onReqAccountResetPasswordCB") {
			Client_onReqAccountResetPasswordCB(args[0].GetValue<uint16>());
		}
		else
		{
			KBE_ERROR(TEXT("LoginApp::HandleMessage: 2 - unknown message '%s'"), *name);
		}

	}

}
