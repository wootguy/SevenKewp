#include "Packet.h"
#include <string.h>

using namespace std;

Packet::Packet(const Packet& other) {
	this->addr = other.addr;
	this->sz = other.sz;
	this->data = new char[sz];
	memcpy(this->data, other.data, sz);
}

Packet::Packet(const string& message)
{
	init(message);
}

Packet::Packet(const string& message, IPV4 addr) : addr(addr)
{
	init(message);
}

Packet::Packet(IPV4 addr, const void * data, int sz) : addr(addr), sz(sz)
{
	this->data = new char[sz];
	memcpy(this->data, data, sz);
}

Packet::Packet(const void * data, int sz) : sz(sz)
{
	this->data = new char[sz];
	memcpy(this->data, data, sz);
}

Packet::Packet()
{
	data = NULL;
	sz = -1;
}

Packet::~Packet()
{
	if (data != NULL)
		delete [] data;
}

string Packet::getString() {
	char* outDat = new char[sz + 1];
	memcpy(outDat, data, sz);
	outDat[sz] = 0;

	string outStr = outDat;
	delete[] outDat;

	return outStr;
}

void Packet::init(const string& message)
{
	sz = message.size();
	data = new char[sz+1];
	memcpy(data, (char*)&message[0], sz);
	data[sz] = 0;
	sz++;
}