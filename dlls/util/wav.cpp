#include "extdll.h"
#include "util.h"
#include "wav.h"

HashMap<WavInfo> g_wavInfos;

int parseWaveInfo(FILE* file, WavInfo& info) {
	memset(&info, 0, sizeof(WavInfo));	

	int sampleRate = 0;
	int bytesPerSample = 0;
	int numSamples = 0;
	int read = 0;
	int fsize = 0;

	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	{
		RIFF_HEADER header;
		read = fread(&header, sizeof(RIFF_HEADER), 1, file);

		if (!read || strncmp((const char*)header.riff, "RIFF", 4)
			|| strncmp((const char*)header.wave, "WAVE", 4)) {

			return WAVERR_INVALID_HEADER;
		}
	}

	//ALERT(at_console, "%s\n", path);

	//static char infoText[512];

	while (1) {
		if (ftell(file) + 8 >= fsize) {
			break;
		}

		WAVE_CHUNK_HEADER chunk;
		read = fread(&chunk, sizeof(WAVE_CHUNK_HEADER), 1, file);

		if (!read) {
			break; // end of file
		}

		std::string chunkName = std::string((const char*)chunk.name, 4);
		//ALERT(at_console, "    Read chunk %s (%d)\n", chunkName.c_str(), chunk.size);

		int seekSize = ((chunk.size + 1) / 2) * 2; // round up to nearest word size

		if (chunkName == "fmt ") {
			if (chunk.size == 16 || chunk.size == 18 || chunk.size == 40) {
				WAVE_CHUNK_FMT cdata;
				read = fread(&cdata, chunk.size, 1, file);

				bytesPerSample = cdata.common.channels * (cdata.common.bits_per_sample / 8);
				sampleRate = cdata.common.sample_rate;
				info.bytesPerSample = bytesPerSample;
				info.channels = cdata.common.channels;
				info.sampleRate = cdata.common.sample_rate;

				if (!read || !bytesPerSample || !sampleRate) {
					return WAVERR_INVALID_FMT;
				}
			}
			else {
				return WAVERR_INVALID_FMT;
			}
		}
		else if (chunkName == "data") {
			if (bytesPerSample && sampleRate && chunk.size) {
				numSamples = chunk.size / bytesPerSample;
				info.durationMillis = ((float)numSamples / sampleRate) * 1000.0f;
				info.numSamples = numSamples;
			}

			info.sampleFileOffset = ftell(file);

			fseek(file, seekSize, SEEK_CUR);
		}
		else if (chunkName == "cue ") {
			WAVE_CUE_HEADER cdata;
			read = fread(&cdata, sizeof(WAVE_CUE_HEADER), 1, file);

			if (!read) {
				return WAVERR_INVALID_CUE;
			}

			// How cue points work:
			// 1  cue point  = loop starts at the cue point and ends at an equal distance from the end
			// 2  cue points = loop starts at the 1st cue and ends at the 2nd cue
			// 3+ cue points = same as 2 and the extras are ignored
			//
			// All fields besides "sampleStart" are ignored
			// inverted cue points will crash the client
			// (possible with 1 cue point if placed after the middle of the file)

			//std::string cueString = "        cue points: ";
			for (int i = 0; i < (int)cdata.numCuePoints && i < 2; i++) {
				WAVE_CUE cue;
				read = fread(&cue, sizeof(WAVE_CUE), 1, file);

				std::string dataChunkId = std::string(cue.dataChunkId, 4);
				if (!read || (dataChunkId != "data" && dataChunkId != "slnt")) {
					return WAVERR_INVALID_CUE;
				}

				float sampTime = sampleRate ? cue.sampleStart / (float)sampleRate : 0;
				info.cues[i] = sampTime;
				//cueString += UTIL_VarArgs("%.2f  ", sampTime);
			}
			//cueString += "\n";
			//ALERT(at_console, cueString.c_str());

			info.isLooped = cdata.numCuePoints > 0;
			info.numCues = cdata.numCuePoints;
		}
		/*
		else if (chunkName == "LIST") {
			WAVE_LIST_HEADER cdata;
			read = fread(&cdata, sizeof(WAVE_LIST_HEADER), 1, file);

			if (!read) {
				ALERT(at_error, "Invalid WAVE list chunk: %s\n", fpath.c_str());
				goto cleanup;
			}

			std::string listType = std::string(cdata.listType, 4);

			ALERT(at_console, "        type: %s\n", listType.c_str());
			if (!strncmp(cdata.listType, "INFO", 4)) {
				WAVE_LIST_INFO_HEADER iheader;
				read = fread(&iheader, sizeof(WAVE_LIST_INFO_HEADER), 1, file);

				if (!read) {
					ALERT(at_error, "Invalid WAVE list chunk: %s\n", fpath.c_str());
					goto cleanup;
				}

				std::string infoType = std::string(iheader.infoType, 4);

				int readSize = V_min(511, iheader.textSize);
				read = fread(infoText, readSize, 1, file);
				infoText[readSize] = '\0';

				ALERT(at_console, "        %s (%d): %s\n", infoType.c_str(), iheader.textSize, infoText);
			}
			else if (!strncmp(cdata.listType, "adtl", 4)) {
				WAVE_ADTL_HEADER aheader;
				read = fread(&aheader, sizeof(WAVE_ADTL_HEADER), 1, file);

				std::string adtlType = std::string(aheader.adtlType, 4);

				if (!strncmp(aheader.adtlType, "labl", 4) || !strncmp(aheader.adtlType, "note", 4)) {
					int headerSize = sizeof(WAVE_ADTL_HEADER) + sizeof(WAVE_LIST_INFO_HEADER) + sizeof(WAVE_LIST_HEADER) + 4;
					int readSize = V_min(511, chunk.size - headerSize);
					read = fread(infoText, readSize, 1, file);
					infoText[readSize] = '\0';
					ALERT(at_console, "        %s for %d (%d): %s\n", adtlType.c_str(), aheader.cueId, readSize, infoText);
				}
				else if (!strncmp(aheader.adtlType, "ltxt", 4)) {
					WAVE_CUE_LABEL lheader;
					read = fread(&aheader, sizeof(WAVE_CUE_LABEL), 1, file);
				}
			}

			fseek(file, chunk.size - sizeof(WAVE_LIST_HEADER), SEEK_CUR);
		}
		*/
		else {
			fseek(file, seekSize, SEEK_CUR);
		}
	}

	return 0;
}

WavInfo getWaveFileInfo(const char* path) {
	WavInfo* cached = g_wavInfos.get(path);
	if (cached) {
		return *cached;
	}

	std::string fpath = getGameFilePath(UTIL_VarArgs("sound/%s", path));
	WavInfo info;
	memset(&info, 0, sizeof(WavInfo));
		
	if (!fpath.size()) {
		ALERT(at_console, "Missing WAVE file: %s\n", path);
		g_wavInfos.put(path, info);
		return info;
	}

	FILE* file = NULL;

	// open file
	file = fopen(fpath.c_str(), "rb");
	if (file == NULL) {
		ALERT(at_error, "Failed to open WAVE file: %s\n", path);
		return info;
	}
	
	int ret = parseWaveInfo(file, info);

	if (!ret) {
		float durationSeconds = info.durationMillis / 1000.0f;
		bool fatalWave = false;

		if (info.numCues == 1 && info.cues[0] > durationSeconds * 0.495f) { // better safe than sorry here - don't use 0.5 exactly
			ALERT(at_error, "'%s' has 1 cue point and it was placed after the mid point! This file will crash clients.\n", path);
			fatalWave = true;
		}
		else if (info.numCues >= 2 && info.cues[0] > info.cues[1]) {
			ALERT(at_error, "'%s' start/end cue points are inverted! This file will crash clients.\n", path);
			fatalWave = true;
		}

		if (fatalWave) {
			// don't allow anyone to download this file! Not all server ops know that you need to rename
			// a file after updating it. Then if some people got the old file, they will crash and others
			// will be like "idk works for me" and people will have to delete their hl folder to fix this.
			// Or worse, they'll just live with it for years because it's just a few maps they crash on.
			ALERT(at_error, "Server shutting down to prevent distribution of the broken file.\n", path);
			g_engfuncs.pfnServerCommand("quit\n");
			g_engfuncs.pfnServerExecute();
		}
	}
	else {
		switch (ret) {
		case WAVERR_INVALID_HEADER:
			ALERT(at_error, "Invalid WAVE header: %s\n", fpath);
			break;
		case WAVERR_INVALID_FMT:
			ALERT(at_error, "Invalid WAVE fmt chunk: %s\n", fpath);
			break;
		case WAVERR_INVALID_CUE:
			ALERT(at_error, "Invalid WAVE cue chunk: %s\n", fpath);
			break;
		default:
			ALERT(at_error, "Invalid WAVE: %s\n", fpath);
			break;
		}
	}

	fclose(file);
	g_wavInfos.put(path, info);
	return info;
}

EXPORT void writeWavFile(const char* fpath, void* samples, int numSamples, int samprate, int bytesPerSample) {
	int headerSizes = sizeof(RIFF_HEADER) + sizeof(WAVE_CHUNK_HEADER)*2 + sizeof(WAVE_CHUNK_FMT_COMMON);

	FILE* fp = fopen(fpath, "wb");

	if (!fp) {
		ALERT(at_error, "Failed to open file for writing: %s\n", fpath);
		return;
	}

	RIFF_HEADER header;
	memcpy(header.riff, "RIFF", 4);
	header.overall_size = (numSamples * bytesPerSample) + headerSizes;
	memcpy(header.wave, "WAVE", 4);
	fwrite(&header, sizeof(RIFF_HEADER), 1, fp);

	WAVE_CHUNK_HEADER fmtheader;
	memcpy(fmtheader.name, "fmt ", 4);
	fmtheader.size = 16;
	fwrite(&fmtheader, sizeof(WAVE_CHUNK_HEADER), 1, fp);

	WAVE_CHUNK_FMT_COMMON fmt;
	memset(&fmt, 0, sizeof(WAVE_CHUNK_FMT_COMMON));
	fmt.bits_per_sample = bytesPerSample*8;
	fmt.channels = 1;
	fmt.block_align = (fmt.channels * fmt.bits_per_sample) / 8;
	fmt.byterate = samprate * 1 *bytesPerSample;
	fmt.format_type = 1;
	fmt.sample_rate = samprate;
	fwrite(&fmt, sizeof(WAVE_CHUNK_FMT_COMMON), 1, fp);

	WAVE_CHUNK_HEADER datheader;
	memcpy(datheader.name, "data", 4);
	datheader.size = numSamples*bytesPerSample;
	fwrite(&datheader, sizeof(WAVE_CHUNK_HEADER), 1, fp);
	fwrite(samples, bytesPerSample*numSamples, 1, fp);

	fclose(fp);
}
