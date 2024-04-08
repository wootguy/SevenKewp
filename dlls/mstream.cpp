#include "extdll.h"
#include "mstream.h"
#include "stdio.h"
#include "string.h"
#include "util.h"

mstream::mstream()
{
	currentBit = start = end = pos = 0;
	eomFlag = true;
}

mstream::mstream(char * buf, uint64_t len)
{
	start = (uint64_t)buf;
	end = start + len;
	pos = start;
	eomFlag = false;
}

mstream::mstream( uint64_t len )
{
	start = (uint64_t)new char[len];
	end = start + len;
	pos = start;
	eomFlag = false;
}

uint64_t mstream::read( void * dest, uint64_t bytes )
{
	if (eomFlag)
		return 0;
	uint64_t newpos = pos + bytes;
	if (newpos > end || newpos < start)
	{
		eomFlag = true;
		bytes = end - pos;
	}
	memcpy(dest, (void*)pos, bytes);
	pos = newpos;

	return bytes;
}

uint32_t mstream::readBit()
{
	if (eomFlag)
		return UINT32_MAX;

	if (currentBit >= 8) {
		if (pos + 1 >= end) {
			eomFlag = true;
			return UINT32_MAX;
		}

		pos++;
		currentBit = 0;
	}

	return (*((uint8_t*)pos) & (1 << currentBit++)) != 0;
}

uint32_t mstream::readBits(uint8_t bitCount)
{
	uint32_t val = 0;

	for (int i = 0; i < bitCount; i++) {
		// TODO: read a byte if position and bitcount allow
		val |= readBit() << i;
	}

	return val;
}

Vector mstream::readBitVec3Coord() {
	bool xflag = readBit();
	bool yflag = readBit();
	bool zflag = readBit();

	Vector vout = g_vecZero;

	if (xflag)
		vout[0] = readBitCoord();
	if (yflag)
		vout[1] = readBitCoord();
	if (zflag)
		vout[2] = readBitCoord();

	return vout;
}

float mstream::readBitCoord() {
	bool hasIntVal = readBit();
	bool hasFracVal = readBit();

	if (hasIntVal || hasFracVal) {
		int signBit = readBit();
		int intVal = hasIntVal ? readBits(12) : 0;
		int fracVal = hasFracVal ? readBits(3) : 0;

		float ret = (float)intVal + ((float)fracVal / 8.0f);
		return signBit ? -ret : ret;
	}

	return 0;
}

uint64_t mstream::write( void * src, uint64_t bytes )
{
	if (eomFlag)
		return 0;
	uint64_t newpos = pos + bytes;
	if (newpos > end || newpos < start)
	{
		eomFlag = true;
		return 0;
	}
	memcpy((void*)pos, src, bytes);
	pos = newpos;

	return bytes;
}

bool mstream::writeBit(bool value) {
	if (eomFlag)
		return 0;

	if (currentBit >= 8) {
		if (pos + 1 >= end) {
			eomFlag = true;
			return 0;
		}
		
		pos++;
		currentBit = 0;
	}

	*((uint8_t*)pos) |= (value ? 1 : 0) << currentBit;

	currentBit++;
	return 1;
}

uint8_t mstream::writeBits(uint32_t value, uint8_t bitCount) {
	for (int i = 0; i < bitCount; i++) {
		// TODO: write a byte if position and bitcount allow

		if (!writeBit(value & 1)) {
			return i;
		}

		value >>= 1;
	}

	return bitCount;
}

bool mstream::writeBitCoord(const float f) {
	int signbit = f <= -0.125;
	int intval = abs((int32)f);
	int fractval = (int32_t)abs(f * 8) & 7;

	writeBit(intval);
	writeBit(fractval);

	if (intval || fractval)
	{
		writeBit(signbit);
		if (intval)
			writeBits(intval, 12);
		if (fractval)
			writeBits(fractval, 3);
	}

	return !eom();
}

bool mstream::writeBitVec3Coord(const float* fa) {
	bool xflag = fa[0] <= -0.125 || fa[0] >= 0.125;
	bool yflag = fa[1] <= -0.125 || fa[1] >= 0.125;
	bool zflag = fa[2] <= -0.125 || fa[2] >= 0.125;

	writeBit(xflag);
	writeBit(yflag);
	writeBit(zflag);

	if (xflag)
		writeBitCoord(fa[0]);
	if (yflag)
		writeBitCoord(fa[1]);
	if (zflag)
		writeBitCoord(fa[2]);

	return !eom();
}

void mstream::seek( uint64_t to )
{
	pos = start + to;
	currentBit = 0;
	eomFlag = false;
	if (pos < start) {
		pos = start;
		eomFlag = true;
	}
	if (pos >= end) {
		pos = end;
		eomFlag = true;
	}
}

void mstream::seek( uint64_t to, int whence )
{
	switch(whence)
	{
	case (SEEK_SET):
		pos = start + to;
		break;
	case (SEEK_CUR):
		pos += to;
		break;
	case (SEEK_END):
		pos = end + to;
		break;
	}
	currentBit = 0;
	eomFlag = false;
	if (pos >= end || pos < start)
		eomFlag = true;
}

uint64_t mstream::skip( uint64_t bytes )
{
	if (eomFlag)
		return 0;
	uint64_t newpos = pos + bytes;
	if (newpos >= end || newpos < start)
	{
		bytes = end - pos;
		eomFlag = true;
	}
	pos = newpos;
	currentBit = 0;
	return bytes;
}

uint64_t mstream::tell()
{
	return pos - start;
}

char * mstream::getBuffer()
{
	return (char*)start;
}

char* mstream::getOffsetBuffer() {
	return (char*)start + pos;
}

bool mstream::eom()
{
	return eomFlag;
}

void mstream::freeBuf()
{
	delete [] (char*)start;
}

mstream::~mstream( void )
{

}

uint64_t mstream::size()
{
	return end - start;
}
