#include "Bsp.h"
#include "util.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdint>

using namespace std;

Bsp::Bsp() {
	header.nVersion = 30;
	loaded = valid = false;

	for (int i = 0; i < HEADER_LUMPS; i++) {
		lumps[i] = NULL;
		header.lump[i].nLength = 0;
	}

	update_lump_pointers();
}

void Bsp::delete_lumps() {
	loaded = false;
	for (int i = 0; i < HEADER_LUMPS; i++) {
		if (lumps[i]) {
			delete[] lumps[i];
			lumps[i] = NULL;
		}
	}
}

bool Bsp::load_lumps(string fpath)
{
	loaded = true;
	valid = true;

	if (!fileExists(fpath.c_str())) {
		ALERT(at_error, "BSP load failed. %s not found\n", fpath.c_str());
		valid = false;
		return false;
	}

	// Read all BSP Data
	ifstream fin(fpath, ios::binary | ios::ate);
	int size = fin.tellg();
	fin.seekg(0, fin.beg);

	if (size < (int)(sizeof(BSPHEADER) + sizeof(BSPLUMP) * HEADER_LUMPS)) {
		update_lump_pointers();
		valid = false;
		return false;
	}

	fin.read((char*)&header.nVersion, sizeof(int));

	
	for (int i = 0; i < HEADER_LUMPS; i++)
	{
		fin.read((char*)&header.lump[i], sizeof(BSPLUMP));
		//ALERT(at_console, "Read lump id: %d. Len: %d. Offset %d.\n", i,header.lump[i].nLength,header.lump[i].nOffset);
	}
	
	for (int i = 0; i < HEADER_LUMPS; i++)
	{
		if (header.lump[i].nLength == 0) {
			lumps[i] = NULL;
			continue;
		}

		fin.seekg(header.lump[i].nOffset);
		if (fin.eof()) {
			ALERT(at_error, "FAILED TO READ BSP LUMP %d\n", i);
			valid = false;
			continue;
		}
		else
		{
			lumps[i] = new uint8_t[header.lump[i].nLength];
			fin.read((char*)lumps[i], header.lump[i].nLength);
		}
	}	
	
	fin.close();

	update_lump_pointers();

	count_entity_bsp_models();

	return valid;
}

void Bsp::update_lump_pointers() {
	planes = (BSPPLANE*)lumps[LUMP_PLANES];
	texinfos = (BSPTEXTUREINFO*)lumps[LUMP_TEXINFO];
	leaves = (BSPLEAF*)lumps[LUMP_LEAVES];
	models = (BSPMODEL*)lumps[LUMP_MODELS];
	nodes = (BSPNODE*)lumps[LUMP_NODES];
	clipnodes = (BSPCLIPNODE*)lumps[LUMP_CLIPNODES];
	faces = (BSPFACE*)lumps[LUMP_FACES];
	verts = (Vector*)lumps[LUMP_VERTICES];
	lightdata = lumps[LUMP_LIGHTING];
	surfedges = (int32_t*)lumps[LUMP_SURFEDGES];
	edges = (BSPEDGE*)lumps[LUMP_EDGES];
	marksurfs = (uint16*)lumps[LUMP_MARKSURFACES];
	visdata = lumps[LUMP_VISIBILITY];
	textures = lumps[LUMP_TEXTURES];

	planeCount = header.lump[LUMP_PLANES].nLength / sizeof(BSPPLANE);
	texinfoCount = header.lump[LUMP_TEXINFO].nLength / sizeof(BSPTEXTUREINFO);
	leafCount = header.lump[LUMP_LEAVES].nLength / sizeof(BSPLEAF);
	modelCount = header.lump[LUMP_MODELS].nLength / sizeof(BSPMODEL);
	nodeCount = header.lump[LUMP_NODES].nLength / sizeof(BSPNODE);
	vertCount = header.lump[LUMP_VERTICES].nLength / sizeof(Vector);
	faceCount = header.lump[LUMP_FACES].nLength / sizeof(BSPFACE);
	clipnodeCount = header.lump[LUMP_CLIPNODES].nLength / sizeof(BSPCLIPNODE);
	marksurfCount = header.lump[LUMP_MARKSURFACES].nLength / sizeof(uint16_t);
	surfedgeCount = header.lump[LUMP_SURFEDGES].nLength / sizeof(int32_t);
	edgeCount = header.lump[LUMP_EDGES].nLength / sizeof(BSPEDGE);
	textureCount = lumps[LUMP_TEXTURES] ? *((int32_t*)(lumps[LUMP_TEXTURES])) : 0;
	lightDataLength = header.lump[LUMP_LIGHTING].nLength;
	visDataLength = header.lump[LUMP_VISIBILITY].nLength;

	if (planeCount > MAX_MAP_PLANES) ALERT(at_console, "Overflowed Planes !!!\n");
	if (texinfoCount > MAX_MAP_TEXINFOS) ALERT(at_console, "Overflowed texinfos !!!\n");
	if (leafCount > MAX_MAP_LEAVES) ALERT(at_console, "Overflowed leaves !!!\n");
	if (modelCount > MAX_MAP_MODELS) ALERT(at_console, "Overflowed models !!!\n");
	if (texinfoCount > MAX_MAP_TEXINFOS) ALERT(at_console, "Overflowed texinfos !!!\n");
	if (nodeCount > MAX_MAP_NODES) ALERT(at_console, "Overflowed nodes !!!\n");
	if (vertCount > MAX_MAP_VERTS) ALERT(at_console, "Overflowed verts !!!\n");
	if (faceCount > MAX_MAP_FACES) ALERT(at_console, "Overflowed faces !!!\n");
	if (clipnodeCount > MAX_MAP_CLIPNODES) ALERT(at_console, "Overflowed clipnodes !!!\n");
	if (marksurfCount > MAX_MAP_MARKSURFS) ALERT(at_console, "Overflowed marksurfs !!!\n");
	if (surfedgeCount > MAX_MAP_SURFEDGES) ALERT(at_console, "Overflowed surfedges !!!\n");
	if (edgeCount > MAX_MAP_EDGES) ALERT(at_console, "Overflowed edges !!!\n");
	if (textureCount > MAX_MAP_TEXTURES) ALERT(at_console, "Overflowed textures !!!\n");
	if (lightDataLength > MAX_MAP_LIGHTDATA) ALERT(at_console, "Overflowed lightdata !!!\n");
	if (visDataLength > MAX_MAP_VISDATA) ALERT(at_console, "Overflowed visdata !!!\n");
}

void Bsp::count_entity_bsp_models()
{
	membuf sbuf((char*)lumps[LUMP_ENTITIES], header.lump[LUMP_ENTITIES].nLength);
	istream in(&sbuf);

	int lineNum = 0;
	int lastBracket = -1;

	StringSet unique_bsp_models;

	string line = "";
	while (getline(in, line))
	{
		lineNum++;
		if (line.length() < 1 || line[0] == '\n')
			continue;

		if (line[0] == '{')
		{
			if (lastBracket == 0)
			{
				ALERT(at_console, "%s.bsp ent data (line %d): Unexpected '{'\n", path.c_str(), lineNum);
				continue;
			}
			lastBracket = 0;
		}
		else if (line[0] == '}')
		{
			if (lastBracket == 1)
				ALERT(at_console, "%s.bsp ent data (line %d): Unexpected '}'\n", path.c_str(), lineNum);
			lastBracket = 1;

			// you can end/start an ent on the same line, you know
			if (line.find("{") != string::npos)
			{
				lastBracket = 0;
			}
		}
		else if (lastBracket == 0) // currently defining an entity
		{
			string key, value;
			parse_keyvalue(line, key, value);
			if (key == "model" && value[0] == '*') {
				unique_bsp_models.put(value.c_str());
			}
		}
	}

	entityBspModelCount = unique_bsp_models.size();
}

void Bsp::parse_keyvalue(const std::string& line, std::string& key, std::string& value) {
	int begin = -1;
	int end = -1;

	key = "";
	value = "";
	int comment = 0;

	for (uint32_t i = 0; i < line.length(); i++)
	{
		if (line[i] == '/')
		{
			if (++comment >= 2)
			{
				key = value = "";
				break;
			}
		}
		else
			comment = 0;
		if (line[i] == '"')
		{
			if (begin == -1)
				begin = i + 1;
			else
			{
				end = i;
				if (key.length() == 0)
				{
					key = line.substr(begin, end - begin);
					begin = end = -1;
				}
				else
				{
					value = line.substr(begin, end - begin);
					break;
				}
			}
		}
	}
}
