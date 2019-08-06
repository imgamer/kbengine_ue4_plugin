#pragma once

#include "KBEDefine.h"

namespace KBEngine
{
	class KBENGINE_API KBEngineArgs
	{
	public:
		// 登录ip和端口
		FString host = "127.0.0.1";
		uint16 port = 20013;

		// 客户端类型
		// Reference: http://www.kbengine.org/docs/programming/clientsdkprogramming.html, client types
		CLIENT_TYPE clientType = CLIENT_TYPE::CLIENT_TYPE_MINI;

		// 持久化插件信息， 例如：从服务端导入的协议可以持久化到本地，下次登录版本不发生改变
		// 可以直接从本地加载来提供登录速度
		FString persistentDataPath = TEXT("Application.PersistentDataPath");

		// Allow synchronization role position information to the server
		// 是否开启自动同步玩家信息到服务端，信息包括位置与方向
		// 非高实时类游戏不需要开放这个选项
		bool syncPlayer = true;

		// 是否使用别名机制
		// 这个参数的选择必须与kbengine_defs.xml::cellapp/aliasEntityID的参数保持一致
		bool useAliasEntityID = true;

		// 在Entity初始化时是否触发属性的set_*事件(callPropertysSetMethods)
		bool isOnInitCallPropertysSetMethods = true;

		// 发送缓冲大小
		uint32 SEND_BUFFER_MAX = 65535;

		// 接收缓冲区大小
		uint32 RECV_BUFFER_MAX = 65535;

		// 心跳包发送间隔（间隔越短，认为断线的时间也越短）；单位：秒
		// 如果为0，则不发送
		// 注意：此值必须小于kbengine_defs.xml或kbengine.xml下<channelCommon><timeout><external>下配置的超时参数
		int32 tickInterval = 15;

		// 通信协议加密，blowfish协议
		TArray<uint8> encryptedKey;

		// 服务端与客户端的版本号以及协议MD5
		FString clientVersion = "1.3.8";
		FString clientScriptVersion = "0.1.0";
	};
}
