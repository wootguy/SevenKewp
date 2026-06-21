#include "Bsp.h"
#include "util.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdint>

using namespace std;

#define BSP_EPSILON	0.03125f // 1/32 (to keep floating point happy -Carmack)

#define TEXTURE_STEP 16
#define MAX_SURFACE_EXTENTS 16

Bsp::Bsp() {
	header.nVersion = 30;
	loaded = valid = false;

	for (int i = 0; i < HEADER_LUMPS; i++) {
		lumps[i] = NULL;
		header.lump[i].nLength = 0;
	}

	faceExtents = NULL;

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

	if (faceExtents) {
		delete[] faceExtents;
		faceExtents = NULL;
	}
}

bool Bsp::load_lumps(string fpath)
{
	delete_lumps();

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

	parseEntities((char*)lumps[LUMP_ENTITIES], header.lump[LUMP_ENTITIES].nLength, ents);

	StringSet unique_bsp_models;
	for (int i = 0; i < (int)ents.size(); i++) {
		const char* model = ents[i].get("model");

		if (model && model[0] == '*') {
			unique_bsp_models.put(model);
		}
	}
	entityBspModelCount = unique_bsp_models.size();

	precalculate_face_extents();

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

int32_t Bsp::pointContents(int iNode, Vector p, int hull) {
	if (iNode < 0) {
		return hull == 0 ? leaves[~iNode].nContents : iNode;
	}

	if (hull == 0) {
		while (iNode >= 0 && iNode < nodeCount)
		{
			BSPNODE& node = nodes[iNode];
			BSPPLANE& plane = planes[node.iPlane];

			float d = DotProduct(plane.vNormal, p) - plane.fDist;
			if (d < 0) {
				iNode = node.iChildren[1];
			}
			else {
				iNode = node.iChildren[0];
			}
		}

		return leaves[~iNode].nContents;
	}
	else {
		while (iNode >= 0 && iNode < clipnodeCount)
		{
			BSPCLIPNODE& node = clipnodes[iNode];
			BSPPLANE& plane = planes[node.iPlane];

			float d = DotProduct(plane.vNormal, p) - plane.fDist;
			if (d < 0) {
				iNode = node.iChildren[1];
			}
			else {
				iNode = node.iChildren[0];
			}
		}

		return iNode;
	}
}

bool Bsp::recursiveHullCheck(int hull, int num, float p1f, float p2f, Vector p1, Vector p2, ModTraceResult* trace)
{
	if (num < 0) {
		int contents = num;
		if (hull == 0) {
			contents = leaves[~num].nContents;
		}

		if (contents != CONTENTS_SOLID) {
			trace->fAllSolid = false;

			if (contents == CONTENTS_EMPTY)
				trace->fInOpen = true;

			else if (contents != CONTENTS_TRANSLUCENT)
				trace->fInWater = true;
		}
		else {
			trace->fStartSolid = true;
		}

		// empty
		return true;
	}

	if (hull == 0 && num >= nodeCount || hull != 0 && num >= clipnodeCount) {
		ALERT(at_warning, "%s: bad node number\n", __func__);
		return false;
	}

	// find the point distances
	BSPCLIPNODE* node = hull == 0 ? (BSPCLIPNODE*)&nodes[num] : &clipnodes[num];
	BSPPLANE* plane = &planes[node->iPlane];

	//float t1 = dotProduct(plane->vNormal, p1) - plane->fDist;
	//float t2 = dotProduct(plane->vNormal, p2) - plane->fDist;
	float t1, t2;

	switch (plane->nType) {
	case 0:
		t1 = p1.x - plane->fDist;
		t2 = p2.x - plane->fDist;
		break;
	case 1:
		t1 = p1.y - plane->fDist;
		t2 = p2.y - plane->fDist;
		break;
	case 2:
		t1 = p1.z - plane->fDist;
		t2 = p2.z - plane->fDist;
		break;
	default:
		t1 = DotProduct(plane->vNormal, p1) - plane->fDist;
		t2 = DotProduct(plane->vNormal, p2) - plane->fDist;
		break;
	}

	// keep descending until we find a plane that bisects the trace line
	if (t1 >= 0.0f && t2 >= 0.0f)
		return recursiveHullCheck(hull, node->iChildren[0], p1f, p2f, p1, p2, trace);
	if (t1 < 0.0f && t2 < 0.0f)
		return recursiveHullCheck(hull, node->iChildren[1], p1f, p2f, p1, p2, trace);

	int side = (t1 < 0.0f) ? 1 : 0;

	// put the crosspoint DIST_EPSILON pixels on the near side
	float frac;
	if (side) {
		frac = (t1 + BSP_EPSILON) / (t1 - t2);
	}
	else {
		frac = (t1 - BSP_EPSILON) / (t1 - t2);
	}
	frac = clamp(frac, 0.0f, 1.0f);

	if (frac != frac) {
		return false; // NaN
	}

	float pdif = p2f - p1f;
	float midf = p1f + pdif * frac;

	Vector delta = p2 - p1;
	Vector mid = p1 + (delta * frac);

	// check if trace is empty up until this plane that was just intersected
	if (!recursiveHullCheck(hull, node->iChildren[side], p1f, midf, p1, mid, trace)) {
		// hit an earlier plane that caused the trace to be fully solid here
		return false;
	}

	// check if trace can go through this plane without entering a solid area
	if (pointContents(node->iChildren[side ^ 1], mid, hull) != CONTENTS_SOLID) {
		// continue the trace from this plane
		// won't collide with it again because trace starts from a child of the intersected node
		return recursiveHullCheck(hull, node->iChildren[side ^ 1], midf, p2f, mid, p2, trace);
	}

	if (trace->fAllSolid) {
		return false; // never got out of the solid area
	}

	// the other side of the node is solid, this is the impact point
	trace->vecPlaneNormal = plane->vNormal;
	trace->flPlaneDist = side ? -plane->fDist : plane->fDist;
	trace->iNode = num;

	// backup the trace if the collision point is considered solid due to poor float precision
	// shouldn't really happen, but does occasionally
	int headnode = models[0].iHeadnodes[hull];
	while (pointContents(headnode, mid, hull) == CONTENTS_SOLID) {
		frac -= 0.1f;
		if (frac < 0.0f)
		{
			trace->flFraction = midf;
			trace->vecEndPos = mid;
			//debugf("backup past 0\n");
			return false;
		}

		midf = p1f + pdif * frac;

		Vector point = p2 - p1;
		mid = p1 + (point * frac);
	}

	trace->flFraction = midf;
	trace->vecEndPos = mid;

	return false;
}

bool Bsp::traceHull(Vector start, Vector end, int hull, ModTraceResult* trace)
{
	if (hull < 0 || hull > 3)
		hull = 0;

	int headnode = models[0].iHeadnodes[hull];

	// fill in a default trace
	memset(trace, 0, sizeof(TraceResult));
	trace->vecEndPos = end;
	trace->flFraction = 1.0f;
	trace->fAllSolid = true;

	// trace a line through the appropriate clipping hull
	return recursiveHullCheck(hull, headnode, 0.0f, 1.0f, start, end, trace);
}

int Bsp::traceFace(Vector start, Vector end, int& u, int& v) {
	u = v = 0;

	ModTraceResult tr;
	if (traceHull(start, end, 0, &tr)) {
		return -1;
	}

	BSPNODE& node = nodes[tr.iNode];

	for (int i = 0; i < node.nFaces; i++) {
		int faceIdx = node.firstFace + i;
		BSPFACE& face = faces[faceIdx];
		BSPTEXTUREINFO& info = texinfos[face.iTextureInfo];
		FaceExtents& ext = faceExtents[faceIdx];

		int ds = (int)(DotProduct(tr.vecEndPos, info.vS) + info.shiftS);
		int dt = (int)(DotProduct(tr.vecEndPos, info.vT) + info.shiftT);

		if (ds >= ext.mins[0] && dt >= ext.mins[1]
			&& ds - ext.mins[0] <= ext.maxs[0]
			&& dt - ext.mins[1] <= ext.maxs[1])
		{
			u = ds - ext.mins[0];
			v = dt - ext.mins[1];
			return faceIdx;
		}
	}

	return -1;
}

RGB Bsp::get_lighting(Vector pos)
{
	int u, v;

	int faceIdx = traceFace(pos, pos - Vector(0, 0, 2048), u, v);

	if (faceIdx == -1) {
		return RGB(0, 0, 0);
	}

	BSPFACE& face = faces[faceIdx];
	FaceExtents& ext = faceExtents[faceIdx];

	int lx = u / 16;
	int ly = v / 16;
	int offset = face.nLightmapOffset + (ly * ext.size[0] + lx) * 3;
	int step = ext.size[0] * ext.size[1] * 3;
	int r = 0;
	int g = 0;
	int b = 0;

	// TODO: use dlights too
	for (int s = 0; s < MAXLIGHTMAPS; s++) {
		if (face.nStyles[s] == 255)
			break;

		float scale = GET_LIGHT_STYLE(face.nStyles[s]);
		r += lightdata[offset] * scale;
		g += lightdata[offset + 1] * scale;
		b += lightdata[offset + 2] * scale;
		offset += step;
	}

	return RGB(V_min(255, r), V_min(255, g), V_min(255, b));
}

float Bsp::CalculatePointVecsProduct(const volatile float* point, const volatile float* vecs)
{
	volatile double val;
	volatile double tmp;

	val = (double)point[0] * (double)vecs[0]; // always do one operation at a time and save to memory
	tmp = (double)point[1] * (double)vecs[1];
	val = val + tmp;
	tmp = (double)point[2] * (double)vecs[2];
	val = val + tmp;
	val = val + (double)vecs[3];

	return (float)val;
}

void Bsp::precalculate_face_extents() {
	if (faceExtents) {
		delete[] faceExtents;
	}

	faceExtents = new FaceExtents[faceCount];

	for (int i = 0; i < faceCount; i++) {
		int emins[2];
		int emaxs[2];
		calculate_face_extents(i, emins, emaxs);

		FaceExtents& ext = faceExtents[i];
		for (int k = 0; k < 2; k++) {
			ext.emins[k] = emins[k];
			ext.emaxs[k] = emaxs[k];
			ext.mins[k] = ext.emins[k] * 16;
			ext.maxs[k] = (ext.emaxs[k] - ext.emins[k]) * 16;
			ext.size[k] = clampi(emaxs[k] - emins[k], 0, MAX_SURFACE_EXTENTS) + 1;
		}
	}
}

void Bsp::calculate_face_extents(int facenum, int mins_out[2], int maxs_out[2]) {
	//CorrectFPUPrecision();

	BSPFACE* f;
	float mins[2], maxs[2], val;
	int i, j, e;
	Vector* v;
	BSPTEXTUREINFO* tex;

	f = &faces[facenum];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -999999;

	tex = &texinfos[f->iTextureInfo];

	for (i = 0; i < f->nEdges; i++)
	{
		e = surfedges[f->iFirstEdge + i];
		if (e >= 0)
		{
			v = &verts[edges[e].iVertex[0]];
		}
		else
		{
			v = &verts[edges[-e].iVertex[1]];
		}
		for (j = 0; j < 2; j++)
		{
			// The old code: val = v->point[0] * tex->vecs[j][0] + v->point[1] * tex->vecs[j][1] + v->point[2] * tex->vecs[j][2] + tex->vecs[j][3];
			//   was meant to be compiled for x86 under MSVC (prior to VS 11), so the intermediate values were stored as 64-bit double by default.
			// The new code will produce the same result as the old code, but it's portable for different platforms.
			// See this article for details: Intermediate Floating-Point Precision by Bruce-Dawson http://www.altdevblogaday.com/2012/03/22/intermediate-floating-point-precision/

			// The essential reason for having this ugly code is to get exactly the same value as the counterpart of game engine.
			// The counterpart of game engine is the function CalcFaceExtents in HLSDK.
			// So we must also know how Valve compiles HLSDK. I think Valve compiles HLSDK with VC6.0 in the past.
			Vector& axis = j == 0 ? tex->vS : tex->vT;
			val = CalculatePointVecsProduct((vec_t*)v, (vec_t*)&axis);

			if (val < mins[j])
			{
				mins[j] = val;
			}
			if (val > maxs[j])
			{
				maxs[j] = val;
			}
		}
	}

	for (i = 0; i < 2; i++)
	{
		mins_out[i] = (int)floor(mins[i] / TEXTURE_STEP);
		maxs_out[i] = (int)ceil(maxs[i] / TEXTURE_STEP);
	}
}

void Bsp::parseEntities(const char* data, int dataLen, std::vector<StringMap>& outputEnts) {
	membuf sbuf((char*)data, dataLen);
	istream in(&sbuf);

	int lineNum = 0;
	int lastBracket = -1;

	StringMap ent;

	outputEnts.clear();

	string current, key, value;
	bool inString = false;
	bool readingKey = true;
	char c;
	while (in.get(c)) {
		if (inString) {
			if (c == '"') {
				inString = false;

				if (readingKey) {
					key = current;
					readingKey = false;
				}
				else {
					value = current;

					if (!key.empty())
						ent.put(key.c_str(), value.c_str());

					key.clear();
					value.clear();
					readingKey = true;
				}

				current.clear();
			}
			else {
				current += c; // keep everything, including newlines
			}
			continue;
		}

		// not inside string
		if (c == '"') {
			inString = true;
			current.clear();
		}
		else if (c == '{') {
			ent.clear();
			readingKey = true;
			key.clear();
			value.clear();
		}
		else if (c == '}') {
			if (ent.get("classname"))
				outputEnts.push_back(ent);

			ent.clear();
		}
		else {
			// ignore everything else outside strings (whitespace, newlines, etc.)
		}
	}

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
				ALERT(at_console, "ent data (line %d): Unexpected '{'\n", lineNum);
				continue;
			}
			lastBracket = 0;
		}
		else if (line[0] == '}')
		{
			if (lastBracket == 1)
				ALERT(at_console, "ent data (line %d): Unexpected '}'\n", lineNum);
			lastBracket = 1;

			// you can end/start an ent on the same line, you know
			if (line.find("{") != string::npos) {
				lastBracket = 0;
			}

			outputEnts.push_back(ent);
			ent.clear();
		}
		else if (lastBracket == 0) // currently defining an entity
		{
			string key, value;
			parse_keyvalue(line, key, value);
			ent.put(key.c_str(), value.c_str());
		}
	}
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
