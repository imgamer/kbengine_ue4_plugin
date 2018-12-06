#include "MemoryStream.h"
#include "KBEnginePrivatePCH.h"


namespace KBEngine
{
	MemoryStreamException::MemoryStreamException(bool _add, size_t _pos, size_t _esize, size_t _size)
		: _m_add(_add), _m_pos(_pos), _m_esize(_esize), _m_size(_size)
	{
		PrintPosError();
	}

	void MemoryStreamException::PrintPosError()
	{
		KBE_ERROR(TEXT("Attempted to %s in MemoryStream (pos:%d  size: %d).\n"),
			(_m_add ? TEXT("put") : TEXT("get")), _m_pos, _m_size);
	}





	std::string MemoryStream::ReadStdString()
	{
		size_t offset = rpos_;
		while (ReadUint8() != 0)
		{
		}

		return std::string((const char *)(Data() + offset), rpos_ - offset - 1);
	}

	FString MemoryStream::ReadString()	//ansi string
	{
		size_t offset = rpos_;
		while (ReadUint8() != 0)
		{
		}

		auto s = StringCast<TCHAR>((const ANSICHAR *)(Data() + offset));
		return FString(s.Length(), s.Get());
	}

	FString MemoryStream::ReadUTF8()		//utf-8 string
	{
		if (Length() <= 0)
			return FString();

		uint32 rsize = ReadUint32();
		if ((size_t)rsize > Length())
			return FString();

		if (rsize > 0)
		{
			FUTF8ToTCHAR tcs((const ANSICHAR *)(Data() + rpos_), rsize);
			ReadSkip(rsize);
			return FString(tcs.Length(), tcs.Get());
		}

		return FString();
	}

	uint32 MemoryStream::ReadBlob(std::string &datas)
	{
		if (Length() <= 0)
			return 0;

		uint32 rsize = ReadUint32();
		if ((size_t)rsize > Length())
			return 0;

		if (rsize > 0)
		{
			datas.assign((char*)(Data() + rpos_), rsize);
			ReadSkip(rsize);
		}

		return rsize;
	}

	uint32 MemoryStream::ReadBlob(TArray<uint8> &bytes)
	{
		if (Length() <= 0)
			return 0;

		uint32 rsize = ReadUint32();
		if ((size_t)rsize > Length())
			return 0;

		if (rsize > 0)
		{
			bytes.SetNumUninitialized(rsize);
			memcpy(bytes.GetData(), Data() + rpos_, rsize);
			ReadSkip(rsize);
		}

		return rsize;
	}

	void MemoryStream::ReadPackXYZ(float& x, float&y, float& z, float minf)
	{
		uint32 packed = ReadUint32();
		x = ((packed & 0x7FF) << 21 >> 21) * 0.25f;
		z = ((((packed >> 11) & 0x7FF) << 21) >> 21) * 0.25f;
		y = ((packed >> 22 << 22) >> 22) * 0.25f;

		x += minf;
		y += minf / 2.f;
		z += minf;
	}

	void MemoryStream::ReadPackXZ(float& x, float& z)
	{
		PackFloatXType & xPackData = (PackFloatXType&)x;
		PackFloatXType & zPackData = (PackFloatXType&)z;

		// 0x40000000 = 1000000000000000000000000000000.
		xPackData.uv = 0x40000000;
		zPackData.uv = 0x40000000;

		uint8 tv;
		uint32 data = 0;

		tv = ReadUint8();
		data |= (tv << 16);

		tv = ReadUint8();
		data |= (tv << 8);

		tv = ReadUint8();
		data |= tv;

		// 复制指数和尾数
		xPackData.uv |= (data & 0x7ff000) << 3;
		zPackData.uv |= (data & 0x0007ff) << 15;

		xPackData.fv -= 2.0f;
		zPackData.fv -= 2.0f;

		// 设置标记位
		xPackData.uv |= (data & 0x800000) << 8;
		zPackData.uv |= (data & 0x000800) << 20;
	}

	void MemoryStream::ReadPackY(float& y)
	{
		PackFloatXType yPackData;
		yPackData.uv = 0x40000000;

		uint16 data = ReadUint16();
		yPackData.uv |= (data & 0x7fff) << 12;
		yPackData.fv -= 2.f;
		yPackData.uv |= (data & 0x8000) << 16;
		y = yPackData.fv;
	}

}
