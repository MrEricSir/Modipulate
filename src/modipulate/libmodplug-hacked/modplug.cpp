/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#include "modplug.h"
#include "stdafx.h"
#include "sndfile.h"

#include <modipulate_common.h>

namespace HackedModPlug
{
	ModPlug_Settings gSettings =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2, // mChannels
		16, // mBits
		44100, // mFrequency
		MODPLUG_RESAMPLE_LINEAR, //mResamplingMode

		128, // mStereoSeparation
		32, // mMaxMixChannels
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

	int gSampleSize;

	void UpdateSettings(bool updateBasicConfig)
	{
		if(gSettings.mFlags & MODPLUG_ENABLE_REVERB)
		{
			HackedCSoundFile::SetReverbParameters(gSettings.mReverbDepth,
			                                gSettings.mReverbDelay);
		}

		if(gSettings.mFlags & MODPLUG_ENABLE_MEGABASS)
		{
			HackedCSoundFile::SetXBassParameters(gSettings.mBassAmount,
			                               gSettings.mBassRange);
		}
		else // modplug seems to ignore the SetWaveConfigEx() setting for bass boost
			HackedCSoundFile::SetXBassParameters(0, 0);

		if(gSettings.mFlags & MODPLUG_ENABLE_SURROUND)
		{
			HackedCSoundFile::SetSurroundParameters(gSettings.mSurroundDepth,
			                                  gSettings.mSurroundDelay);
		}

		if(updateBasicConfig)
		{
			HackedCSoundFile::SetWaveConfig(gSettings.mFrequency,
                                                  gSettings.mBits,
			                          gSettings.mChannels);
			HackedCSoundFile::SetMixConfig(gSettings.mStereoSeparation,
                                                 gSettings.mMaxMixChannels);

			gSampleSize = gSettings.mBits / 8 * gSettings.mChannels;
		}

		HackedCSoundFile::SetWaveConfigEx(gSettings.mFlags & MODPLUG_ENABLE_SURROUND,
		                            !(gSettings.mFlags & MODPLUG_ENABLE_OVERSAMPLING),
		                            gSettings.mFlags & MODPLUG_ENABLE_REVERB,
		                            true,
		                            gSettings.mFlags & MODPLUG_ENABLE_MEGABASS,
		                            gSettings.mFlags & MODPLUG_ENABLE_NOISE_REDUCTION,
		                            false);
		HackedCSoundFile::SetResamplingMode(gSettings.mResamplingMode);
	}
}

HackedModPlugFile* HackedModPlug_Load(const void* data, int size)
{
    DPRINT("ModPlug_Load");
	HackedModPlugFile* result = new HackedModPlugFile;
	HackedModPlug::UpdateSettings(true);
	if(result->mSoundFile.Create((const BYTE*)data, size))
	{
		result->mSoundFile.SetRepeatCount(HackedModPlug::gSettings.mLoopCount);
		return result;
	}
	else
	{
		delete result;
		return NULL;
	}
}

void ModPlug_Unload(HackedModPlugFile* file)
{
	file->mSoundFile.Destroy();
	delete file;
}

int HackedModPlug_Read(HackedModPlugFile* file, void* buffer, int size)
{
	return file->mSoundFile.Read(buffer, size) * HackedModPlug::gSampleSize;
}

const char* ModPlug_GetName(HackedModPlugFile* file)
{
	return file->mSoundFile.GetTitle();
}

int ModPlug_GetLength(HackedModPlugFile* file)
{
	return file->mSoundFile.GetSongTime() * 1000;
}

void ModPlug_InitMixerCallback(HackedModPlugFile* file,ModPlugMixerProc proc)
{
	file->mSoundFile.gpSndMixHook = (LPSNDMIXHOOKPROC)proc ;
	return;
}

void ModPlug_UnloadMixerCallback(HackedModPlugFile* file)
{
	file->mSoundFile.gpSndMixHook = NULL;
	return ;
}

unsigned int ModPlug_GetMasterVolume(HackedModPlugFile* file)
{
	return (unsigned int)file->mSoundFile.m_nMasterVolume;
}

void ModPlug_SetMasterVolume(HackedModPlugFile* file,unsigned int cvol)
{
	(void)file->mSoundFile.SetMasterVolume( (UINT)cvol,
						FALSE );
	return ;
}

int ModPlug_GetCurrentSpeed(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nMusicSpeed;
}

int ModPlug_GetCurrentTempo(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nMusicTempo;
}

int ModPlug_GetCurrentOrder(HackedModPlugFile* file)
{
	return file->mSoundFile.GetCurrentOrder();
}

int ModPlug_GetCurrentPattern(HackedModPlugFile* file)
{
	return file->mSoundFile.GetCurrentPattern();
}

int ModPlug_GetCurrentRow(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nRow;
}

int ModPlug_GetPlayingChannels(HackedModPlugFile* file)
{
	return ( file->mSoundFile.m_nMixChannels < file->mSoundFile.m_nMaxMixChannels ? file->mSoundFile.m_nMixChannels : file->mSoundFile.m_nMaxMixChannels );
}

void ModPlug_SeekOrder(HackedModPlugFile* file,int order)
{
	file->mSoundFile.SetCurrentOrder(order);
}

int ModPlug_GetModuleType(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nType;
}

char* ModPlug_GetMessage(HackedModPlugFile* file)
{
	return file->mSoundFile.m_lpszSongComments;
}

#ifndef MODPLUG_NO_FILESAVE
char ModPlug_ExportS3M(HackedModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveS3M(filepath,0);
}

char ModPlug_ExportXM(HackedModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveXM(filepath,0);
}

char ModPlug_ExportMOD(HackedModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveMod(filepath,0);
}

char ModPlug_ExportIT(HackedModPlugFile* file,const char* filepath)
{
	return (char)file->mSoundFile.SaveIT(filepath,0);
}
#endif // MODPLUG_NO_FILESAVE

unsigned int ModPlug_NumInstruments(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nInstruments;
}

unsigned int ModPlug_NumSamples(HackedModPlugFile* file)
{
	return file->mSoundFile.m_nSamples;
}

unsigned int ModPlug_NumPatterns(HackedModPlugFile* file)
{
	return file->mSoundFile.GetNumPatterns();
}

unsigned int ModPlug_NumChannels(HackedModPlugFile* file)
{
	return file->mSoundFile.GetNumChannels();
}

unsigned int ModPlug_SampleName(HackedModPlugFile* file,unsigned int qual,char* buff)
{
	return file->mSoundFile.GetSampleName(qual,buff);
}

unsigned int ModPlug_InstrumentName(HackedModPlugFile* file,unsigned int qual,char* buff)
{
	return file->mSoundFile.GetInstrumentName(qual,buff);
}

ModPlugNote* ModPlug_GetPattern(HackedModPlugFile* file,int pattern,unsigned int* numrows) {
	if ( pattern<MAX_PATTERNS ) {
		if (file->mSoundFile.Patterns[pattern]) {
			if (numrows) *numrows=(unsigned int)file->mSoundFile.PatternSize[pattern];
			return (ModPlugNote*)file->mSoundFile.Patterns[pattern];
		}
	}
	return NULL;
}

void ModPlug_Seek(HackedModPlugFile* file, int millisecond)
{
	int maxpos;
	int maxtime = file->mSoundFile.GetSongTime() * 1000;
	float postime;

	if(millisecond > maxtime)
		millisecond = maxtime;
	maxpos = file->mSoundFile.GetMaxPosition();
	postime = (float)maxpos / (float)maxtime;

	file->mSoundFile.SetCurrentPos((int)(millisecond * postime));
}

void ModPlug_GetSettings(ModPlug_Settings* settings)
{
	memcpy(settings, &HackedModPlug::gSettings, sizeof(ModPlug_Settings));
}

void ModPlug_SetSettings(const ModPlug_Settings* settings)
{
	memcpy(&HackedModPlug::gSettings, settings, sizeof(ModPlug_Settings));
	HackedModPlug::UpdateSettings(false); // do not update basic config.
}
