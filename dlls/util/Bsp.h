#pragma once
#include "extdll.h"
#include "vector.h"
#include "bsplimits.h"
#include <string.h>
#include "bsptypes.h"
#include <streambuf>
#include "HashMap.h"

class Entity;

struct membuf : std::streambuf
{
	membuf(char* begin, int len) {
		this->setg(begin, begin, begin + len);
	}
};

class Bsp
{
public:
	std::string path;
	BSPHEADER header = BSPHEADER();
	uint8_t* lumps[HEADER_LUMPS];
	bool valid;
	bool loaded;

	std::vector<StringMap> ents;

	BSPPLANE* planes;
	BSPTEXTUREINFO* texinfos;
	uint8_t* textures;
	BSPLEAF* leaves;
	BSPMODEL* models;
	BSPNODE* nodes;
	BSPCLIPNODE* clipnodes;
	BSPFACE* faces;
	Vector* verts;
	uint8_t* lightdata;
	int32_t* surfedges;
	BSPEDGE* edges;
	uint16* marksurfs;
	uint8_t* visdata;

	int planeCount;
	int texinfoCount;
	int leafCount;
	int modelCount;
	int nodeCount;
	int vertCount;
	int faceCount;
	int clipnodeCount;
	int marksurfCount;
	int surfedgeCount;
	int edgeCount;
	int textureCount;
	int lightDataLength;
	int visDataLength;

	int entityBspModelCount;

	Bsp();

	bool load_lumps(std::string fname);
	void delete_lumps();

	EXPORT static void parseEntities(const char* data, int dataLen, std::vector<StringMap>& outputEnts);

private:
	void update_lump_pointers();

	static void parse_keyvalue(const std::string& line, std::string& key, std::string& value);
};
