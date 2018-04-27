// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KBEngineApp.h"
#include "GameFramework/Actor.h"
#include "KBEMain.generated.h"

/*
插件的入口模块
在这个入口中安装了需要监听的事件(installEvents)，同时初始化KBEngine(initKBEngine)
*/

UCLASS()
class KBENGINE_API AKBEMain : public AActor
{
	GENERATED_BODY()

	static AKBEMain* instance;

public:
	static AKBEMain* Instance()
	{
		return AKBEMain::instance;
	}

	// Sets default values for this actor's properties
	AKBEMain();
	~AKBEMain();

	virtual void PreInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called whenever this actor is being removed from a level
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	virtual void KBEUpdate();
	virtual void InitKBEngine();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString host = "127.0.0.1";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 port = 20013;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	KBEngine::CLIENT_TYPE clientType = KBEngine::CLIENT_TYPE::CLIENT_TYPE_MINI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString persistentDataPath = "Application.PersistentDataPath";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool syncPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 sendBufferMax = 32768;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 recvBufferMax = 65535;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool useAliasEntityID = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isOnInitCallPropertysSetMethods = true;


	KBEngine::KBEngineApp * pKBEApp = nullptr;
};
