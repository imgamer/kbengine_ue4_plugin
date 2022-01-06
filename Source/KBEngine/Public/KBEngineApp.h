#pragma once

#include <string.h>

#include "KBEDebug.h"
#include "Entity.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "KBEngineArgs.h"
#include "PersistentInfos.h"
#include "KBEUpdatable.h"
#include "Message.h"
#include "BaseApp.h"
#include "LoginApp.h"

#define byte uint8

namespace KBEngine
{
	class LoginApp;
	class BaseApp;

	class KBENGINE_API KBEngineApp
	{
	public:
		static KBEngineApp* app;

	public:
		KBEngineApp(KBEngineArgs* args);
		virtual ~KBEngineApp();

		// 插件的主循环处理函数
		virtual void Process();

		// 断开与服务器的连接
		void Disconnect();

		// 获取玩家自身的角色Entity
		Entity* Player();

		/*
		登录到服务端，必须登录完成loginapp与网关(baseapp)，登录流程才算完毕
		*/
		void Login(const FString& username, const FString& password, const TArray<uint8>& datas);

		void CreateAccount(const FString& username, const FString& password, const TArray<uint8>& datas);
		void ResetPassword(const FString& username);

		void ReLoginBaseapp();
		void BindAccountEmail(const FString& emailAddress);
		void NewPassword(const FString old_password, const FString new_password);

		// 使用未过期的登录凭证跨服登录baseapp
		void AcrossLoginBaseapp();
		void AcrossServerReady(UINT64 loginKey, FString& baseappHost, UINT16 port);
		// 跨服登陆结束后重新登陆源服务器
		void AcrossLoginBack();

		// 按照标准，每个客户端部分都应该包含这个属性
		const FString& Component() { return component_; }

		// 通过EntityID查找与之对应的Entity实例
		Entity* FindEntity(int32 entityID);

		// 获取Entity字典
		const TMap<int32, Entity*>* Entities();

	public:
		// for internal

		PersistentInofs* pPersistentInofs() { return persistentInofs_; }

		BaseApp* pBaseApp()
		{
			if(baseApp_!= nullptr)
				return baseApp_;
			if (acrossBaseApp_ != nullptr) 
				return acrossBaseApp_;
			return nullptr;
		}

		LoginApp* pLoginApp() { return loginApp_; }
		Messages* pMessages() { return &messages_; }

		void OnLoseConnect();  // 失去与服务器的连接（非主动断开）

	public:
		// args for internal
		const TArray<uint8>& EncryptedKey() { return args_->encryptedKey; }
		const FString& ClientVersion() { return args_->clientVersion; }
		const FString& ClientScriptVersion() { return args_->clientScriptVersion; }
		int32 TickInterval() { return args_->tickInterval; }
		bool IsOnInitCallPropertysSetMethods() { return args_->isOnInitCallPropertysSetMethods; }
		bool UseAliasEntityID() { return args_->useAliasEntityID; }
		bool SyncPlayer() { return args_->syncPlayer; }
		const FString& PersistentDataPath() { return args_->persistentDataPath; }
		CLIENT_TYPE ClientType() { return args_->clientType; }
		const FString& LoginappHost() { return args_->host; }
		uint16 LoginappPort() { return args_->port; }
		uint32 GetTcpRecvBufferMax() { return args_->TCP_RECV_BUFFER_MAX; }
		uint32 GetTcpSendBufferMax() {	return args_->TCP_SEND_BUFFER_MAX; }
		
		uint32 GetUdpRecvBufferMax() { return args_->UDP_RECV_BUFFER_MAX; }
		uint32 GetUdpSendBufferMax() { return args_->UDP_SEND_BUFFER_MAX; }
		bool IsForceDisableUDP() { return args_->forceDisableUDP; }

	private:
		// 取得初始化时的参数
		KBEngineArgs* GetInitArgs() { return args_; }

		void CloseLoginApp();
		void CloseBaseApp();
		void CloseAcrossBaseApp();

		void OnConnectToLoginappCB(int32 code, FString key);
		void OnLoginToLoginappCB(int32 code);

		void OnConnectToBaseappCB(int32 code);
		void OnLoginToBaseappCB(int32 code);

		void OnConnectAcrossBaseappCB(int32 code);
		void OnLoginAcrossBaseappCB(int32 code);

		void OnCreateAccountCB(int32 code);
		void OnResetPasswordCB(int32 code);

		void OnReLoginBaseappCB(int32 code);
		void OnBindAccountEmailCB(int32 code);
		void OnNewPasswordCB(int32 code);

	private:
		void ResetAcrossData();

		FString component_ = TEXT("client");

		LoginApp* loginApp_ = nullptr;
		BaseApp* baseApp_ = nullptr;

		BaseApp* acrossBaseApp_ = nullptr;	// 跨服登录的baseapp

		// 消息管理器
		Messages messages_;

		// 每帧都执行的额外任务
		Updatables updatables_;

		// 构造时传递进来的运行参数
		KBEngineArgs* args_ = NULL;

		// 持久化插件信息， 例如：从服务端导入的协议可以持久化到本地，下次登录版本不发生改变
		// 可以直接从本地加载来提供登录速度
		PersistentInofs* persistentInofs_ = nullptr;

		FString username_;
		FString password_;
		TArray<uint8> clientdatas_;
		FString baseappAccount_;

		UINT64 acrossLoginKey_;
		FString acrossBaseappHost_;
		UINT16 acrossBaseappPort_;
		FDateTime acrossLoginReadyTime_;

		bool isInAcrossServer_ = false;

		bool loseConnectedFromServer_ = false;
	};

}


