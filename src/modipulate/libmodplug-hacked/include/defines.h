#ifndef __DEFINES_H
#define __DEFINES_H

#define MOD_AMIGAC2			0x1AB
#define MAX_SAMPLE_LENGTH	16000000
#define MAX_SAMPLE_RATE		192000
#define MAX_ORDERS			256
#define MAX_PATTERNS		240
#define MAX_SAMPLES			240
#define MAX_INSTRUMENTS		MAX_SAMPLES
#ifdef MODPLUG_FASTSOUNDLIB
#define MAX_CHANNELS		80
#else
#define MAX_CHANNELS		128
#endif
#define MAX_BASECHANNELS	64
#define MAX_ENVPOINTS		32
#define MIN_PERIOD			0x0020
#define MAX_PERIOD			0xFFFF
#define MAX_PATTERNNAME		32
#define MAX_CHANNELNAME		20
#define MAX_INFONAME		80
#define MAX_EQ_BANDS		6
#define MAX_MIXPLUGINS		8


#define MOD_TYPE_NONE		0x00
#define MOD_TYPE_MOD		0x01
#define MOD_TYPE_S3M		0x02
#define MOD_TYPE_XM			0x04
#define MOD_TYPE_MED		0x08
#define MOD_TYPE_MTM		0x10
#define MOD_TYPE_IT			0x20
#define MOD_TYPE_669		0x40
#define MOD_TYPE_ULT		0x80
#define MOD_TYPE_STM		0x100
#define MOD_TYPE_FAR		0x200
#define MOD_TYPE_WAV		0x400
#define MOD_TYPE_AMF		0x800
#define MOD_TYPE_AMS		0x1000
#define MOD_TYPE_DSM		0x2000
#define MOD_TYPE_MDL		0x4000
#define MOD_TYPE_OKT		0x8000
#define MOD_TYPE_MID		0x10000
#define MOD_TYPE_DMF		0x20000
#define MOD_TYPE_PTM		0x40000
#define MOD_TYPE_DBM		0x80000
#define MOD_TYPE_MT2		0x100000
#define MOD_TYPE_AMF0		0x200000
#define MOD_TYPE_PSM		0x400000
#define MOD_TYPE_J2B		0x800000
#define MOD_TYPE_ABC		0x1000000
#define MOD_TYPE_PAT		0x2000000
#define MOD_TYPE_UMX		0x80000000 // Fake type
#define MAX_MODTYPE		24



// Channel flags:
// Bits 0-7:	Sample Flags
#define CHN_16BIT               0x01
#define CHN_LOOP                0x02
#define CHN_PINGPONGLOOP        0x04
#define CHN_SUSTAINLOOP         0x08
#define CHN_PINGPONGSUSTAIN     0x10
#define CHN_PANNING             0x20
#define CHN_STEREO              0x40
#define CHN_PINGPONGFLAG	0x80
// Bits 8-31:	Channel Flags
#define CHN_MUTE                0x100
#define CHN_KEYOFF              0x200
#define CHN_NOTEFADE		0x400
#define CHN_SURROUND            0x800
#define CHN_NOIDO               0x1000
#define CHN_HQSRC               0x2000
#define CHN_FILTER              0x4000
#define CHN_VOLUMERAMP		0x8000
#define CHN_VIBRATO             0x10000
#define CHN_TREMOLO             0x20000
#define CHN_PANBRELLO		0x40000
#define CHN_PORTAMENTO		0x80000
#define CHN_GLISSANDO		0x100000
#define CHN_VOLENV              0x200000
#define CHN_PANENV              0x400000
#define CHN_PITCHENV		0x800000
#define CHN_FASTVOLRAMP		0x1000000
#define CHN_EXTRALOUD		0x2000000
#define CHN_REVERB              0x4000000
#define CHN_NOREVERB		0x8000000


#define ENV_VOLUME              0x0001
#define ENV_VOLSUSTAIN		0x0002
#define ENV_VOLLOOP             0x0004
#define ENV_PANNING             0x0008
#define ENV_PANSUSTAIN		0x0010
#define ENV_PANLOOP             0x0020
#define ENV_PITCH               0x0040
#define ENV_PITCHSUSTAIN	0x0080
#define ENV_PITCHLOOP		0x0100
#define ENV_SETPANNING		0x0200
#define ENV_FILTER              0x0400
#define ENV_VOLCARRY		0x0800
#define ENV_PANCARRY		0x1000
#define ENV_PITCHCARRY		0x2000

#define CMD_NONE                        0
#define CMD_ARPEGGIO			1
#define CMD_PORTAMENTOUP		2
#define CMD_PORTAMENTODOWN		3
#define CMD_TONEPORTAMENTO		4
#define CMD_VIBRATO                     5
#define CMD_TONEPORTAVOL		6
#define CMD_VIBRATOVOL			7
#define CMD_TREMOLO                     8
#define CMD_PANNING8			9
#define CMD_OFFSET                      10
#define CMD_VOLUMESLIDE			11
#define CMD_POSITIONJUMP		12
#define CMD_VOLUME                      13
#define CMD_PATTERNBREAK		14
#define CMD_RETRIG                      15
#define CMD_SPEED                       16
#define CMD_TEMPO                       17
#define CMD_TREMOR                      18
#define CMD_MODCMDEX			19
#define CMD_S3MCMDEX			20
#define CMD_CHANNELVOLUME		21
#define CMD_CHANNELVOLSLIDE		22
#define CMD_GLOBALVOLUME		23
#define CMD_GLOBALVOLSLIDE		24
#define CMD_KEYOFF                      25
#define CMD_FINEVIBRATO			26
#define CMD_PANBRELLO			27
#define CMD_XFINEPORTAUPDOWN            28
#define CMD_PANNINGSLIDE		29
#define CMD_SETENVPOSITION		30
#define CMD_MIDI                        31


// Volume Column commands
#define VOLCMD_VOLUME			1
#define VOLCMD_PANNING			2
#define VOLCMD_VOLSLIDEUP		3
#define VOLCMD_VOLSLIDEDOWN		4
#define VOLCMD_FINEVOLUP		5
#define VOLCMD_FINEVOLDOWN		6
#define VOLCMD_VIBRATOSPEED		7
#define VOLCMD_VIBRATO			8
#define VOLCMD_PANSLIDELEFT		9
#define VOLCMD_PANSLIDERIGHT	        10
#define VOLCMD_TONEPORTAMENTO	        11
#define VOLCMD_PORTAUP			12
#define VOLCMD_PORTADOWN		13

#endif
