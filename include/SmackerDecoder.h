/*
 * libsmackerdec - Smacker video decoder
 * Copyright (C) 2011 Barry Duncan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* This code is based on smacker.c from the FFmpeg project which can be obtained from http://www.ffmpeg.org/
 * below is the license from smacker.c
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Smacker decoder
 * Copyright (c) 2006 Konstantin Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _SmackerDecoder_h_
#define _SmackerDecoder_h_

#include <stdint.h>
#include "FileStream.h"
#include "BitReader.h"
#include <vector>

// exportable interface
struct SmackerHandle
{
	bool isValid;
	int instanceIndex;
};

struct SmackerAudioInfo
{
	uint32_t sampleRate;
	uint32_t nChannels;
	uint32_t bitsPerSample;

	uint32_t idealBufferSize;
};

SmackerHandle     Smacker_Open( const char* fileName );
void              Smacker_Close( SmackerHandle& handle );
uint32_t          Smacker_GetNumAudioTracks( SmackerHandle& handle );
SmackerAudioInfo  Smacker_GetAudioTrackDetails( SmackerHandle& handle, uint32_t trackIndex );
uint32_t          Smacker_GetAudioData( SmackerHandle& handle, uint32_t trackIndex, int16_t* data );
uint32_t          Smacker_GetNumFrames( SmackerHandle& handle );
void              Smacker_GetFrameSize( SmackerHandle& handle, uint32_t& width, uint32_t& height );
uint32_t          Smacker_GetCurrentFrameNum( SmackerHandle& handle );
uint32_t          Smacker_GetNextFrame( SmackerHandle& handle );
float             Smacker_GetFrameRate( SmackerHandle& handle );
void              Smacker_GetPalette( SmackerHandle& handle, uint8_t* palette );
void              Smacker_GetFrame( SmackerHandle& handle, uint8_t* frame );

const int kMaxAudioTracks = 7;

// forward declare
struct HuffContext;
struct DBCtx;

struct SmackerAudioTrack
{
	uint32_t sizeInBytes;
	uint32_t flags;
	uint32_t sampleRate;
	uint32_t nChannels;
	uint8_t  bitsPerSample;
	int	     compressionType;

	uint8_t* buffer;
	uint32_t bufferSize;

	uint32_t bytesReadThisFrame;
};

class SmackerDecoder
{
public:

	uint32_t frameWidth;
	uint32_t frameHeight;

	SmackerDecoder();
	~SmackerDecoder();

	bool Open( const std::string& fileName );
	void GetPalette( uint8_t* palette );
	void GetFrame( uint8_t* frame );

	SmackerAudioInfo GetAudioTrackDetails( uint32_t trackIndex );
	uint32_t GetAudioData( uint32_t trackIndex, int16_t* audioBuffer );
	uint32_t GetNumFrames();
	uint32_t GetCurrentFrameNum();
	float GetFrameRate();
	void GetNextFrame();

private:

	SmackerCommon::FileStream file;
	uint32_t signature;

	// video related members
	uint32_t nFrames;
	uint32_t fps; // frames per second

	uint8_t palette[768];
	uint8_t* picture;

	bool isVer4;

	SmackerAudioTrack audioTracks[kMaxAudioTracks];

	uint32_t treeSize;
	uint32_t mMapSize, MClrSize, fullSize, typeSize;

	std::vector<int> mmap_tbl;
	std::vector<int> mclr_tbl;
	std::vector<int> full_tbl;
	std::vector<int> type_tbl;

	int mmap_last[3], mclr_last[3], full_last[3], type_last[3];

	std::vector<uint32_t> frameSizes;
	std::vector<uint8_t> frameFlags;

	uint32_t currentFrame;

	int32_t nextPos;

	bool DecodeHeaderTrees();
	int DecodeHeaderTree( SmackerCommon::BitReader& bits, std::vector<int>& recodes, int* last, int size );
	int DecodeTree( SmackerCommon::BitReader& bits, HuffContext* hc, uint32_t prefix, int length );
	int DecodeBigTree( SmackerCommon::BitReader& bits, HuffContext* hc, DBCtx* ctx );
	int GetCode( SmackerCommon::BitReader& bits, std::vector<int>& recode, int* last );
	int ReadPacket();
	int DecodeFrame( uint32_t frameSize );
	void GetFrameSize( uint32_t& width, uint32_t& height );
	int DecodeAudio( uint32_t size, SmackerAudioTrack& track );
};

#endif
