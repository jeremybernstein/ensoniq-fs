//----------------------------------------------------------------------------
// EnsoniqUnpackerEFE plugin v1.21 for TotalCommander
//----------------------------------------------------------------------------
// (c) 2006 thoralt@thoralt.de
// 
// Plugin for reading Ensoniq instrument files
//----------------------------------------------------------------------------

// This source code was written using Dev-Cpp 4.9.9.2
// If you want to compile it, get Dev-Cpp. Normally The code should compile
// with other IDEs/compilers too (with small modifications), but I did not
// test it.

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "wcxhead.h"
#include "EnsoniqUnpackerEFE.h"
#include "EnsoniqInstrument.h"

// If you want to have logging built in, uncomment the line below
//#define LOGGING 1
#define LOGFILE	"C:\\EnsoniqUnpackerEFE-LOG.txt"

/*
	1.21: corrected some bugs in key mapping display
	      suppressed more invalid wavesamples in files
	1.2:  release
	1.14: added key mapping to info.txt
	1.13: added lots of text to info.txt
	      all filenames are now "<instrument name>-<num>-<wavesample name>"
	      tolerate non-instrument efe files
	1.12: made DetectEFEInstrument a bit weaker
	1.11: bugfix: files were not always closed correctly
	1.10: released EnsoniqUnpacker/EnsoniqUnpackerEFE
*/

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------
HANDLE_LIST *g_pHandleRoot = NULL;
tProcessDataProc ProgressCallback;

//----------------------------------------------------------------------------
// logging support
//----------------------------------------------------------------------------
#ifdef LOGGING
	void LOG(char *c)
	{
		FILE *pDebug;
		pDebug = fopen(LOGFILE, "a+");
		if(NULL==pDebug) return;
		fprintf(pDebug, c);
		fclose(pDebug);
	}
	void LOG_HEX8(int i)
	{
		char cBuf[16];
		sprintf(cBuf, "0x%08X", i);
		LOG(cBuf);
	}
	void LOG_HEX2(int i)
	{
		char cBuf[16];
		sprintf(cBuf, "0x%02X", i);
		LOG(cBuf);
	}
	void LOG_INT(int i)
	{
		char cBuf[16];
		sprintf(cBuf, "%i", i);
		LOG(cBuf);
	}
#else
	#define LOG(x)		{}
	#define LOG_HEX8(x) {}
	#define LOG_HEX2(x) {}
	#define LOG_INT(x) 	{}
#endif

//----------------------------------------------------------------------------
// GetSampleRate
// 
// Converts an Ensoniq sample rate to an unsigned int
// 
// -> o: sample rate to convert
// <- converted plain sample rate in Hz
//----------------------------------------------------------------------------
int GetSampleRate(int SR)
{
	if(SR>100) return SR;
	switch(SR)
	{
		case  0: return 48000;
		case  1: return 44100;
		default: return 1.0f/(((float)SR)*(1.6f/1000000.0f));
	}
}

//----------------------------------------------------------------------------
// GetOffset
// 
// Converts an EPSOFFSET structure to an unsigned int
// 
// -> o: EPSOFFSET to convert
// <- converted plain offset
//----------------------------------------------------------------------------
unsigned int GetOffset(EPSOFFSET O)
{
	return (O.Data[0]<<12)+(O.Data[1]<<4)+(O.Data[2]<<4)+(O.Data[3]>>4);
}

//----------------------------------------------------------------------------
// GetLong
// 
// Converts an EPSLONG structure to an unsigned int
// 
// -> L: EPSLONG to convert
// <- converted plain long
//----------------------------------------------------------------------------
unsigned int GetLong(EPSLONG L)
{
  return ((long) L.Data[0] << 4)  | ((long) L.Data[1] >> 4)
       | ((long) L.Data[2] << 16) | ((long) L.Data[3] << 8);
}

/*
//----------------------------------------------------------------------------
// GetWord
// 
// Converts an ensoniq word to an unsigned int
// 
// -> L: WORD to convert
// <- converted plain long
//----------------------------------------------------------------------------
unsigned int GetWord(WORD w)
{
  return ((w>>8)&0xFF) | ((w<<8)&0xFF00);
}
*/

//----------------------------------------------------------------------------
// GetArcNameWithoutPath
//
// Extracts the name of the archive out of the full filename
//
// -> pHandle = pointer to valid handle structure
//    cArcName = pointer to destination string
// <- --
//----------------------------------------------------------------------------
void GetArcNameWithoutPath(HANDLE_LIST *pHandle, char *cArcName)
{
	int i;
	
	// check pointers
	if(NULL==pHandle) return;
	if(NULL==cArcName) return;
	cArcName[0] = 0;
	
	// find last delimiter '\' or '/'
	for(i=strlen(pHandle->cArcName); i>=0; i--)
	{
		if(('\\'==pHandle->cArcName[i])||('/'==pHandle->cArcName[i])) break;
	}
	
	if(0!=i) i++;
	
	strcpy(cArcName, pHandle->cArcName + i);
}

//----------------------------------------------------------------------------
// AddFileinfoString
//
// Adds a string to the file info block of a given handle
//
// -> pHandle = pointer to valid handle structure
//    c = string
// <- 0: OK
//    1: error
//----------------------------------------------------------------------------
int AddFileinfoString(HANDLE_LIST *pHandle, char *c)
{
	int iLen = 0;
	char *cTemp;
	
	// check pointer
	if(NULL==pHandle) return 1;
	
	if(NULL==pHandle->cInfotext) iLen = 0;
	else iLen = strlen(pHandle->cInfotext);

	// create new string containing previous text + new text
	cTemp = malloc(strlen(c)+1+iLen);
	if(NULL==cTemp) return 1;
	if(pHandle->cInfotext) strcpy(cTemp, pHandle->cInfotext);
	strcat(cTemp, c);
	
	// free old text, save new text
	free(pHandle->cInfotext);
	pHandle->cInfotext = cTemp;
	
	return 0;
}

//----------------------------------------------------------------------------
// ReadSamples
//
// Reads an EFE instrument, scans for sample names, adds the contents to name
// list
//
// -> pHandle = pointer to valid handle structure
// <- 0: OK
//    1: error
//----------------------------------------------------------------------------
int ReadSamples(HANDLE_LIST *pHandle)
{
/*
cKeys[128] = {
  "C-2", "C#-2", "D-2", "D#-2", "E-2", "F-2", "F#-2", "G-2", 
  "G#-2", "A-2", "A#-2", "B-2"
  "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", 
  "G#-1", "A-1", "A#-1", "B-1", 
  "C0", "C#0", "D0", "D#0", 
  "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0", 
  "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", 
  "G#1", "A1", "A#1", "B1", 
  "C2", "C#2", "D2", "D#2", 
  "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2", 
  "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", 
  "G#3", "A3", "A#3", "B3", 
  "C4", "C#4", "D4", "D#4", 
  "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4", 
  "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", 
  "G#5", "A5", "A#5", "B5", 
  "C6", "C#6", "D6", "D#6", 
  "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6", 
  "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", 
  "G#7", "A7", "A#7", "B7", 
  "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8"
};
*/
	char *cKeys[12] = {
		"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
	
	int i, iOffset, j, iLen, iStart, iSampleRate, k, iStartKey, iLastKey,
		iIsFirstMapping;
	char cFN[17], cOriginalFN[13], cTemp[8192], cArcName[260], cLayerName[13],
		 cWSMapping[8192];
	FILE_LIST *pListEntry;
	WAVESAMPLEHEADER WSH;
	INSTRUMENTHEADER IH;
	LAYERHEADER LH[8];

	// read header
	i = sizeof(INSTRUMENTHEADER);
	if(i!=fread(&IH, 1, i, pHandle->pFile))
	{
		LOG("Error reading instrument header.\n");
		return 1;
	}

	// load all layers
	for(i=0; i<8; i++)
	{
		LOG("Layer "); LOG_INT(i+1); LOG(": header start=");
		iOffset = GetOffset(IH.LayerOfs[i]);
		LOG_HEX8(iOffset);

		// skip non-existent layers
		if(!iOffset)
		{
			LOG(" (empty)\n");
			LH[i].HeaderSize.Data[0] = 0x00;
			LH[i].HeaderSize.Data[1] = 0x00;
			LH[i].HeaderSize.Data[2] = 0x00;
			LH[i].HeaderSize.Data[3] = 0x00;
			continue;
		}
		
		// seek to position of layer
		if(fseek(pHandle->pFile, iOffset + pHandle->iImageOffset, SEEK_SET))
		{
			LOG(" error seeking file position.\n");
			return 1;
		}
		
		// read header
		j = sizeof(LAYERHEADER);
		if(j!=fread(&LH[i], 1, j, pHandle->pFile))
		{
			LOG(" error reading LAYERHEADER.\n");
			return 1;
		}

		// skip null length layers
		if(!GetLong(LH[i].HeaderSize))
		{
			LOG(" (zero HeaderSize)\n");
			continue;
		}

		// skip layers with illegal layer num
		if(LH[i].LayerNum!=(i+1))
		{
			LOG(" (illegal layer num)\n");
			continue;
		}

		// read layername
		memset(cLayerName, 0, 13);
		for(j=0; j<12; j++)
		{
			cLayerName[j] = LH[i].LayerName.Data[j];
		}
		LOG(", name=\""); LOG(cLayerName); LOG("\"\n");
		sprintf(cTemp, "Layer %i: name=\"%s\"\r\n", i+1, cLayerName);
		AddFileinfoString(pHandle, cTemp);
/*
		sprintf(cTemp, "HeaderSize=%i, SavedFrom=%i, LayerNum=%i, "
			"LayerNumAgain=%i, GlideMode=%i, DelayVelAmt=%i, GlideTime=%i,"
			"RestrikeDelay=%i, LegatoLayer=%i, VelLo=%i, VelHi=%i, "
			"PitchTable=%i, LayerDelay=%i\r\n",
			LH[i].HeaderSize,
			LH[i].SavedFrom,
			LH[i].LayerNum,
			LH[i].LayerNumAgain,
			LH[i].GlideMode,
			LH[i].DelayVelAmt,
			LH[i].GlideTime,
			LH[i].RestrikeDelay,
			LH[i].LegatoLayer,
			LH[i].VelLo,
			LH[i].VelHi,
			LH[i].PitchTable,
			LH[i].LayerDelay);
		AddFileinfoString(pHandle, cTemp);
*/
	}

	// loop through all wavesamples
	for(i=1; i<128; i++)
	{
		LOG("WS "); LOG_INT(i); LOG(": header start=");
		iOffset = GetOffset(IH.WaveOfs[i]);
		LOG_INT(iOffset);

		// skip non-existent samples
		if(!iOffset)
		{
			LOG(" (empty)\n");
			continue;
		}
		
		// seek to position of wavesample
		if(fseek(pHandle->pFile, iOffset + pHandle->iImageOffset, SEEK_SET))
		{
			LOG(" error seeking file position.\n");
			return 1;
		}
		
		// read header
		j = sizeof(WAVESAMPLEHEADER);
		if(j!=fread(&WSH, 1, j, pHandle->pFile))
		{
			LOG(" error reading WAVESAMPLEHEADER.\n");
			return 1;
		}

		// skip null-length samples	
		if(sizeof(WAVESAMPLEHEADER)>GetLong(WSH.HeaderSize))
		{
			LOG(" (size=0)\n");
			continue;
		}
		
		// skip illegal CopyWave or CopyLayer values
		if((WSH.CopyWave>127)||(WSH.CopyLayer>7))
		{
			LOG(" (illegal CopyWave or CopyLayer)\n");
			continue;
		}
		
		LOG(", CopyWave="); LOG_INT(WSH.CopyWave);

		sprintf(cTemp, "WS %03i: ", i);
		AddFileinfoString(pHandle, cTemp);
/*
		sprintf(cTemp,
			"\r\n    HeaderSize=%i, SavedFrom=%i, Zero1=%i, Zero2=%i,"
			"\r\n    CopyWave=%i, CopyLayer=%i, RootKey=%i, AModCurve=%i,"
			"\r\n    PitchEnvAmt=%i, LFOAmt=%i, LA0=%i, RandomAmt=%i,"
			"\r\n    PitchBend=%i, MixerAModCurve=%i, ModSource=%i,"
			"\r\n    FineTune=%i, FT0=%i, FilterMode=%i, MixerBModSource=%i,"
			"\r\n    FC1Cutoff=%i, FC2Cutoff=%i, FC1KeyAmt=%i, FC2KeyAmt=%i,"
			"\r\n    FC1ModSource=%i, MixerAModAmt=%i, FC2ModSource=%i,"
			"\r\n    MixerBModAmt=%i, FC1ModAmt=%i, FC2ModAmt=%i, Vol=%i,"
			"\r\n    BusSelect=%i, AmpModSource=%i, PanModSource=%i,"
			"\r\n    XFadeA=%i, XFadeB=%i, XFadeC=%i, XFadeD=%i,"
			"\r\n    Pan=%i, PanFrac=%i, AmpModAmt=%i, PanModAmt=%i,"
			"\r\n    LFOWave=%i, LFOBoost=%i, LFOSpeed=%i, LFORateModAmt=%i,"
			"\r\n    LFODepth=%i, LFODelay=%i, LFOModAmt=%i, LFOModSource=%i,"
			"\r\n    LFORateModSource=%i, LFOMode=%i, RandomModFreq=%i,"
			"\r\n    LoopMode=%i, SampleStart=%i, SampleStartFrac=%i,"
			"\r\n    SampleEnd=%i, SampleEndFrac=%i, LoopStart=%i,"
			"\r\n    LoopStartFrac=%i, LoopEnd=%i, LoopEndFrac=%i,"
			"\r\n    SampleRate=%i, KeyLo=%i, KeyHi=%i"
			"\r\n    StartLoopModSource=%i, DelayModAmt=%i,"
			"\r\n    StartLoopModAmt=%i, StartLoopModRange=%i, ModType=%i\r\n",
			WSH.HeaderSize, WSH.SavedFrom, WSH.Zero1, WSH.Zero2,
			WSH.CopyWave, WSH.CopyLayer, WSH.RootKey,
			WSH.AModCurve, WSH.PitchEnvAmt, WSH.LFOAmt, WSH.LA0, WSH.RandomAmt, 
			WSH.PitchBend, WSH.MixerAModCurve, WSH.ModSource, WSH.FineTune, 
			WSH.FT0, WSH.ModAmt, WSH.FilterMode, WSH.MixerBModSource, 
			WSH.FC1Cutoff, WSH.FC2Cutoff, WSH.FC1KeyAmt, WSH.FC2KeyAmt,
			WSH.FC1EnvAmt, WSH.FC2EnvAmt, WSH.FC1ModSource, WSH.MixerAModAmt, 
			WSH.FC2ModSource, WSH.MixerBModAmt, WSH.FC1ModAmt, WSH.FC2ModAmt, 
			WSH.Vol, WSH.BusSelect, WSH.AmpModSource, WSH.PanModSource, 
			WSH.XFadeA, WSH.XFadeB, WSH.XFadeC, WSH.XFadeD,	WSH.Pan, 
			WSH.PanFrac, WSH.AmpModAmt, WSH.PanModAmt, WSH.LFOWave, 
			WSH.LFOBoost, WSH.LFOSpeed, WSH.LFORateModAmt, WSH.LFODepth, 
			WSH.LFODelay, WSH.LFOModAmt, WSH.LFOModSource,
			WSH.LFORateModSource, WSH.LFOMode, WSH.RandomModFreq, WSH.LoopMode,
			WSH.SampleStart, WSH.SampleStartFrac, WSH.SampleEnd, 
			WSH.SampleEndFrac, WSH.LoopStart, WSH.LoopStartFrac, WSH.LoopEnd, 
			WSH.LoopEndFrac, WSH.SampleRate, WSH.KeyLo, WSH.KeyHi, 
			WSH.StartLoopModSource, WSH.DelayModAmt, WSH.StartLoopModAmt,
			WSH.StartLoopModRange, WSH.ModType);
		AddFileinfoString(pHandle, cTemp);
*/

		// check all layers for key mapping
		memset(cWSMapping, 0, 8192);
		iIsFirstMapping = 1;
		for(j=0; j<8; j++)
		{
			iStartKey = -2; iLastKey = -2;
			for(k=0; k<88; k++)
			{
				// skip empty layers
				if(!GetLong(LH[j].HeaderSize)) continue;
				
				// current wavesample found?
				if(LH[j].LayerMap[k]==i)
				{
					// is this the first occurence in this layer?
					if(-2==iStartKey)
					{
						// print comma if necessary
						if(iIsFirstMapping)
						{
							iIsFirstMapping = 0;
						}
						else
						{
							strcat(cWSMapping, ", ");
						}			
									
						// print layer
						sprintf(cTemp, "layer %i: ", j+1);
						strcat(cWSMapping, cTemp);
						iStartKey = k;
					}

					// was there a gap in the mapping for WS <i>?
					if(-1==iStartKey)
					{
						// set start key
						iStartKey = k;

						// print comma for next key
						strcat(cWSMapping, ", ");
					}
					
					iLastKey = k;
				}
				else
				// this is _not_ the current WS
				{
					// was the last entry of layer map the current <i> WS?
					// if so, output mapping range and reset starting values
					if(iStartKey>-1)
					{
						if(iStartKey!=iLastKey)
						{
							// output start and end key
							sprintf(cTemp, "%s%i/%s%i",
								cKeys[iStartKey%12], (iStartKey-3)/12 - 3,
								cKeys[iLastKey%12], (iLastKey-3)/12 - 3);
							strcat(cWSMapping, cTemp);
						}						
						else
						{
							// output only one key
							sprintf(cTemp, "%s%i",
								cKeys[iStartKey%12], (iStartKey-3)/12 - 3);
							strcat(cWSMapping, cTemp);
						}						
						
						// reset iStartKey
						iStartKey = -1;
					}
				}
			}
			
			// is the mapping including the highest possible key?
			// if so, output it (it was not output before)
			if(iStartKey>-1)
			{
				if(iStartKey!=iLastKey)
				{
					// output start and end key
					sprintf(cTemp, "%s%i/%s%i",
						cKeys[iStartKey%12], (iStartKey-3)/12 - 3,
						cKeys[iLastKey%12], (iLastKey-3)/12 - 3);
					strcat(cWSMapping, cTemp);
				}						
				else
				{
					// output only one key
					sprintf(cTemp, "%s%i",
						cKeys[iStartKey%12], (iStartKey-3)/12 - 3);
					strcat(cWSMapping, cTemp);
				}						
			}
		}
	
		// skip wavesample copies
		if(WSH.CopyWave)
		{
			sprintf(cTemp, "is a copy of wavesample %i, mapping: %s\r\n",
				WSH.CopyWave&0xFF, cWSMapping);
			AddFileinfoString(pHandle, cTemp);
			LOG(" is a copy, skipping; mapping: ");
			LOG(cWSMapping); LOG("\n");
			
			continue;
		}
		
		// read filename		
		memset(cFN, 0, 17);
		memset(cOriginalFN, 0, 13);
		for(j=0; j<12; j++)
		{
			cFN[j] = WSH.WSName.Data[j];
			cOriginalFN[j] = WSH.WSName.Data[j];
		}

		// trim right spaces
		for(j=12; j>=0; j--)
		{
			if(0==cFN[j]) continue;
			else if(32==cFN[j])
			{
				cFN[j] = 0;
				continue;
			}
			else break;
		}

		// replace illegal chars for windows compatibility
		for(j=12; j>=0; j--)
		{
			switch(cFN[j])
			{
				case '*':
				case '?':
				case '\\':
				case '/':
				case '|':
				case '>':
				case '<':
				case ':':
					cFN[j] = '_';
					break;
					
				default:
					break;
			}
		}

		// calculate file properties
		iLen = GetLong(WSH.HeaderSize) - sizeof(WAVESAMPLEHEADER);
		iStart = iOffset + pHandle->iImageOffset + sizeof(WAVESAMPLEHEADER);
		iSampleRate = GetSampleRate(WSH.SampleRate);
		LOG(", name=\""); LOG(cOriginalFN);
		LOG("\", start "); LOG_INT(iStart);
		LOG(", len "); LOG_INT(iLen);
		LOG(", sample rate="); LOG_INT(iSampleRate);
		LOG(", mapping: "); LOG(cWSMapping);
		LOG("\n");

		sprintf(cTemp, "name=\"%s\", size=%i bytes, "
			"sample rate=%i Hz\r\n        mapping: %s\r\n",
			cOriginalFN, iLen, iSampleRate, cWSMapping);
		AddFileinfoString(pHandle, cTemp);
		
		// create new file list entry
		pListEntry = malloc(sizeof(FILE_LIST));
		if(NULL==pListEntry) return 1;
		memset(pListEntry, 0, sizeof(FILE_LIST));
		
		// copy path and name
		GetArcNameWithoutPath(pHandle, cArcName);
		sprintf(pListEntry->HeaderData.FileName, "%s-%03i-%s.wav", 
			cArcName, i, cFN);
		strcpy(pListEntry->cOriginalName, cOriginalFN);
		
		// copy file data
		pListEntry->iLen = iLen;
		pListEntry->iStart = iStart;
		pListEntry->iSampleRate = iSampleRate;
		
		pListEntry->HeaderData.FileAttr = 0x20;
		pListEntry->HeaderData.PackSize = iLen;
		pListEntry->HeaderData.UnpSize = iLen;
		
		// is this the first entry?
		if(NULL==pHandle->pFileList)
		{
			// set this file as root of the file list
			pHandle->pFileList = pListEntry;
		}
		else
		{
			pHandle->pLastFile->pNext = pListEntry;
		}
		
		pHandle->pLastFile = pListEntry;
	}
	
	return 0;
}

//----------------------------------------------------------------------------
// DetectEFE
//
// Try to detect the open file as an EFE file
//
// -> ucBuf: pointer to first 512 bytes of file
// <-  >0: EFE file detected (return value equals detected type, see below)
//     0: EFE not detected
//----------------------------------------------------------------------------
int DetectEFE(unsigned char *ucBuf, HANDLE_LIST *pHandle)
{
	if((ucBuf[0x00]!=0x0D) || (ucBuf[0x01]!=0x0A))
	{
		LOG("[0x00]!=0x0D || [0x01]!=0x0A, no EFE file.\n");
		return 0;
	}
	
//	if((ucBuf[0x2F]!=0x0D) || (ucBuf[0x30]!=0x0A) || (ucBuf[0x31]!=0x1A))
//	{
//		LOG("[0x2F}!=0x0D || [0x30]!=0x0A || {0x31]!=0x1A, "
//			"no EFE instrument.\n");
//		return 0;
//	}

	switch(ucBuf[0x32])
	{
		case 1:
			AddFileinfoString(pHandle, "Filetype 01: EPS operating system. "
				"No further information available.\r\n");
			break;

		case 2:
			AddFileinfoString(pHandle, "Filetype 02: Directory. "
				"No further information available.\r\n");
			break;

		case 3:
			AddFileinfoString(pHandle, "Filetype 03: Ensoniq instrument.\r\n");
			break;

		case 4:
			AddFileinfoString(pHandle, "Filetype 04: EPS bank. "
				"No further information available.\r\n");
			break;

		case 5:
			AddFileinfoString(pHandle, "Filetype 05: EPS sequence. "
				"No further information available.\r\n");
			break;

		case 6:
			AddFileinfoString(pHandle, "Filetype 06: EPS song. "
				"No further information available.\r\n");
			break;

		case 7:
			AddFileinfoString(pHandle, "Filetype 07: EPS sysex. "
				"No further information available.\r\n");
			break;

		case 8:
			AddFileinfoString(pHandle, "Filetype 08: Directory. "
				"No further information available.\r\n");
			break;

		case 9:
			AddFileinfoString(pHandle, "Filetype 09: EPS macro. "
				"No further information available.\r\n");
			break;

		case 10:
			AddFileinfoString(pHandle, "Filetype 10: VFX1PROG. "
				"No further information available.\r\n");
			break;

		case 11:
			AddFileinfoString(pHandle, "Filetype 11: VFX6PROG. "
				"No further information available.\r\n");
			break;

		case 12:
			AddFileinfoString(pHandle, "Filetype 12: VFX30PROG. "
				"No further information available.\r\n");
			break;

		case 13:
			AddFileinfoString(pHandle, "Filetype 13: VFX60PROG. "
				"No further information available.\r\n");
			break;

		case 14:
			AddFileinfoString(pHandle, "Filetype 14: VFX1PRESET. "
				"No further information available.\r\n");
			break;

		case 15:
			AddFileinfoString(pHandle, "Filetype 15: VFX10PRESET. "
				"No further information available.\r\n");
			break;

		case 16:
			AddFileinfoString(pHandle, "Filetype 16: VFX20PRESET. "
				"No further information available.\r\n");
			break;

		case 17:
			AddFileinfoString(pHandle, "Filetype 17: VFX1SONG. "
				"No further information available.\r\n");
			break;

		case 18:
			AddFileinfoString(pHandle, "Filetype 18: VFX30SONG. "
				"No further information available.\r\n");
			break;

		case 19:
			AddFileinfoString(pHandle, "Filetype 19: VFX60SONG. "
				"No further information available.\r\n");
			break;

		case 20:
			AddFileinfoString(pHandle, "Filetype 20: VFX1SYSEX. "
				"No further information available.\r\n");
			break;

		case 21:
			AddFileinfoString(pHandle, "Filetype 21: VFX1SETUP. "
				"No further information available.\r\n");
			break;

		case 22:
			AddFileinfoString(pHandle, "Filetype 22: VFX operating system. "
				"No further information available.\r\n");
			break;

		case 23:
			AddFileinfoString(pHandle, "Filetype 23: EPS16+ bank. "
				"No further information available.\r\n");
			break;

		case 24:
			AddFileinfoString(pHandle, "Filetype 24: EPS16+ effect. "
				"No further information available.\r\n");
			break;

		case 25:
			AddFileinfoString(pHandle, "Filetype 25: EPS16+ sequence. "
				"No further information available.\r\n");
			break;

		case 26:
			AddFileinfoString(pHandle, "Filetype 26: EPS16+ song. "
				"No further information available.\r\n");
			break;

		case 27:
			AddFileinfoString(pHandle, "Filetype 27: EPS16+ operating system. "
				"No further information available.\r\n");
			break;

		case 28:
			AddFileinfoString(pHandle, "Filetype 28: ASR sequence. "
				"No further information available.\r\n");
			break;

		case 29:
			AddFileinfoString(pHandle, "Filetype 29: ASR song. "
				"No further information available.\r\n");
			break;

		case 30:
			AddFileinfoString(pHandle, "Filetype 30: ASR bank. "
				"No further information available.\r\n");
			break;

		case 31:
			AddFileinfoString(pHandle, "Filetype 31: ASR track. "
				"No further information available.\r\n");
			break;

		case 32:
			AddFileinfoString(pHandle, "Filetype 32: ASR operating system. "
				"No further information available.\r\n");
			break;

		case 33:
			AddFileinfoString(pHandle, "Filetype 33: ASR effect. "
				"No further information available.\r\n");
			break;

		default:
			LOG("[0x32}="); LOG_HEX2(ucBuf[0x32]); 
			LOG(", no EFE instrument.\n");
			return 0;
			break;
			
	}
	
	return ucBuf[0x32];
}

//----------------------------------------------------------------------------
// FreeHandle
// 
// Frees all the memory allocated by the handle
// 
// -> pHandle: pointer to handle
// <- --
//----------------------------------------------------------------------------
void FreeHandle(HANDLE_LIST *pHandle)
{
	FILE_LIST *pFileList, *pFileListTemp;

	LOG("FreeHandle("); LOG_HEX8((int)pHandle); LOG(")\n");
	// close associated file
	if(pHandle->pFile) fclose(pHandle->pFile);
	
	// free info text
	if(pHandle->cInfotext) free(pHandle->cInfotext);
	
	// remove filename list
	if(pHandle->pFileList)
	{
		pFileList = pHandle->pFileList;
		while(pFileList)
		{
			pFileListTemp = pFileList->pNext;
			free(pFileList);
			pFileList = pFileListTemp;
		}
	}
	
	// free allocated memory
	free(pHandle);
}

//----------------------------------------------------------------------------
// OpenArchive
//
// nOpenArchive should perform all necessary operations when an archive is to 
// be opened.
//
// OpenArchive should return a unique handle representing the archive. The
// handle should remain valid until CloseArchive is called. If an error 
// occurs, you should return zero, and specify the error by setting 
// OpenResult member of ArchiveData.
//
// You can use the ArchiveData to query information about the archive being
// open, and store the information in ArchiveData to some location that can 
// be accessed via the handle.
//
// ->
// <-
//----------------------------------------------------------------------------
DLLIMPORT HANDLE __stdcall OpenArchive(tOpenArchiveData *ArchiveData)
{
	HANDLE_LIST *pHandle, *pTemp;
	unsigned char ucBuf[512], ucDetectOnly = 0, ucFiletype = 0;
	int iImageOffset = 0, i;
	FILE *pFile;

	LOG("\n--------------------------------------------------------------\n");
	
	// filetype detection only?
	if('?'==ArchiveData->ArcName[strlen(ArchiveData->ArcName)-1])
	{
		ArchiveData->ArcName[strlen(ArchiveData->ArcName)-1] = 0x00;
		ucDetectOnly = 1;
	}
	
	// try to open file for reading
	LOG("fopen(\""); LOG(ArchiveData->ArcName); LOG("\"): ");
	pFile = fopen(ArchiveData->ArcName, "rb");
	if(NULL==pFile)
	{
		LOG("failed.\n");
		ArchiveData->OpenResult = E_EOPEN;
		return 0;	// Error
	}
	LOG("OK.\n");
	
	// seek to beginning of file
	fseek(pFile, 0, SEEK_SET);
	
	// read 512 bytes
	if(512!=fread(ucBuf, 1, 512, pFile))
	{
		LOG("fread(512) (reading id-header) failed.\n");
		ArchiveData->OpenResult = E_EREAD;
		fclose(pFile);
		return 0;	// Error
	}

	LOG("Reading header: OK\n");
	
	// I'm using this more complex structure stuff instead of a simple
	// OS-level file handle to be able to store more status information and
	// to be able to clean up everything when DLL is being unloaded
	
	// allocate new handle structure
	pHandle = malloc(sizeof(HANDLE_LIST));
	if(NULL==pHandle)
	{
		ArchiveData->OpenResult = E_NO_MEMORY;
		LOG("Unable to allocate new handle structure.\n");
		fclose(pFile);
		return 0;	// Error
	}
	memset(pHandle, 0, sizeof(HANDLE_LIST));

	// try to detect instrument file
	LOG("Detecting file type: ");
	ucFiletype = DetectEFE(ucBuf, pHandle);
	LOG_HEX2(ucFiletype); LOG("\n");
	if(0!=ucFiletype)
	{
		iImageOffset = 512;
	}
	else
	{
		LOG("Unknown instrument format.\n");
		ArchiveData->OpenResult = E_UNKNOWN_FORMAT;
		FreeHandle(pHandle);
		fclose(pFile);
		return 0;
	}
	
	// fill in values
	pHandle->pFile = pFile;
	pHandle->iImageOffset = iImageOffset;
	pHandle->ucFiletype = ucFiletype;
	strcpy(pHandle->cArcName, ArchiveData->ArcName);
	
	// is this the first handle?
	if(NULL==g_pHandleRoot)
	{
		pHandle->pPrevious = NULL;
		g_pHandleRoot = pHandle;
	}
	else
	{
		// find last handle in list
		pTemp = g_pHandleRoot;
		while(pTemp->pNext) pTemp = pTemp->pNext;
		
		// attach newly created handle to last member of list
		pTemp->pNext = pHandle;
		pHandle->pPrevious = pTemp;
	}

	// read files only if being called by TotalCommander directly
	if(!ucDetectOnly)
	{
		if(0x03==ucFiletype) ReadSamples(pHandle);

		// set current file to be the first file for ReadHeader
		pHandle->pCurrentFile = (FILE_LIST*)0xFFFFFFFF;
		pHandle->cInfofile = 1;

		AddFileinfoString(pHandle, "\r\n\r\nEnsoniqUnpackerEFE "
			"plugin v1.21 for TotalCommander\r\n"
			"(c) 2006 thoralt@thoralt.de\r\n");
	}

	LOG("Returning from OpenArchive(): "); LOG_HEX8((int)pHandle); LOG("\n");
	
	// pHandle is now a valid new handle structure coupled with an open file
	return pHandle;
}

//----------------------------------------------------------------------------
// ReadHeader
//
// Totalcmd calls ReadHeader to find out what files are in the archive.
// 
// ReadHeader is called as long as it returns zero (as long as the previous 
// call to this function returned zero). Each time it is called, HeaderData 
// is supposed to provide Totalcmd with information about the next file 
// contained in the archive. When all files in the archive have been returned,
// ReadHeader should return E_END_ARCHIVE which will prevent ReaderHeader from
// being called again. If an error occurs, ReadHeader should return one of the
// error values or 0 for no error.
// 
// hArcData contains the handle returned by OpenArchive. The programmer is 
// encouraged to store other information in the location that can be accessed 
// via this handle. For example, you may want to store the position in the 
// archive when returning files information in ReadHeader.
// 
// In short, you are supposed to set at least PackSize, UnpSize, FileTime, and 
// FileName members of tHeaderData. Totalcmd will use this information to 
// display content of the archive when the archive is viewed as a directory.
//
// ->
// <-
//----------------------------------------------------------------------------
DLLIMPORT int __stdcall ReadHeader(HANDLE hArcData, tHeaderData *HeaderData)
{
	HANDLE_LIST *pHandle = (HANDLE_LIST*) hArcData;
	char cArcName[260];

	// is this the first call to ReadHeader?
	if((FILE_LIST*)0xFFFFFFFF==pHandle->pCurrentFile)
	{
		if(pHandle->cInfofile)
		{
			// display infofile first
			pHandle->cInfofile = 0;
			GetArcNameWithoutPath(pHandle, cArcName);
			strcpy(HeaderData->FileName, cArcName);
			strcat(HeaderData->FileName, "-info.txt");
			HeaderData->FileAttr = 0x20;
			HeaderData->PackSize = strlen(pHandle->cInfotext);
			HeaderData->UnpSize = strlen(pHandle->cInfotext);
			return 0; // OK
		}
		else
		{
			// display first file second
			pHandle->pCurrentFile = pHandle->pFileList;
		}
	}
	else
	{
		pHandle->pCurrentFile = pHandle->pCurrentFile->pNext;
	}
	
	// did we reach the end?
	if(NULL==pHandle->pCurrentFile) return E_END_ARCHIVE;

	// copy data	
	memcpy(HeaderData, &pHandle->pCurrentFile->HeaderData,
		sizeof(tHeaderData));
	
	return 0; // OK
}

//----------------------------------------------------------------------------
// ProcessFile
// 
// ProcessFile should unpack the specified file or test the integrity of the
// archive.
// 
// ProcessFile should return zero on success, or one of the error values
// otherwise.
// 
// hArcData contains the handle previously returned by you in OpenArchive. 
// Using this, you should be able to find out information (such as the archive
// filename) that you need for extracting files from the archive.
// Unlike PackFiles, ProcessFile is passed only one filename. Either DestName
// contains the full path and file name and DestPath is NULL, or DestName
// contains only the file name and DestPath the file path. This is done for
// compatibility with unrar.dll.
// 
// When Total Commander first opens an archive, it scans all file names with
// OpenMode==PK_OM_LIST, so ReadHeader() is called in a loop with calling 
// ProcessFile(...,PK_SKIP,...). When the user has selected some files and 
// started to decompress them, Total Commander again calls ReadHeader() in a
// loop. For each file which is to be extracted, Total Commander calls
// ProcessFile() with Operation==PK_EXTRACT immediately after the ReadHeader()
// call for this file. If the file needs to be skipped, it calls it with
// Operation==PK_SKIP.
// 
// Each time DestName is set to contain the filename to be extracted, tested,
// or skipped. To find out what operation out of these last three you should
// apply to the current file within the archive, Operation is set to one of
// the following:
// 
// Constant		Value	Description
// PK_SKIP		0		Skip this file
// PK_TEST		1		Test file integrity
// PK_EXTRACT	2		Extract to disk
//
// ->
// <-
//----------------------------------------------------------------------------
DLLIMPORT int __stdcall ProcessFile (HANDLE hArcData, int Operation,
									 char *DestPath, char *DestName)
{
	HANDLE_LIST *pHandle = (HANDLE_LIST*) hArcData;
	unsigned char ucBuf[512];
	RIFF_WAVE RiffWave;
	FILE_LIST *pFile;
	int i, iBlock;
	char cFN[260];
	FILE *f;
	
	// skip or test files
	if(PK_SKIP==Operation) return 0;
	if(PK_TEST==Operation) return 0;
	
	// extract file
	if(PK_EXTRACT!=Operation) return 0;
	
	LOG("ProcessFile()\n");
	
	// grab the file which was accessed with the last call of ReadHeader
	// (this should be exactly the file which is asked to be extracted)
	pFile = pHandle->pCurrentFile;
	
	if(NULL==DestPath)
	{
		strcpy(cFN, DestName);
	}
	else
	{
		strcpy(cFN, DestPath);
		strcat(cFN, DestName);
	}

	// is this the info file?
	if((FILE_LIST*)0xFFFFFFFF==pFile)
	{
		LOG("ProcessFile/Extract: infofile open ");
		
		f = fopen(cFN, "wb");
		if(NULL==f)
		{
			LOG("failed.\n");
			return E_ECREATE;
		}
		LOG("OK.\n");
		
		// todo: add some more info here
		fprintf(f, pHandle->cInfotext);

		fclose(f);
		return 0; // OK
	}

	// begin sample extraction
	LOG("ProcessFile/Extract: \""); LOG(cFN); LOG("\" open ");
	f = fopen(cFN, "wb");
	if(NULL==f)
	{
		LOG("failed.\n");
		return E_ECREATE;
	}
	LOG("OK.\n");
	
	// writing header
	LOG("Writing header... ");
	RiffWave.RIFF[0] = 'R';
	RiffWave.RIFF[1] = 'I';
	RiffWave.RIFF[2] = 'F';
	RiffWave.RIFF[3] = 'F';
	RiffWave.Size = pFile->iLen + 36;
	RiffWave.WAVE[0] = 'W';
	RiffWave.WAVE[1] = 'A';
	RiffWave.WAVE[2] = 'V';
	RiffWave.WAVE[3] = 'E';
	RiffWave.fmt[0] = 'f';
	RiffWave.fmt[1] = 'm';
	RiffWave.fmt[2] = 't';
	RiffWave.fmt[3] = ' ';
	RiffWave.fmtSize = 0x10;
	RiffWave.FormatTag = 1;
	RiffWave.Channels = 1;
	RiffWave.SamplesPerSec = pFile->iSampleRate;
	RiffWave.AvgBytesPerSec = RiffWave.SamplesPerSec*2;
	RiffWave.BlockAlign = 2;
	RiffWave.BitsPerSample = 16;
	RiffWave.data[0] = 'd';
	RiffWave.data[1] = 'a';
	RiffWave.data[2] = 't';
	RiffWave.data[3] = 'a';
	RiffWave.dataSize = pFile->iLen;

	i = sizeof(RIFF_WAVE);
	if(i!=fwrite(&RiffWave, 1, i, f))
	{
		LOG("Error writing to destination file.\n");
		fclose(f);
		return E_EWRITE;
	}
	LOG("OK.\n");
	
	// seek to sample start
	fseek(pHandle->pFile, pFile->iStart, SEEK_SET);
	
	// loop through all sample words
	for(i=0; i<(pFile->iLen/2); i++)
	{
		// read next sample word
		if(2!=fread(ucBuf, 1, 2, pHandle->pFile))
		{
			LOG("Error reading sample word from instrument file.\n");
			fclose(f);
			return E_EREAD;
		}
		
		// swap H and L byte
		ucBuf[2] = ucBuf[0]; ucBuf[0] = ucBuf[1]; ucBuf[1] = ucBuf[2];
		
		// write next sample word to file
		if(2!=fwrite(ucBuf, 1, 2, f))
		{
			LOG("Error writing to destination file.\n");
			fclose(f);
			return E_EWRITE;
		}
	}
	LOG("OK.\n");
	fclose(f);

	// notify TotalCmd of progress
	if(0==ProgressCallback(NULL, pFile->HeaderData.UnpSize))
		return E_EABORTED;
	
	return 0;
}

//----------------------------------------------------------------------------
// CloseArchive
// 
// CloseArchive should perform all necessary operations when an archive is
// about to be closed.
// 
// CloseArchive should return zero on success, or one of the error values
// otherwise. It should free all the resources associated with the open
// archive.
// 
// The parameter hArcData refers to the value returned by a programmer within
// a previous call to OpenArchive.
//
// ->
// <-
//----------------------------------------------------------------------------
DLLIMPORT int __stdcall CloseArchive (HANDLE hArcData)
{
	HANDLE_LIST *pHandle = (HANDLE_LIST*) hArcData;

	LOG("Closing archive.\n");
	
	// remove handle from list
	if(pHandle->pPrevious)
	{
		pHandle->pPrevious->pNext = pHandle->pNext;
	}
	else // delete first element of list
	{
		if(NULL==pHandle->pNext)	// is this the first and last?
		{
			g_pHandleRoot = NULL;	// list is now empty
		}
		else
		{
			// make next item the new start of the list
			g_pHandleRoot = pHandle->pNext;
		}
	}

	FreeHandle(pHandle);

	// return with no error
	return 0;
}

//----------------------------------------------------------------------------
// pChangeVolProc1
// 
// This function allows you to notify user about changing a volume when
// packing files.
// 
// pChangeVolProc1 contains a pointer to a function that you may want to call
// when notifying user to change volume (e.g. inserting another diskette). 
// You need to store the value at some place if you want to use it; you can 
// use hArcData that you have returned by OpenArchive to identify that place.
//
// ->
// <-
//----------------------------------------------------------------------------
void __stdcall SetChangeVolProc (HANDLE hArcData,
								 tChangeVolProc pChangeVolProc1)
{
}

//----------------------------------------------------------------------------
// pProcessDataProc
//
// This function allows you to notify user about the progress when you un/pack
// files.
// 
// pProcessDataProc contains a pointer to a function that you may want to call
// when notifying user about the progress being made when you pack or extract
// files from an archive. You need to store the value at some place if you
// want to use it; you can use hArcData that you have returned by OpenArchive
// to identify that place.
//
// ->
// <-
//----------------------------------------------------------------------------
DLLIMPORT void __stdcall SetProcessDataProc (HANDLE hArcData,
											 tProcessDataProc pProcessDataProc)
{
	ProgressCallback = pProcessDataProc;
}

//----------------------------------------------------------------------------
// GetPackerCaps
//
// GetPackerCaps tells Totalcmd what features your packer plugin supports.
// 
// Implement GetPackerCaps to return a combination of the following values:
// 
// Constant				Value	Description
// 
// PK_CAPS_NEW			  1	Can create new archives
// PK_CAPS_MODIFY		  2	Can modify existing archives
// PK_CAPS_MULTIPLE		  4	Archive can contain multiple files
// PK_CAPS_DELETE		  8	Can delete files
// PK_CAPS_OPTIONS		 16	Has options dialog
// PK_CAPS_MEMPACK		 32	Supports packing in memory
// PK_CAPS_BY_CONTENT	 64	Detect archive type by content
// PK_CAPS_SEARCHTEXT	128	Allow searching for text in archives created with
// 						    this plugin
// PK_CAPS_HIDE			256	Don't show packer icon, don't open with Enter but
// 							with Ctrl+PgDn
// 
// Omitting PK_CAPS_NEW and PK_CAPS_MODIFY means PackFiles will never be
// called and so you don’t have to implement PackFiles. Omitting 
// PK_CAPS_MULTIPLE means PackFiles will be supplied with just one file. 
// Leaving out PK_CAPS_DELETE means DeleteFiles will never be called; leaving
// out PK_CAPS_OPTIONS means ConfigurePacker will not be called. 
// PK_CAPS_MEMPACK enables the functions StartMemPack, PackToMem and
// DoneMemPack. If PK_CAPS_BY_CONTENT is returned, Totalcmd calls the 
// function CanYouHandleThisFile when the user presses Ctrl+PageDown on an 
// unknown archive type. Finally, if PK_CAPS_SEARCHTEXT is returned, Total 
// Commander will search for text inside files packed with this plugin. This 
// may not be a good idea for certain plugins like the diskdir plugin, where 
// file contents may not be available. If PK_CAPS_HIDE is set, the plugin 
// will not show the file type as a packer. This is useful for plugins which 
// are mainly used for creating files, e.g. to create batch files, avi files 
// etc. The file needs to be opened with Ctrl+PgDn in this case, because 
// Enter will launch the associated application.
// 
// Important note:
// 
// If you change the return values of this function, e.g. add packing 
// support, you need to reinstall the packer plugin in Total Commander, 
// otherwise it will not detect the new capabilities.
//----------------------------------------------------------------------------
DLLIMPORT int __stdcall GetPackerCaps()
{
	return PK_CAPS_BY_CONTENT;
}

//----------------------------------------------------------------------------
// CanYouHandleThisFile
// 
// CanYouHandleThisFile allows the plugin to handle files with different 
// extensions than the one defined in Total Commander. It is called when the 
// plugin defines PK_CAPS_BY_CONTENT, and the user tries to open an archive 
// with Ctrl+PageDown.
// 
// CanYouHandleThisFile should return true (nonzero) if the plugin recognizes 
// the file as an archive which it can handle. The detection must be by 
// contents, NOT by extension. If this function is not implemented, Totalcmd 
// assumes that only files with a given extension can be handled by the plugin.
// 
// Filename contains the fully qualified name (path+name) of the file to be 
// checked.
//----------------------------------------------------------------------------
DLLIMPORT BOOL __stdcall CanYouHandleThisFile (char *FileName)
{
	tOpenArchiveData ArchiveData;
	HANDLE_LIST *pHandle;
	char ArcName[262];
	
	LOG("\n--------------------------------------------------------------\n");
	LOG("CanYouHandleThisFile(\"");
	LOG(FileName);
	LOG("\"\n"
		"--------------------------------------------------------------\n");

	// copy file name
	strcpy(ArcName, FileName);
	strcat(ArcName, "?");	// Marker for OpenArchive: Do only detecting
	ArchiveData.ArcName = ArcName;
	
	// try to open the archive (detect only)
	pHandle = OpenArchive(&ArchiveData);
	if(pHandle)
	{
		CloseArchive((HANDLE)pHandle);
		return 1;	// yes, we can handle this archive
	}
	
	CloseArchive((HANDLE)pHandle);
	return 0;		// no, unknown archive
}

//----------------------------------------------------------------------------
// DllMain
//
// ->
// <-
//----------------------------------------------------------------------------
BOOL APIENTRY DllMain (HINSTANCE hInst /*Library instance handle.*/,
                       DWORD reason    /*Reason why function is being called.*/,
                       LPVOID reserved /*Not used.*/ )
{
	HANDLE_LIST *pHandle, *pTemp;
	
    switch (reason)
    {
		case DLL_PROCESS_ATTACH:
	        break;

		case DLL_PROCESS_DETACH:
			// free all memory
			if(NULL!=g_pHandleRoot)
			{
				// start with first handle
				pHandle = g_pHandleRoot;
				
				// loop through all handles
				while(pHandle)
				{
					pTemp = pHandle->pNext;
					FreeHandle(pHandle);
					pHandle = pTemp;
				}	
			}
			g_pHandleRoot = NULL;
	        break;

		case DLL_THREAD_ATTACH:
    	    break;

      	case DLL_THREAD_DETACH:
        	break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
