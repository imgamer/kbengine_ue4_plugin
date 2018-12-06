// Fill out your copyright notice in the Description page of Project Settings.
#include "KBEMain.h"
#include "KBEnginePrivatePCH.h"
#include "KBEngineArgs.h"
#include "KBEEvent.h"

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
	KBEngine::KBEngineArgs *args = new KBEngine::KBEngineArgs();

	args->host = host;
	args->port = (uint16)port;
	args->clientType = clientType;

	args->persistentDataPath = persistentDataPath;

	args->syncPlayer = syncPlayer;
	args->useAliasEntityID = useAliasEntityID;
	args->isOnInitCallPropertysSetMethods = isOnInitCallPropertysSetMethods;

	args->SEND_BUFFER_MAX = sendBufferMax;
	args->RECV_BUFFER_MAX = recvBufferMax;

	pKBEApp = new KBEngine::KBEngineApp(args);

	//KBEngine::KBEEvent::useSyncMode(!isMultiThreads);
}

