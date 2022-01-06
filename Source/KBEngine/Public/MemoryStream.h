#pragma once

#include <vector>
#include <string>

#define EndianConvert(value)
#define EndianConvertReverse(value)

namespace KBEngine
{
	class KBENGINE_API MemoryStreamException
	{
	public:
		MemoryStreamException(bool _add, size_t _pos, size_t _esize, size_t _size);

		void PrintPosError();

	private:
		bool 		_m_add;
		size_t 		_m_pos;
		size_t 		_m_esize;
		size_t 		_m_size;
	};

	/*
	二进制数据流模块
	能够将一些基本类型序列化(writeXXX)成二进制流同时也提供了反序列化(readXXX)等操作
	*/
	class MemoryStream
	{
	public:
		const static size_t BUFFER_MAX = 1460 * 4;

		union PackFloatXType
		{
			float fv;
			uint32 uv;
			int32 iv;
		};

		MemoryStream()
			: rpos_(0), wpos_(0)
		{
			data_.resize(BUFFER_MAX);
		}

		explicit MemoryStream(size_t res)
			: rpos_(0), wpos_(0)
		{
			if (res > 0)
				data_.resize(res);
		}

		MemoryStream(const MemoryStream &buf)
			: rpos_(buf.rpos_), wpos_(buf.wpos_), data_(buf.data_)
		{
		}

		virtual ~MemoryStream()
		{
		}

		uint8 *Data() { return &data_[0]; }
		const uint8 *Data() const { return &data_[0]; }

		size_t RPos() const     { return rpos_; }
		void   RPos(size_t pos) { rpos_ = pos; }
		size_t WPos() const     { return wpos_; }
		void   WPos(size_t pos) { wpos_ = pos; }

		//---------------------------------------------------------------------------------
		template <typename T> T Read()
		{
			T r = Read<T>(rpos_);
			rpos_ += sizeof(T);
			return r;
		}

		template <typename T> T Read(size_t pos) const
		{
			if (pos + sizeof(T) > WPos())
				throw MemoryStreamException(false, pos, sizeof(T), Length());

			T val = *((T const*)&data_[pos]);
			EndianConvert(val);
			return val;
		}

		void Read(uint8 *dest, size_t len)
		{
			if (len > Length())
				throw MemoryStreamException(false, rpos_, len, Length());

			memcpy(dest, &data_[rpos_], len);
			rpos_ += len;
		}

		int8   ReadInt8()   { return Read<int8>(); }
		int16  ReadInt16()  { return Read<int16>(); }
		int32  ReadInt32()  { return Read<int32>(); }
		int64  ReadInt64()  { return Read<int64>(); }
		uint8  ReadUint8()  { return Read<uint8>(); }
		uint16 ReadUint16() { return Read<uint16>(); }
		uint32 ReadUint32() { return Read<uint32>(); }
		uint64 ReadUint64() { return Read<uint64>(); }
		float  ReadFloat()  { return Read<float>(); }
		double ReadDouble() { return Read<double>(); }

		std::string ReadStdString();
		FString ReadString();	//ansi string
		FString ReadUTF8();		//utf-8 string
		uint32 ReadBlob(std::string &datas);
		uint32 ReadBlob(TArray<uint8> &bytes);

		void ReadPackXYZ(float& x, float&y, float& z, float minf = -256.f);
		void ReadPackXZ(float& x, float& z);
		void ReadPackY(float& y);

		FVector2D ReadPackXZ()
		{
			FVector2D vec;
			ReadPackXZ(vec.X, vec.Y);
			return vec;
		}

		float ReadPackY()
		{
			float y;
			ReadPackY(y);
			return y;
		}

		//---------------------------------------------------------------------------------
		template <typename T> void Append(T value)
		{
			EndianConvert(value);
			Append((uint8 *)&value, sizeof(value));
		}

		void Append(const uint8 *src, size_t cnt)
		{
			if (!cnt)
				return;

			check(Size() < 10000000);

			if (data_.size() < wpos_ + cnt)
				data_.resize(wpos_ + cnt);

			memcpy(&data_[wpos_], src, cnt);
			wpos_ += cnt;
		}

		void Append(const char *src, size_t cnt) { Append((const uint8 *)src, cnt); }

		void WriteInt8(int8 v)     { Append<int8>(v); }
		void WriteInt16(int16 v)   { Append<int16>(v); }
		void WriteInt32(int32 v)   { Append<int32>(v); }
		void WriteInt64(int64 v)   { Append<int64>(v); }
		void WriteUint8(uint8 v)   { Append<uint8>(v); }
		void WriteUint16(uint16 v) { Append<uint16>(v); }
		void WriteUint32(uint32 v) { Append<uint32>(v); }
		void WriteUint64(uint64 v) { Append<uint64>(v); }
		void WriteFloat(float v)   { Append<float>(v); }
		void WriteDouble(double v) { Append<double>(v); }

		void WriteBlob(const char *src, uint32 cnt)
		{
			WriteUint32(cnt);

			if (cnt > 0)
			{
				Append(src, cnt);
			}
		}

		void WriteBlob(const std::string& bytes) { WriteBlob(bytes.data(), (uint32)bytes.size()); }
		void WriteBlob(const TArray<uint8>& bytes) { WriteBlob((const char *)bytes.GetData(), (uint32)bytes.Num()); }

		void WriteStdString(const std::string& v)
		{
			Append(v.data(), v.length());
			WriteInt8(0);
		}

		void WriteString(const FString &v)		//ansi string
		{
			auto s = StringCast<ANSICHAR>(*v);
			Append(s.Get(), s.Length());
			WriteInt8(0);
		}

		void WriteUTF8(const FString &v)		//utf-8 string
		{
			FTCHARToUTF8 utf8(*v);
			WriteBlob(utf8.Get(), utf8.Length());
		}

		//---------------------------------------------------------------------------------
		template<typename T>
		void ReadSkip() { ReadSkip(sizeof(T)); }

		void ReadSkip(size_t skip)
		{
			if (skip > Length())
				throw MemoryStreamException(false, rpos_, skip, Length());

			rpos_ += skip;
		}

		//---------------------------------------------------------------------------------
		virtual size_t Size() const { return data_.size(); }

		virtual size_t Space() const { return WPos() >= Size() ? 0 : Size() - WPos(); }

		virtual size_t Length() const { return RPos() >= WPos() ? 0 : WPos() - RPos(); }

		bool ReadEOF() const { return (Size() - RPos()) <= 0; }

		void Done() { ReadSkip(Length()); }

		//---------------------------------------------------------------------------------
		void Clear()
		{
			rpos_ = wpos_ = 0;

			if (data_.size() > BUFFER_MAX)
				data_.resize(BUFFER_MAX);
		}

		//---------------------------------------------------------------------------------
		void GetBuffer(std::vector<uint8> &buf) const
		{
			buf.resize(Length());
			memcpy(buf.data(), data_.data() + RPos(), Length());
		}

		private:
			mutable size_t rpos_, wpos_;
			std::vector<uint8> data_;

	};
}
