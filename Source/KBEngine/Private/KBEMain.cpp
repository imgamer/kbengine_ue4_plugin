// Fill out your copyright notice in the Description page of Project Settings.
#include "KBEMain.h"
#include "KBEnginePrivatePCH.h"
#include "KBEngineArgs.h"
#include "KBEEvent.h"

#include "Networking.h"
#include "SocketSubsystem.h"
#include "KBEDebug.h"

AKBEMain* AKBEMain::instance = nullptr;

// Sets default values
AKBEMain::AKBEMain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AKBEMain::~AKBEMain()
{
	if (pKBEApp)
	{
		delete pKBEApp;
		pKBEApp = nullptr;
	}

	if (AKBEMain::instance)
	{
		AKBEMain::instance = nullptr;
	}
}

void AKBEMain::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	KBE_ASSERT(!AKBEMain::instance);

	AKBEMain::instance = this;
	InitKBEngine();
}

// Called when the game starts or when spawned
void AKBEMain::BeginPlay()
{
	Super::BeginPlay();	
}

void AKBEMain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (pKBEApp)
	{
		delete pKBEApp;
		pKBEApp = nullptr;
	}

	if (AKBEMain::instance)
	{
		AKBEMain::instance = nullptr;
	}
}

// Called every frame
void AKBEMain::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	KBEUpdate();
}

void AKBEMain::KBEUpdate()
{
	if (pKBEApp)
		pKBEApp->Process();

	KBEngine::KBEEvent::Instance()->ProcessAsyncEvents();
}

void AKBEMain::InitKBEngine()
{
	//TestResolveIPAddress();

	KBEngine::KBEngineArgs *args = new KBEngine::KBEngineArgs();

	args->host = host;
	args->port = (uint16)port;
	args->clientType = clientType;

	args->persistentDataPath = persistentDataPath;

	args->syncPlayer = syncPlayer;
	args->useAliasEntityID = useAliasEntityID;
	args->isOnInitCallPropertysSetMethods = isOnInitCallPropertysSetMethods;

	args->TCP_SEND_BUFFER_MAX = TCP_SEND_BUFFER_MAX;
	args->TCP_RECV_BUFFER_MAX = TCP_RECV_BUFFER_MAX;

	args->forceDisableUDP = forceDisableUDP;
	args->UDP_SEND_BUFFER_MAX = UDP_SEND_BUFFER_MAX;
	args->UDP_RECV_BUFFER_MAX = UDP_RECV_BUFFER_MAX;

	pKBEApp = new KBEngine::KBEngineApp(args);

	//KBEngine::KBEEvent::useSyncMode(!isMultiThreads);
}

void AKBEMain::TestResolveIPAddress()
{
	KBE_WARNING(TEXT("AKBEMain::TestGetIPAddress:host:%s"), *host);

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	FSocket* socket = SocketSubsystem->CreateSocket(NAME_Stream, "TestGetIPAddress", true);

	TSharedPtr<FInternetAddr> remoteAddr = SocketSubsystem->CreateInternetAddr();
	
	bool bIsValid = false;
	remoteAddr->SetIp(*host, bIsValid);	// 验证是否ip地址
	if (!bIsValid)						// 不是ip地址，作为域名解析
	{
		ESocketErrors HostResolveError = SocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*host), *remoteAddr);
		if (HostResolveError == SE_NO_ERROR || HostResolveError == SE_EWOULDBLOCK)
		{
			FString outIP = remoteAddr->ToString(false);
			KBE_WARNING(TEXT("AKBEMain::TestGetIPAddress:resolve ---> %s to %s."), *host, *outIP);
		}

		/* 以下解析方式会导致崩溃问题，还未研究原因
		FResolveInfo *resolvInfo = SocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*host));
		if (resolvInfo == NULL)
		{
			KBE_ERROR(TEXT("AKBEMain::TestGetIPAddress:invalid server addr ------------------------>:%s."), *host);
			return;
		}

		FString outIP = resolvInfo->GetResolvedAddress().ToString(false);	// 此处会崩
		KBE_WARNING(TEXT("AKBEMain::TestGetIPAddress:resolve ------------------------> %s to %s."), *host, *outIP);
		*/
	}
}

