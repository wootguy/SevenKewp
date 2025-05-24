#pragma once
#include <stdint.h>

// WAVE file header format
#pragma pack(push, 1)
struct RIFF_HEADER {
	char riff[4];		// RIFF string
	uint32_t overall_size;	// overall size of file in bytes
	char wave[4];		// WAVE string
};

struct RIFF_FMT_PCM {
	uint16_t format_type;		// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;			// no.of channels
	uint32_t sample_rate;		// sampling rate (blocks per second)
	uint32_t byterate;			// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;		// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc
};

// common format fields
struct WAVE_CHUNK_FMT_COMMON {
	uint16_t format_type;		// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;			// no.of channels
	uint32_t sample_rate;		// sampling rate (blocks per second)
	uint32_t byterate;			// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;		// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc
};

// non-pcm format fields
struct WAVE_CHUNK_FMT_EXTENDED {
	uint16_t cbSize;			// size of the extension

	// extensible format fields
	uint16_t validBitsPerSample;
	uint32_t dwChannelMask;
	uint8_t subFormat[16];
};

struct WAVE_CHUNK_FMT {
	WAVE_CHUNK_FMT_COMMON common;
	WAVE_CHUNK_FMT_EXTENDED extended; // optional
};

struct WAVE_CUE_HEADER {
	uint32_t numCuePoints;
	// WAVE_CUE structs follow
};

struct WAVE_CUE {
	uint32_t id; // A unique number for the point used by other chunks to identify the cue point. For example, a playlist chunk creates a playlist by referring to cue points, which themselves define points somewhere in the file
	uint32_t position; // If there is no playlist chunk, this value is zero. If there is a playlist chunk, this value is the sample at which the cue point should occur
	char dataChunkId[4]; // Either "data" or "slnt" depending on whether the cue occurs in a data chunk or in a silent chunk
	uint32_t chunkStart; // The position of the start of the data chunk that contains the cue point. If there is a wave list chunk, this value is the byte position of the chunk that contains the cue. If there is no wave list chunk, there is only one data chunk in the file and this value is zero
	uint32_t blockStart; // The byte position of the cue in the "data" or "slnt" chunk. If this is an uncompressed PCM file, this is counted from the beginning of the chunk's data. If this is a compressed file, the byte position can be counted from the last byte from which one can start decompressing to find the cue
	uint32_t sampleStart; // The position of the cue in number of bytes from the start of the block
};

struct WAVE_LIST_HEADER {
	char listType[4];
};

struct WAVE_LIST_INFO_HEADER {
	char infoType[4];
	uint32_t textSize;
};

struct WAVE_ADTL_HEADER {
	char adtlType[4];
	uint32_t subSize;
	uint32_t cueId;
	// ascii text follows, if "labl" or "note" chunk
};

// for "ltxt" adtl chunks
struct WAVE_CUE_LABEL {
	uint32_t sampleLength;
	uint32_t purposeId;
	uint16_t country;
	uint16_t lang;
	uint16_t dialect;
	uint16_t codePage;
	// ascii text follows
};

struct WAVE_CHUNK_HEADER {
	uint8_t name[4];
	uint32_t size;
};
#pragma pack(pop)

struct WavInfo {
	int durationMillis;
	bool isLooped; // sound is looped with cue points
	uint8_t channels;
	uint8_t bytesPerSample;
	uint8_t numCues;
	int numSamples;
	int sampleFileOffset; // file offset where sample data begins
	int sampleRate;
	float cues[2];
};

extern HashMap<WavInfo> g_wavInfos; // cached wav info, cleared on map change

enum wav_error_codes {
	WAVERR_INVALID_HEADER, // invalid file header
	WAVERR_INVALID_FMT, // invalid fmt chunk
	WAVERR_INVALID_CUE, // invalid cue chunk
};

// parses wav data from a file, returns 0 on sucess, else an error code
EXPORT int parseWaveInfo(FILE* fp, WavInfo& info);

// similar to parseWaveInfo, but caches results, searches game paths for the file, and shuts
// down the server if a fatal wave is detected (bad cue points which crash the client)
EXPORT WavInfo getWaveFileInfo(const char* path);

EXPORT void writeWavFile(const char* fpath, void* samples, int numSamples, int samprate, int bytesPerSample);