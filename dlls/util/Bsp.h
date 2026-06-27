#pragma once
#include "vector.h"
#include "bsplimits.h"
#include <string.h>
#include "bsptypes.h"
#include <streambuf>
#include "HashMap.h"
#include "rgb.h"

class Entity;

struct membuf : std::streambuf
{
	membuf(char* begin, int len) {
		this->setg(begin, begin, begin + len);
	}
};

struct ModTraceResult
{
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	Vector	vecEndPos;			// final position
	float	flPlaneDist;
	Vector	vecPlaneNormal;		// surface normal at impact
	//edict_t* pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part

	int iNode;					// node that clipped the trace
};

struct FaceExtents {
	int32_t emins[2];	// face extents in lightmap space
	int32_t emaxs[2];	// face extents in lightmap space
	int32_t mins[2];	// face extents
	int32_t maxs[2];	// face extents
	uint8_t size[2];	// lightmap size
};

class EXPORT Bsp
{
public:
	BSPHEADER header = BSPHEADER();
	uint8_t* lumps[HEADER_LUMPS];
	bool valid;
	bool loaded;

	StringMap* ents;
	int numEnts;

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

	FaceExtents* faceExtents;

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

	int32_t pointContents(int iNode, Vector p, int hull);

	bool traceHull(Vector start, Vector end, int hull, ModTraceResult* trace);

	int traceFace(Vector start, Vector end, int& u, int& v);

	RGB get_lighting(Vector pos);

	static void parseEntities(const char* data, int dataLen, std::vector<StringMap>& outputEnts);

private:
	void update_lump_pointers();
	bool recursiveHullCheck(int hull, int num, float p1f, float p2f, Vector p1, Vector p2, ModTraceResult* trace);
	void precalculate_face_extents();
	void calculate_face_extents(int facenum, int mins_out[2], int maxs_out[2]);
	float CalculatePointVecsProduct(const volatile float* point, const volatile float* vecs);

	static void parse_keyvalue(const std::string& line, std::string& key, std::string& value);
};
