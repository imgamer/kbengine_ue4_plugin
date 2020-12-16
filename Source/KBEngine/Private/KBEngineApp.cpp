#include "KBEngineApp.h"
#include "KBEnginePrivatePCH.h"
#include <functional>
#include "DateTime.h"
#include "KBEMath.h"
#include "KBEPersonality.h"
#include "LoginApp.h"
#include "BaseApp.h"
#include "KBEErrors.h"
#include "EntityDef.h"
#include "KBEEvent.h"

namespace KBEngine
{
	KBEngineApp* KBEngineApp::app = nullptr;




	KBEngineApp::KBEngineApp(KBEngineArgs* args)
	{
		KBE_ASSERT(!app);

		app = this;

		KBEErrors::InitLocalErrors();

		args_ = args;

		// 允许持久化KBE(例如:协议，entitydef等)
		if (args->persistentDataPath != "")
			persistentInofs_ = new PersistentInofs(args_->persistentDataPath, ClientVersion(), ClientScriptVersion(), LoginappHost(), LoginappPort());
	}

	KBEngineApp::~KBEngineApp()
	{
		KBE_INFO(TEXT("KBEngine::~KBEngineApp()"));

		loseConnectedFromServer_ = false;
		KBEEvent::Instance()->Clear();
		CloseLoginApp();
		CloseBaseApp();
		CloseAcrossBaseApp();
		KBEErrors::Clear();
		EntityDef::Clear();
		KBEngineApp::app = nullptr;
	}

	void KBEngineApp::OnLoseConnect()
	{
		loseConnectedFromServer_ = true;
		if (loginApp_)
			loginApp_->OnLoseConnect();

		if (baseApp_)
			baseApp_->OnLoseConnect();

		if (acrossBaseApp_)
			acrossBaseApp_->OnLoseConnect();

		if (isInAcrossServer_)
			isInAcrossServer_ = false;

		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnDisconnect();
	}

	Entity* KBEngineApp::Player()
	{
		if (baseApp_)
			return baseApp_->Player();
		if (acrossBaseApp_)
			return acrossBaseApp_->Player();
		return nullptr;
	}

	Entity* KBEngineApp::FindEntity(int32 entityID)
	{
		if (baseApp_)
			return baseApp_->FindEntity(entityID);
		if (acrossBaseApp_)
			return acrossBaseApp_->FindEntity(entityID);
		return nullptr;
	}

	const TMap<int32, Entity*>* KBEngineApp::Entities()
	{
		if (baseApp_)
			return baseApp_->Entities();
		if (acrossBaseApp_)
			return acrossBaseApp_->Entities();
		return nullptr;
	}



	void KBEngineApp::Process()
	{
		if (loseConnectedFromServer_)
		{
			SAFE_DELETE(loginApp_);
			SAFE_DELETE(baseApp_);
			SAFE_DELETE(acrossBaseApp_);
			loseConnectedFromServer_ = false;
		}

		// 处理额外任务
		updatables_.Update();

		if (loginApp_)
			loginApp_->Process();

		if (baseApp_)
			baseApp_->Process();

		if (acrossBaseApp_)
			acrossBaseApp_->Process();
	}

	void KBEngineApp::Disconnect()
	{
		username_ = TEXT("");
		password_ = TEXT("";)
		clientdatas_.Empty();

		CloseLoginApp();
		CloseBaseApp();
		CloseAcrossBaseApp();
	}

	void KBEngineApp::CloseLoginApp()
	{
		if (loginApp_)
		{
			loginApp_->Disconnect();
			delete loginApp_;
			loginApp_ = nullptr;
		}
	}

	void KBEngineApp::CloseBaseApp()
	{
		if (baseApp_)
		{
			baseApp_->Disconnect();
			delete baseApp_;
			baseApp_ = nullptr;
		}
	}

	void KBEngineApp::CloseAcrossBaseApp()
	{
		if (acrossBaseApp_)
		{
			acrossBaseApp_->Disconnect();
			delete acrossBaseApp_;
			acrossBaseApp_ = nullptr;
		}
	}

	void KBEngineApp::Login(const FString& username, const FString& password, const TArray<uint8>& datas)
	{
		KBE_ASSERT(!loginApp_);
		KBE_ASSERT(!baseApp_);
		KBE_ASSERT(!acrossBaseApp_);

		EntityDef::Init();

		username_ = username;
		password_ = password;
		clientdatas_ = datas;

		loginApp_ = new LoginApp(this);
		loginApp_->Connect(LoginappHost(), LoginappPort(), std::bind(&KBEngineApp::OnConnectToLoginappCB, this, std::placeholders::_1, TEXT("Login")));
	}

	void KBEngineApp::OnConnectToLoginappCB(int32 code, FString key)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
			return;
		}

		if (key == TEXT("Login"))
			loginApp_->Login(username_, password_, clientdatas_, args_->clientType, std::bind(&KBEngineApp::OnLoginToLoginappCB, this, std::placeholders::_1));
		else if (key == TEXT("CreateAccount"))
			loginApp_->CreateAccount(username_, password_, clientdatas_, std::bind(&KBEngineApp::OnCreateAccountCB, this, std::placeholders::_1));
		else if (key == TEXT("ResetPassword"))
			loginApp_->ResetPassword(username_, std::bind(&KBEngineApp::OnResetPasswordCB, this, std::placeholders::_1));
		else
			KBE_ERROR(TEXT("KBEngineApp::OnConnectToLoginappCB: unknown key '%s'"), *key);
	}

	void KBEngineApp::OnLoginToLoginappCB(int32 code)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
			return;
		}

		auto baseappHost = loginApp_->BaseAppHost();
		auto baseappPort = loginApp_->BaseAppPort();
		baseappAccount_ = loginApp_->BaseAppAccount();

		baseApp_ = new BaseApp(this);
		
		// 不能在这里销毁LoginApp，因为此时代码还在LoginApp的Process()层次
		//CloseLoginApp();

		baseApp_->Connect(baseappHost, baseappPort, std::bind(&KBEngineApp::OnConnectToBaseappCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnConnectToBaseappCB(int32 code)
	{
		// 不管如何，在这里清理LoginApp是安全的
		CloseLoginApp();

		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
			return;
		}

		baseApp_->Login(baseappAccount_, password_, std::bind(&KBEngineApp::OnLoginToBaseappCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnLoginToBaseappCB(int32 code)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
			return;
		}
	}

	void KBEngineApp::CreateAccount(const FString& username, const FString& password, const TArray<uint8>& datas)
	{
		KBE_ASSERT(!loginApp_);
		KBE_ASSERT(!baseApp_);
		KBE_ASSERT(!acrossBaseApp_);

		EntityDef::Init();

		username_ = username;
		password_ = password;
		clientdatas_ = datas;

		loginApp_ = new LoginApp(this);
		loginApp_->Connect(LoginappHost(), LoginappPort(), std::bind(&KBEngineApp::OnConnectToLoginappCB, this, std::placeholders::_1, TEXT("CreateAccount")));
	}

	void KBEngineApp::OnCreateAccountCB(int32 code)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
		}
		else
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnCreateAccountSuccess(username_);
		}
	}

	void KBEngineApp::ResetPassword(const FString& username)
	{
		KBE_ASSERT(!loginApp_);
		KBE_ASSERT(!baseApp_);
		KBE_ASSERT(!acrossBaseApp_);

		EntityDef::Init();

		username_ = username;
		password_ = TEXT("");
		clientdatas_.Empty();

		loginApp_ = new LoginApp(this);
		loginApp_->Connect(LoginappHost(), LoginappPort(), std::bind(&KBEngineApp::OnConnectToLoginappCB, this, std::placeholders::_1, TEXT("ResetPassword")));
	}

	void KBEngineApp::OnResetPasswordCB(int32 code)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
		}
		else
		{
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnResetPasswordSuccess(username_);
		}
	}

	void KBEngineApp::ReLoginBaseapp()
	{
		KBE_ASSERT(baseApp_);
		baseApp_->Relogin(std::bind(&KBEngineApp::OnReLoginBaseappCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnReLoginBaseappCB(int32 code)
	{
		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnReLoginBaseapp(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
	}

	void KBEngineApp::BindAccountEmail(const FString& emailAddress)
	{
		KBE_ASSERT(baseApp_);
		baseApp_->BindAccountEmail(emailAddress, std::bind(&KBEngineApp::OnBindAccountEmailCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnBindAccountEmailCB(int32 code)
	{
		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnBindAccountEmail(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
	}

	void KBEngineApp::NewPassword(const FString old_password, const FString new_password)
	{
		KBE_ASSERT(baseApp_);
		baseApp_->NewPassword(old_password, new_password, std::bind(&KBEngineApp::OnNewPasswordCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnNewPasswordCB(int32 code)
	{
		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnNewPassword(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
	}

	void KBEngineApp::AcrossServerReady(UINT64 loginKey, FString& baseappHost, UINT16 port)
	{
		acrossLoginKey_ = loginKey;
		acrossBaseappHost_ = baseappHost;
		acrossBaseappPort_ = port;
		acrossLoginReadyTime_ = FDateTime::UtcNow();

		// todo: 登录过期机制，状态设置，过期清理数据。

		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnAcrossServerReady();
	}

	void KBEngineApp::ResetAcrossData()
	{
		acrossLoginKey_ = 0;
		acrossBaseappHost_ = "";
		acrossBaseappPort_ = 0;
		acrossLoginReadyTime_;
	}

	void KBEngineApp::AcrossLoginBaseapp()
	{
		KBE_ASSERT(!loginApp_);
		KBE_ASSERT(!baseApp_);
		KBE_ASSERT(!acrossBaseApp_);

		//恢复一下，不然之后会因为已经初始化而触发assert
		pMessages()->BaseappMessageImported(false);
		EntityDef::EntityDefImported(false);

		acrossBaseApp_ = new BaseApp(this);
		acrossBaseApp_->Connect(acrossBaseappHost_, acrossBaseappPort_, std::bind(&KBEngineApp::OnConnectAcrossBaseappCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnConnectAcrossBaseappCB(int32 code)
	{
		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			ResetAcrossData();
			if (KBEPersonality::Instance())
				KBEPersonality::Instance()->OnAcrossLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
			return;
		}

		acrossBaseApp_->AcrossLogin(baseappAccount_, password_, ClientType(), acrossLoginKey_, std::bind(&KBEngineApp::OnLoginAcrossBaseappCB, this, std::placeholders::_1));
	}

	void KBEngineApp::OnLoginAcrossBaseappCB(int32 code)
	{
		ResetAcrossData();
		KBE_DEBUG(TEXT("KBEngineApp::OnLoginAcrossBaseappCB: (%s:%s)!"), *KBEErrors::ErrorName(code), *KBEErrors::ErrorDesc(code));

		if (code != (int32)ERROR_TYPE::SUCCESS)
		{
			if (KBEPersonality::Instance())
			{
				KBEPersonality::Instance()->OnAcrossLoginFailed(code, KBEErrors::ErrorName(code), KBEErrors::ErrorDesc(code));
				return;
			}
		}

		isInAcrossServer_ = true;

		if (KBEPersonality::Instance())
			KBEPersonality::Instance()->OnAcrossLoginSuccess();

		acrossBaseApp_->AcrossLoginSuccess();
	}

	void KBEngineApp::AcrossLoginBack()
	{
		KBE_ASSERT(!loginApp_);
		KBE_ASSERT(!baseApp_);
		KBE_ASSERT(!acrossBaseApp_);

		EntityDef::Init();

		//恢复一下，不然之后会因为已经初始化而触发assert
		pMessages()->BaseappMessageImported(false);
		EntityDef::EntityDefImported(false);

		// TODO: 使用已有的账号密码登陆源服务器
		loginApp_ = new LoginApp(this);
		loginApp_->Connect(LoginappHost(), LoginappPort(), std::bind(&KBEngineApp::OnConnectToLoginappCB, this, std::placeholders::_1, TEXT("Login")));
	}
}
