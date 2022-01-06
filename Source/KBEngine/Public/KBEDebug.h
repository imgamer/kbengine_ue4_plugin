// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"

#define KBE_ASSERT(exp) verify( (exp) )

//DECLARE_LOG_CATEGORY_EXTERN(KBELog, Log, All);
DEFINE_LOG_CATEGORY_STATIC(KBELog, Log, All);

//FDateTime::UtcNow().ToString(TEXT("[%Y-%m-%d %H:%M:%S %N]")));

// see also: Engine\Source\Runtime\Core\Public\Logging\LogVerbosity.h
#define KBE_DEBUG(msg, ...)   UE_LOG(KBELog, Log, msg, __VA_ARGS__);
#define KBE_INFO(msg, ...)    UE_LOG(KBELog, Display, msg, __VA_ARGS__);
#define KBE_WARNING(msg, ...) UE_LOG(KBELog, Warning, msg, __VA_ARGS__);
#define KBE_ERROR(msg, ...)   UE_LOG(KBELog, Error, msg, __VA_ARGS__);
#define KBE_FATAL(msg, ...)   UE_LOG(KBELog, Fatal, msg, __VA_ARGS__);
#define KBE_DISPLAY(msg, ...) UE_LOG(KBELog, Display, msg, __VA_ARGS__);
#define KBE_VERBOSE(msg, ...) UE_LOG(KBELog, Verbose, msg, __VA_ARGS__);

// 打印输出
inline void PrintString(FString name)
{
	KBE_DISPLAY(TEXT("%s"), *FString::Printf(TEXT("\n%s\n"), *name));
}

inline void PrintInteger(int32 value)
{
	KBE_DISPLAY(TEXT("%s"), *FString::Printf(TEXT("\n%d\n"), value));
}

inline void PrintInteger64(int64 value)
{
	KBE_DISPLAY(TEXT("%s"), *FString::Printf(TEXT("\n%llu\n"), value));
}

// 从缓存的指定位置读取T类型的值。
template <typename T>
T read(const uint8* buffer, uint32 pos)
{
	// 把buffer当作数组首地址，buffer[pos]得到数组元素的值, 然后&取得地址，
	// (T const *)把地址转为指定类型，然后*取值
	T val = *((T const*)&buffer[pos]);

	//EndianConvert(val);
	return val;
}


// 以16进制从指定位置输出缓存的字节
void hexlike(const uint8* buffer, uint32 startPos, uint32 size);