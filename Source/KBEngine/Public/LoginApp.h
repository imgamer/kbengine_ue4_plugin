#pragma once
#include <functional>

#include "Core.h"
#include "MessagesHandler.h"

namespace KBEngine
{
	class KBEngineApp;
	class NetworkInterface;
	class MessageReader;
	class Messages;

	class KBENGINE_API LoginApp : public MessagesHandler
	{
	public:
		/* 连接回调函数
		int的参数表示错误码，更多的信息可以参考KBEngineApp::serverErrs_
		*/
		typedef std::function<void(int32)> ConnectCallbackFunc;

	public:
		LoginApp(KBEngineApp* app);
		virtual ~LoginApp();

		// 连接到服务器
		void Connect(const FString& host, uint16 port, ConnectCallbackFunc func);

		// 主动断开网络连接
		void Disconnect();

		// 检查网络是否有效（连接上）
		bool NetworkIsValid();

		// 登录
		void Login(const FString& account, const FString& password, const TArray<uint8>& datas, CLIENT_TYPE clientType, ConnectCallbackFunc func);

		// 重置密码
		void ResetPassword(const FString& account, ConnectCallbackFunc func);

		/*
		创建账号
		*/
		void CreateAccount(const FString& username, const FString& password, const TArray<uint8>& datas, ConnectCallbackFunc func);

		// 每个Tick执行一次
		void Process();

		virtual void HandleMessage(const FString &name, MemoryStream *stream) override;
		virtual void HandleMessage(const FString &name, const TArray<FVariant> &args) override;

		const FString& BaseAppAccount() { return baseappAccount_; }
		const FString& BaseAppHost() { return baseappHost_; }
		uint16 BaseAppPort() { return baseappPort_; }

	public:
		// for internal

		NetworkInterface* pNetworkInterface() { return networkInterface_; }
		void OnLoseConnect();  // 失去与服务器的连接（非主动断开）


	private:
		void ClearNetwork();
		void SendTick();

		void OnConnected(const FString& host, uint16 port, bool success);

		void CmdHello();
		void CmdImportClientMessages();
		void CmdImportServerErrorsDescr();

		void Client_onHelloCB(MemoryStream &stream);
		void Client_onVersionNotMatch(MemoryStream &stream);
		void Client_onScriptVersionNotMatch(MemoryStream &stream);
		void Client_onImportClientMessages(MemoryStream &stream);
		void Client_onImportServerErrorsDescr(MemoryStream &stream);

		void Client_onLoginFailed(MemoryStream &stream);
		void Client_onLoginSuccessfully(MemoryStream &stream);
		void Client_onReqAccountResetPasswordCB(uint16 failcode);
		void Client_onCreateAccountResult(MemoryStream &stream);

		void Client_onAppActiveTickCB();

	private:
		KBEngineApp* app_ = nullptr;

		// 消息处理器
		MessageReader *messageReader_ = nullptr;

		// 消息处执行器
		Messages *messages_ = nullptr;

		NetworkInterface* networkInterface_ = nullptr;

		// 记录下登录时的地址、账号等信息
		FString host_;
		uint16 port_;
		FString account_;
		FString password_;
		ConnectCallbackFunc connectedCallbackFunc_;


		// 登录成功后，loginapp返回的BaseApp登录信息
		FString baseappAccount_;
		FString baseappHost_;
		uint16 baseappPort_ = 0;

		// 最后一次心跳发送时间、最后一次收到心跳回复的时间
		// 用于作为是否断线的判断依据
		FDateTime lastTicktime_ = FDateTime::UtcNow();
		FDateTime lastTickCBTime_ = FDateTime::UtcNow();


	};  // end of class LoginApp;


}