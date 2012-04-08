/*
 * This source code is public domain.
 *
 * Authors: Kenton Varda <temporal@gauge3d.org> (C interface wrapper)
 */

#ifndef MODPLUG_H__INCLUDED
#define MODPLUG_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct _HackedModPlugFile;
typedef struct _HackedModPlugFile HackedModPlugFile;


// Modipulate callbacks.
typedef void (*modplug_hacked_on_pattern_changed_cb) (unsigned pattern, void* user_data);
typedef void (*modplug_hacked_on_tempo_changed_cb) (int tempo, void* user_data);
typedef void (*modplug_hacked_on_note_change_cb) (unsigned channel, int note, int instrument, int sample, int volume, void* user_data);
typedef void (*modplug_hacked_on_row_changed_cb) (int row, void* user_data);

typedef void (*modplug_hacked_increase_sample_count_cb) (int add, void* user_data);
typedef bool (*modplug_hacked_is_volume_command_enabled_cb) (int channel, int volume_command, void* user_data);
typedef bool (*modplug_hacked_is_volume_command_pending_cb) (unsigned channel, void* user_data);
typedef unsigned (*modplug_hacked_pop_volume_command_cb) (unsigned channel, void* user_data);
typedef unsigned (*modplug_hacked_pop_volume_parameter_cb) (unsigned channel, void* user_data);
typedef bool (*modplug_hacked_is_effect_command_enabled_cb) (int channel, int effect_command, void* user_data);
typedef bool (*modplug_hacked_is_effect_command_pending_cb) (unsigned channel, void* user_data);
typedef unsigned (*modplug_hacked_pop_effect_command_cb) (unsigned channel, void* user_data);
typedef unsigned (*modplug_hacked_pop_effect_parameter_cb) (unsigned channel, void* user_data);



struct _ModPlugNote {
	unsigned char Note;
	unsigned char Instrument;
	unsigned char VolumeEffect;
	unsigned char Effect;
	unsigned char Volume;
	unsigned char Parameter;
};
typedef struct _ModPlugNote ModPlugNote;

typedef void (*ModPlugMixerProc)(int*, unsigned long, unsigned long);

/* Load a mod file.  [data] should point to a block of memory containing the complete
 * file, and [size] should be the size of that block.
 * Return the loaded mod file on success, or NULL on failure. */
HackedModPlugFile* HackedModPlug_Load(const void* data, int size,
    // For Modipulation, a number of additional parameters are required.
    void* mod_stream,  // this is the ModStream instance.
    modplug_hacked_increase_sample_count_cb increase_sample_count,
    modplug_hacked_is_volume_command_enabled_cb is_volume_command_enabled,
    modplug_hacked_is_volume_command_pending_cb is_volume_command_pending,
    modplug_hacked_pop_volume_command_cb pop_volume_command,
    modplug_hacked_pop_volume_parameter_cb pop_volume_parameter,
    modplug_hacked_is_effect_command_enabled_cb is_effect_command_enabled,
    modplug_hacked_is_effect_command_pending_cb is_effect_command_pending,
    modplug_hacked_pop_effect_command_cb pop_effect_command,
    modplug_hacked_pop_effect_parameter_cb pop_effect_parameter
);

/* Unload a mod file. */
void ModPlug_Unload(HackedModPlugFile* file);

/* Read sample data into the buffer.  Returns the number of bytes read.  If the end
 * of the mod has been reached, zero is returned. */
int  HackedModPlug_Read(HackedModPlugFile* file, void* buffer, int size);

/* Get the name of the mod.  The returned buffer is stored within the HackedModPlugFile
 * structure and will remain valid until you unload the file. */
const char* ModPlug_GetName(HackedModPlugFile* file);

/* Get the length of the mod, in milliseconds.  Note that this result is not always
 * accurate, especially in the case of mods with loops. */
int ModPlug_GetLength(HackedModPlugFile* file);

/* Seek to a particular position in the song.  Note that seeking and MODs don't mix very
 * well.  Some mods will be missing instruments for a short time after a seek, as ModPlug
 * does not scan the sequence backwards to find out which instruments were supposed to be
 * playing at that time.  (Doing so would be difficult and not very reliable.)  Also,
 * note that seeking is not very exact in some mods -- especially those for which
 * ModPlug_GetLength() does not report the full length. */
void ModPlug_Seek(HackedModPlugFile* file, int millisecond);

enum _ModPlug_Flags
{
	MODPLUG_ENABLE_OVERSAMPLING     = 1 << 0,  /* Enable oversampling (*highly* recommended) */
	MODPLUG_ENABLE_NOISE_REDUCTION  = 1 << 1,  /* Enable noise reduction */
	MODPLUG_ENABLE_REVERB           = 1 << 2,  /* Enable reverb */
	MODPLUG_ENABLE_MEGABASS         = 1 << 3,  /* Enable megabass */
	MODPLUG_ENABLE_SURROUND         = 1 << 4   /* Enable surround sound. */
};

enum _ModPlug_ResamplingMode
{
	MODPLUG_RESAMPLE_NEAREST = 0,  /* No interpolation (very fast, extremely bad sound quality) */
	MODPLUG_RESAMPLE_LINEAR  = 1,  /* Linear interpolation (fast, good quality) */
	MODPLUG_RESAMPLE_SPLINE  = 2,  /* Cubic spline interpolation (high quality) */
	MODPLUG_RESAMPLE_FIR     = 3   /* 8-tap fir filter (extremely high quality) */
};

typedef struct _ModPlug_Settings
{
	int mFlags;  /* One or more of the MODPLUG_ENABLE_* flags above, bitwise-OR'ed */
	
	/* Note that ModPlug always decodes sound at 44100kHz, 32 bit, stereo and then
	 * down-mixes to the settings you choose. */
	int mChannels;       /* Number of channels - 1 for mono or 2 for stereo */
	int mBits;           /* Bits per sample - 8, 16, or 32 */
	int mFrequency;      /* Sampling rate - 11025, 22050, or 44100 */
	int mResamplingMode; /* One of MODPLUG_RESAMPLE_*, above */

	int mStereoSeparation; /* Stereo separation, 1 - 256 */
	int mMaxMixChannels; /* Maximum number of mixing channels (polyphony), 32 - 256 */
	
	int mReverbDepth;    /* Reverb level 0(quiet)-100(loud)      */
	int mReverbDelay;    /* Reverb delay in ms, usually 40-200ms */
	int mBassAmount;     /* XBass level 0(quiet)-100(loud)       */
	int mBassRange;      /* XBass cutoff in Hz 10-100            */
	int mSurroundDepth;  /* Surround level 0(quiet)-100(heavy)   */
	int mSurroundDelay;  /* Surround delay in ms, usually 5-40ms */
	int mLoopCount;      /* Number of times to loop.  Zero prevents looping.
	                        -1 loops forever. */
} ModPlug_Settings;

/* Get and set the mod decoder settings.  All options, except for channels, bits-per-sample,
 * sampling rate, and loop count, will take effect immediately.  Those options which don't
 * take effect immediately will take effect the next time you load a mod. */
void ModPlug_GetSettings(ModPlug_Settings* settings);
void ModPlug_SetSettings(const ModPlug_Settings* settings);

/* New ModPlug API Functions */
/* NOTE: Master Volume (1-512) */
unsigned int ModPlug_GetMasterVolume(HackedModPlugFile* file) ;
void ModPlug_SetMasterVolume(HackedModPlugFile* file,unsigned int cvol) ;

int ModPlug_GetCurrentSpeed(HackedModPlugFile* file);
int ModPlug_GetCurrentTempo(HackedModPlugFile* file);
int ModPlug_GetCurrentOrder(HackedModPlugFile* file);
int ModPlug_GetCurrentPattern(HackedModPlugFile* file);
int ModPlug_GetCurrentRow(HackedModPlugFile* file);
int ModPlug_GetPlayingChannels(HackedModPlugFile* file);

void ModPlug_SeekOrder(HackedModPlugFile* file,int order);
int ModPlug_GetModuleType(HackedModPlugFile* file);
char* ModPlug_GetMessage(HackedModPlugFile* file);

#ifndef MODPLUG_NO_FILESAVE
/*
 * EXPERIMENTAL Export Functions
 */
/*Export to a Scream Tracker 3 S3M module. EXPERIMENTAL (only works on Little-Endian platforms)*/
char ModPlug_ExportS3M(HackedModPlugFile* file, const char* filepath);

/*Export to a Extended Module (XM). EXPERIMENTAL (only works on Little-Endian platforms)*/
char ModPlug_ExportXM(HackedModPlugFile* file, const char* filepath);

/*Export to a Amiga MOD file. EXPERIMENTAL.*/
char ModPlug_ExportMOD(HackedModPlugFile* file, const char* filepath);

/*Export to a Impulse Tracker IT file. Should work OK in Little-Endian & Big-Endian platforms :-) */
char ModPlug_ExportIT(HackedModPlugFile* file, const char* filepath);
#endif // MODPLUG_NO_FILESAVE

unsigned int ModPlug_NumInstruments(HackedModPlugFile* file);
unsigned int ModPlug_NumSamples(HackedModPlugFile* file);
unsigned int ModPlug_NumPatterns(HackedModPlugFile* file);
unsigned int ModPlug_NumChannels(HackedModPlugFile* file);
unsigned int ModPlug_SampleName(HackedModPlugFile* file, unsigned int qual, char* buff);
unsigned int ModPlug_InstrumentName(HackedModPlugFile* file, unsigned int qual, char* buff);

void HackedModPlug_SetChannelEnabled(HackedModPlugFile* file, int channel, bool is_enabled);
bool HackedModPlug_GetChannelEnabled(HackedModPlugFile* file, int channel);
int HackedModPlug_GetTransposition(HackedModPlugFile* file, int channel);
void HackedModPlug_SetTransposition(HackedModPlugFile* file, int channel, int offset);

void HackedModPlug_SetOnTempoChanged(HackedModPlugFile* file, modplug_hacked_on_tempo_changed_cb cb);
void HackedModPlug_SetOnNoteChange(HackedModPlugFile* file, modplug_hacked_on_note_change_cb cb);
void HackedModPlug_SetOnPatternChanged(HackedModPlugFile* file, modplug_hacked_on_pattern_changed_cb cb);
void HackedModPlug_SetOnRowChanged(HackedModPlugFile* file, modplug_hacked_on_row_changed_cb cb);


/*
 * Retrieve pattern note-data
 */
ModPlugNote* ModPlug_GetPattern(HackedModPlugFile* file, int pattern, unsigned int* numrows);

/*
 * =================
 * Mixer callback
 * =================
 *
 * Use this callback if you want to 'modify' the mixed data of LibModPlug.
 * 
 * void proc(int* buffer,unsigned long channels,unsigned long nsamples) ;
 *
 * 'buffer': A buffer of mixed samples
 * 'channels': N. of channels in the buffer
 * 'nsamples': N. of samples in the buffeer (without taking care of n.channels)
 *
 * (Samples are signed 32-bit integers)
 */
void ModPlug_InitMixerCallback(HackedModPlugFile* file,ModPlugMixerProc proc) ;
void ModPlug_UnloadMixerCallback(HackedModPlugFile* file) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
