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
	class Entity;

	class KBENGINE_API BaseApp : public MessagesHandler
	{
		const static int ADDITIONAL_UPDATE_COUNT = 8;

	public:
		/* 连接回调函数
		int的参数表示错误码，更多的信息可以参考KBEngineApp::serverErrs_
		*/
		typedef std::function<void(int32)> ConnectCallbackFunc;

	public:
		BaseApp(KBEngineApp* app);
		virtual ~BaseApp();

		// 连接到服务器
		void Connect(const FString& host, uint16 port, ConnectCallbackFunc func);

		// 主动断开网络连接
		void Disconnect();

		// 检查网络是否有效（连接上）
		bool NetworkIsValid();

		// 登录
		void Login(const FString& account, const FString& password, ConnectCallbackFunc func);

		/*
		重登录到网关(baseapp)
		一些移动类应用容易掉线，可以使用该功能快速的重新与服务端建立通信
		*/
		void Relogin(ConnectCallbackFunc func);

		// 绑定账号邮箱
		void BindAccountEmail(const FString& emailAddress, ConnectCallbackFunc func);

		// 修改密码
		void NewPassword(const FString& old_password, const FString& new_password, ConnectCallbackFunc func);

		// 每个Tick执行一次
		void Process();

		// 获取玩家自身的角色Entity
		Entity* Player();

		// 通过entityID找到对应的entity实例
		Entity* FindEntity(int32 entityID);

		// 获取Entity字典
		const TMap<int32, Entity*>* Entities() { return &entities_; }

		// 根据Key取相应的SpaceData
		const FString& GetSpaceData(const FString& key);

		// 处理来自服务器的消息
		virtual void HandleMessage(const FString &name, MemoryStream *stream) override;
		virtual void HandleMessage(const FString &name, const TArray<FVariant> &args) override;

	public:
		// for internal

		void EntityServerPos(FVector pos) { entityServerPos_ = pos; }
		Messages* pMessages() { return messages_; }
		NetworkInterface* pNetworkInterface() { return networkInterface_; }
		void OnLoseConnect();  // 失去与服务器的连接（非主动断开）


	private:
		void UpdatePlayerToServer();
		void ClearNetwork();
		MemoryStream* FindBufferedCreateEntityMessage(int32 entityID);
		int32 GetAoiEntityIDFromStream(MemoryStream &stream);
		void ClearEntities(bool isall);
		void ClearSpace(bool isall);
		void SendTick();


		void OnConnected(const FString& host, uint16 port, bool success);

		void CmdHello();
		void CmdImportClientMessages();
		void CmdImportClientEntityDef();

		void Client_onHelloCB(MemoryStream &stream);
		void Client_onVersionNotMatch(MemoryStream &stream);
		void Client_onScriptVersionNotMatch(MemoryStream &stream);
		void Client_onImportClientMessages(MemoryStream &stream);
		void Client_onImportClientEntityDef(MemoryStream &stream);
		
		void Client_onLoginBaseappFailed(uint16 failedcode);
		void Client_onReloginBaseappFailed(uint16 failedcode);
		void Client_onReloginBaseappSuccessfully(MemoryStream &stream);
		void Client_onReqAccountBindEmailCB(uint16 failcode);
		void Client_onReqAccountNewPasswordCB(uint16 failcode);

		void Client_onAppActiveTickCB();
		void Client_onKicked(uint16 failedcode);
		void Client_onControlEntity(int32 eid, uint8 isControlled);
		void Client_onCreatedProxies(uint64 rndUUID, int32 eid, FString entityType);
		void Client_onEntityDestroyed(int32 eid);

		// 客户端属性值改变通知
		void Client_onUpdatePropertysOptimized(MemoryStream &stream);
		void Client_onUpdatePropertys(MemoryStream &stream);
		void OnUpdatePropertys(int32 eid, MemoryStream& stream);

		// 客户端远程方法调用
		void Client_onRemoteMethodCallOptimized(MemoryStream &stream);
		void Client_onRemoteMethodCall(MemoryStream &stream);
		void OnRemoteMethodCall(int32 eid, MemoryStream& stream);

		// 进入、离开世界
		void Client_onEntityEnterWorld(MemoryStream &stream);
		void Client_onEntityLeaveWorldOptimized(MemoryStream &stream);
		void Client_onEntityLeaveWorld(int32 eid);

		// 进入、离开当前地图
		void Client_onEntityEnterSpace(MemoryStream &stream);
		void Client_onEntityLeaveSpace(int32 eid);

		// 地图数据相关
		void AddSpaceGeometryMapping(uint32 spaceID, const FString& respath);
		void Client_initSpaceData(MemoryStream &stream);
		void Client_setSpaceData(uint32 spaceID, const FString& key, const FString& value);
		void Client_delSpaceData(uint32 spaceID, const FString& key);

		/*
		服务端更新玩家的基础位置和朝向， 客户端以这个基础位置加上便宜值计算出玩家周围实体的坐标
		*/
		void Client_onUpdateBasePos(float x, float y, float z);
		void Client_onUpdateBasePosXZ(float x, float z);
		void Client_onUpdateBaseDir(MemoryStream &stream);

		/*
		服务端强制设置了玩家的坐标
		例如：在服务端使用avatar.position=(0,0,0), 或者玩家位置与速度异常时会强制拉回到一个位置
		*/
		void Client_onSetEntityPosAndDir(MemoryStream &stream);

		// 坐标、朝向改变相关
		void Client_onUpdateData(MemoryStream &stream);
		void Client_onUpdateData_ypr(MemoryStream &stream);
		void Client_onUpdateData_yp(MemoryStream &stream);
		void Client_onUpdateData_yr(MemoryStream &stream);
		void Client_onUpdateData_pr(MemoryStream &stream);
		void Client_onUpdateData_y(MemoryStream &stream);
		void Client_onUpdateData_p(MemoryStream &stream);
		void Client_onUpdateData_r(MemoryStream &stream);
		void Client_onUpdateData_xz(MemoryStream &stream);
		void Client_onUpdateData_xz_ypr(MemoryStream &stream);
		void Client_onUpdateData_xz_yp(MemoryStream &stream);
		void Client_onUpdateData_xz_yr(MemoryStream &stream);
		void Client_onUpdateData_xz_pr(MemoryStream &stream);
		void Client_onUpdateData_xz_y(MemoryStream &stream);
		void Client_onUpdateData_xz_p(MemoryStream &stream);
		void Client_onUpdateData_xz_r(MemoryStream &stream);
		void Client_onUpdateData_xyz(MemoryStream &stream);
		void Client_onUpdateData_xyz_ypr(MemoryStream &stream);
		void Client_onUpdateData_xyz_yp(MemoryStream &stream);
		void Client_onUpdateData_xyz_yr(MemoryStream &stream);
		void Client_onUpdateData_xyz_pr(MemoryStream &stream);
		void Client_onUpdateData_xyz_y(MemoryStream &stream);
		void Client_onUpdateData_xyz_p(MemoryStream &stream);
		void Client_onUpdateData_xyz_r(MemoryStream &stream);
		void UpdateVolatileData(int32 entityID, float x, float y, float z, float yaw, float pitch, float roll, int8 isOnGround);

		// 数据流接收相关
		void Client_onStreamDataStarted(int16 id, uint32 datasize, const FString& descr);
		void Client_onStreamDataRecv(MemoryStream &stream);
		void Client_onStreamDataCompleted(int16 id);

		// 服务器通知客户端：某个entity的parent改变了
		void Client_onParentChanged(int32 eid, int32 parentID);

		bool NeedAdditionalUpdate(Entity *entity);
		void ResetAdditionalUpdateCount();
		bool PlayerNeedUpdate(Entity *entity, bool moveChanged);

	private:
		KBEngineApp* app_ = nullptr;

		// 消息处理器
		MessageReader *messageReader_ = nullptr;

		// 消息处执行器
		Messages *messages_ = nullptr;

		NetworkInterface* networkInterface_ = nullptr;

		FString host_;
		uint16 port_;
		ConnectCallbackFunc connectedCallbackFunc_;

		// 当前玩家的实体id与实体类别
		uint64 entity_uuid_ = 0;
		int32 entity_id_ = 0;
		FString entity_type_ = "";

		// 登录时的账号密码等信息
		FString account_;
		FString password_;

		// space的数据，具体看API手册关于spaceData
		// https://github.com/kbengine/kbengine/tree/master/docs/api
		TMap<FString, FString> spaceDatas_;

		// 所有实体都保存于这里， 请参看API手册关于entities部分
		// https://github.com/kbengine/kbengine/tree/master/docs/api
		TMap<int32, Entity*> entities_;

		// 在玩家AOI范围小于256个实体时我们可以通过一字节索引来找到entity
		TArray<int32> entityIDAliasIDList_;

		// controlledBy机制中记录的被本地客户端控制的Entity（自己除外）
		TArray<Entity *> controlledEntities_;

		TMap<int32, MemoryStream*> bufferedCreateEntityMessage_;

		// 玩家当前所在空间的id， 以及空间对应的资源
		uint32 spaceID_ = 0;
		FString spaceResPath_ = "";
		bool isLoadedGeometry_ = false;

		// 当前服务端最后一次同步过来的玩家位置
		FVector entityServerPos_ = FVector::ZeroVector;

		// 最后一次心跳发送时间、最后一次收到心跳回复的时间
		// 用于作为是否断线的判断依据
		FDateTime lastTicktime_ = FDateTime::UtcNow();
		FDateTime lastTickCBTime_ = FDateTime::UtcNow();

		// 最后一次同步坐标、朝向给服务器的时间，用于控制同步频率
		FDateTime lastUpdateToServerTime_ = FDateTime::UtcNow();
		
		// 玩家停止移动后，额外向服务端同步位置朝向的次数
		uint8 additionalUpdateCount_ = BaseApp::ADDITIONAL_UPDATE_COUNT;
	};  // end of class BaseApp;


}