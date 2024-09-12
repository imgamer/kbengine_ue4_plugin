#pragma once

#include "NetworkInterfaceBase.h"
#include "HAL/Runnable.h"
#include "Containers/UnrealString.h"
#include "MemoryStream.h"
#include "KBEDebug.h"
#include "HAL/PlatformMisc.h"


namespace KBEngine
{

class NetworkStatus
{
public:
	virtual ~NetworkStatus() {}
	virtual void MainThreadProcess(NetworkInterfaceBase* networkInterface) = 0;
};

class Status_Connected : public NetworkStatus
{
public:
	virtual void MainThreadProcess(NetworkInterfaceBase* networkInterface) override
	{
		if (networkInterface->Valid())
			networkInterface->ProcessMessage();
	}
};

class Status_Connecting : public NetworkStatus, public FRunnable
{
public:
	Status_Connecting()
	{}

	Status_Connecting(NetworkInterfaceBase::ConnectState& opt) :
		opt_(opt)
	{
		thread_ = FRunnableThread::Create(this, *FString::Printf(TEXT("KBEngineNetworkInterfaceStatus_Connecting:%p"), this));
	}

	virtual ~Status_Connecting()
	{
		if (thread_)
		{
			thread_->WaitForCompletion();
			delete thread_;
			thread_ = nullptr;
		}
	}

	// call by sub-thread
	virtual uint32 Run() override
	{
		auto Addr = opt_.networkInterface->SocketSubsystem()->CreateInternetAddr(0, opt_.connectPort);
		bool bIsValid;

		Addr->SetIp(*opt_.connectIP, bIsValid);
		if (opt_.socket->Connect(*Addr) == false)
		{
			opt_.error = TEXT("Connect Error");
		}

		// 无论连接成功与否，都标识已进行过连接
		connected_ = true;
		return 0;
	}

	virtual void MainThreadProcess(NetworkInterfaceBase* networkInterface) override
	{
		if (connected_)
			networkInterface->OnConnected(opt_);
	}

protected:
	NetworkInterfaceBase::ConnectState opt_;
	FRunnableThread* thread_ = nullptr;
	bool connected_ = false;
};

class NetworkStatusKCPConnecting : public NetworkStatus
{
public:
	const FString UDP_HELLO = TEXT("62a559f3fa7748bc22f8e0766019d498");
	const FString UDP_HELLO_ACK = TEXT("1432ad7c829170a76dd31982c3501eca");

	NetworkStatusKCPConnecting(NetworkInterfaceBase::ConnectState& opt)
	{
		opt_ = opt;

		StartConnect();
	};

	void MainThreadProcess(NetworkInterfaceBase* networkInterface) override
	{
		CheckConnect();

		if (connected_)
			networkInterface->OnConnected(opt_);
	}

	uint32 GetConnID()
	{
		return connID_;
	};

private:
	void StartConnect()
	{
		MemoryStream s;
		s.WriteString(UDP_HELLO);

		auto addr = opt_.networkInterface->SocketSubsystem()->CreateInternetAddr(0, opt_.connectPort);
		bool bIsValid;
		addr->SetIp(*opt_.connectIP, bIsValid);

		int32 sent = 0;
		opt_.socket->SendTo(s.Data(), s.Length(), sent, *addr);

		startTime_ = FPlatformTime::Seconds();
		connectCount_ += 1;

		KBE_INFO(TEXT("NetworkStatusKCPConnecting KCP connect to %s:%d"), *opt_.connectIP, opt_.connectPort);
	};

	void CheckConnect()
	{
		UINT32 DataSize;
		if (opt_.socket->HasPendingData(DataSize))
		{
			MemoryStream s;
			int32 bytesRead = 0;
			TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			if (opt_.socket->RecvFrom(s.Data(), s.Size(), bytesRead, *addr))
			{
				s.WPos(bytesRead);
				FString helloAck = s.ReadString();
				FString versionString = s.ReadString();
				uint32 connID = s.ReadUint32();

				KBE_INFO(TEXT("NetworkStatusKCPConnecting: handshake success, helloAck(%s), versionString(%s), connID(%d)"),
					*helloAck, *versionString, connID);

				if (helloAck != UDP_HELLO_ACK)
				{
					opt_.error = FString::Printf(TEXT("failed to connect to '%s:%d'! receive hello-ack mismatch!"), *opt_.connectIP, opt_.connectPort);
				}
				else if (connID == 0)
				{
					opt_.error = TEXT("failed to connect! conv is 0!");
				}
				// TODO：验证 versionString 是否一致

				connID_ = connID;
			}

			// 无论连接成功与否，都标识已进行过连接
			connected_ = true;
		}
		else
		{
			auto currTime = FPlatformTime::Seconds();

			if (currTime - startTime_ > 1.5f && connectCount_ < 2)	// 短时间内未收到服务端回复，再次尝试连接
			{
				StartConnect();
				connectCount_ += 1;
				KBE_WARNING(TEXT("NetworkStatusKCPConnecting::CheckConnect:failed to connect to '%s:%d', try again"), *opt_.connectIP, opt_.connectPort);
				return;
			}

			if (currTime - startTime_ > 15.0f)	// 超时失败，业务层需要进行处理
			{
				opt_.error = FString::Printf(TEXT("failed to connect to '%s:%d'!"), *opt_.connectIP, opt_.connectPort);
				connected_ = true;
				return;
			}
		}
	};

private:
	NetworkInterfaceBase::ConnectState opt_;

	bool connected_ = false;

	uint32 connID_ = 0;

	double startTime_ = 0.0f;

	uint8 connectCount_ = 0;

};

}	// end namespace KBEngine