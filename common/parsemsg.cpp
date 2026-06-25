/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  parsemsg.cpp
//
//--------------------------------------------------------------------------------------------------------------
#include "parsemsg.h"
#include <port.h>
#include "mstream.h"

typedef unsigned char byte;
#define true 1

static byte *gpBuf;
static int giSize;
static int giRead;
static int giBadRead;

mstream g_msg_bit_reader;

int READ_OK( void )
{
	return !giBadRead;
}

void BEGIN_READ( void *buf, int size )
{
	giRead = 0;
	giBadRead = 0;
	giSize = size;
	gpBuf = (byte*)buf;
	g_msg_bit_reader = mstream((char*)buf, size);
}


int READ_CHAR( void )
{
	int     c;
	
	if (giRead + 1 > giSize)
	{
		giBadRead = true;
		return -1;
	}
		
	c = (signed char)gpBuf[giRead];
	giRead++;
	
	return c;
}

int READ_BYTE( void )
{
	int     c;
	
	if (giRead+1 > giSize)
	{
		giBadRead = true;
		return -1;
	}
		
	c = (unsigned char)gpBuf[giRead];
	giRead++;
	
	return c;
}

void READ_BYTES(uint8_t* bytes, int count) {
	int longCount = count / 4;
	int byteCount = count % 4;

	int32_t* longPtr = (int32_t*)bytes;
	for (int i = 0; i < longCount; i++) {
		*longPtr = READ_LONG();
		longPtr++;
	}

	uint8_t* bytePtr = bytes + longCount * 4;
	for (int i = 0; i < byteCount; i++) {
		*bytePtr = READ_BYTE();
		bytePtr++;
	}
}

uint64_t READ_BITS(int numBits) {
	return g_msg_bit_reader.readBits(numBits);
}

bool READ_BIT() {
	return g_msg_bit_reader.readBit();
}

float READ_FLOAT_LOWP(int numBits) {
	if (numBits == 10) {
		int sign = READ_BIT() ? -1 : 1;
		int power = READ_BITS(2);
		int mantissa = READ_BITS(7);

		switch (power) {
		default:
		case 0: return mantissa * sign * 0.1f;
		case 1: return mantissa * sign;
		case 2: return mantissa * sign * 10;
		case 3: return mantissa * sign * 100;
		}
	}
	else {
		return 0;
	}
}

Vector READ_VECTOR_LOWP(int numBits) {
	if (numBits == 10) {
		Vector vec;
		
		if (READ_BIT()) {
			vec.x = READ_FLOAT_LOWP(numBits);
		}
		if (READ_BIT()) {
			vec.y = READ_FLOAT_LOWP(numBits);
		}
		if (READ_BIT()) {
			vec.z = READ_FLOAT_LOWP(numBits);
		}

		return vec;
	}
	else {
		return Vector(0,0,0);
	}
}

int READ_SHORT( void )
{
	int     c;
	
	if (giRead+2 > giSize)
	{
		giBadRead = true;
		return -1;
	}
		
	c = (short)( gpBuf[giRead] + ( gpBuf[giRead+1] << 8 ) );
	
	giRead += 2;
	
	return c;
}

int READ_WORD( void )
{
	return READ_SHORT();
}


int READ_LONG( void )
{
	int     c;
	
	if (giRead+4 > giSize)
	{
		giBadRead = true;
		return -1;
	}
		
 	c = gpBuf[giRead] + (gpBuf[giRead + 1] << 8) + (gpBuf[giRead + 2] << 16) + (gpBuf[giRead + 3] << 24);
	
	giRead += 4;
	
	return c;
}

float READ_FLOAT( void )
{
	union
	{
		byte    b[4];
		float   f;
		int     l;
	} dat;
	
	dat.b[0] = gpBuf[giRead];
	dat.b[1] = gpBuf[giRead+1];
	dat.b[2] = gpBuf[giRead+2];
	dat.b[3] = gpBuf[giRead+3];
	giRead += 4;
	
//	dat.l = LittleLong (dat.l);

	return dat.f;   
}

char* READ_STRING( void )
{
	static char     string[2048];
	int             l,c;

	string[0] = 0;

	l = 0;
	do
	{
		if ( giRead+1 > giSize )
			break; // no more characters

		c = READ_CHAR();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

float READ_COORD( void )
{
	return (float)(READ_SHORT() * (1.0/8));
}

float READ_ANGLE( void )
{
	return (float)(READ_CHAR() * (360.0/256));
}

float READ_HIRESANGLE( void )
{
	return (float)(READ_SHORT() * (360.0/65536));
}

//--------------------------------------------------------------------------------------------------------------
BufferWriter::BufferWriter()
{
	Init( NULL, 0 );
}

//--------------------------------------------------------------------------------------------------------------
BufferWriter::BufferWriter( unsigned char *buffer, int bufferLen )
{
	Init( buffer, bufferLen );
}

//--------------------------------------------------------------------------------------------------------------
void BufferWriter::Init( unsigned char *buffer, int bufferLen )
{
	m_overflow = false;
	m_buffer = buffer;
	m_remaining = bufferLen;
	m_overallLength = bufferLen;
}

//--------------------------------------------------------------------------------------------------------------
void BufferWriter::WriteByte( unsigned char data )
{
	if (!m_buffer || !m_remaining)
	{
		m_overflow = true;
		return;
	}

	*m_buffer = data;
	++m_buffer;
	--m_remaining;
}

//--------------------------------------------------------------------------------------------------------------
void BufferWriter::WriteLong( int data )
{
	if (!m_buffer || m_remaining < 4)
	{
		m_overflow = true;
		return;
	}

	m_buffer[0] = data&0xff;
	m_buffer[1] = (data>>8)&0xff;
	m_buffer[2] = (data>>16)&0xff;
	m_buffer[3] = data>>24;
	m_buffer += 4;
	m_remaining -= 4;
}

//--------------------------------------------------------------------------------------------------------------
void BufferWriter::WriteString( const char *str )
{
	if (!m_buffer || !m_remaining)
	{
		m_overflow = true;
		return;
	}

	if (!str)
		str = "";

	int len = strlen(str)+1;
	if ( len > m_remaining )
	{
		m_overflow = true;
		str = "";
		len = 1;
	}

	strcpy((char *)m_buffer, str);
	m_remaining -= len;
	m_buffer += len;
}

//--------------------------------------------------------------------------------------------------------------
int BufferWriter::GetSpaceUsed()
{
	return m_overallLength - m_remaining;
}

//--------------------------------------------------------------------------------------------------------------
