// Fill out your copyright notice in the Description page of Project Settings.
#include "KBEDebug.h"
#include "KBEnginePrivatePCH.h"
#include <string>

//DEFINE_LOG_CATEGORY(KBELog)


// 以16进制从指定位置输出缓存的字节
void hexlike(const uint8* buffer, uint32 startPos, uint32 size)
{
	uint32 j = 1, k = 1;
	char buf[1024];
	std::string fbuffer;
	uint32 endPos = startPos + size;

	_snprintf(buf, 1024, "STORAGE_SIZE: startPos=%lu. endPos=%lu, \n", (unsigned long)startPos, (unsigned long)endPos);
	fbuffer += buf;

	uint32 i = 0;
	for (uint32 idx = startPos; idx < endPos; ++idx)
	{
		++i;
		if ((i == (j * 8)) && ((i != (k * 16))))
		{
			if (read<uint8>(buffer, idx) < 0x10)
			{
				_snprintf(buf, 1024, "| 0%X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}
			else
			{
				_snprintf(buf, 1024, "| %X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}
			++j;
		}
		else if (i == (k * 16))
		{
			if (read<uint8>(buffer, idx) < 0x10)
			{
				_snprintf(buf, 1024, "\n0%X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}
			else
			{
				_snprintf(buf, 1024, "\n%X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}

			++k;
			++j;
		}
		else
		{
			if (read<uint8>(buffer, idx) < 0x10)
			{
				_snprintf(buf, 1024, "0%X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}
			else
			{
				_snprintf(buf, 1024, "%X ", read<uint8>(buffer, idx));
				fbuffer += buf;
			}
		}
	}

	fbuffer += "\n";

	KBE_ERROR(TEXT("%s"), *FString(fbuffer.c_str()));
}
