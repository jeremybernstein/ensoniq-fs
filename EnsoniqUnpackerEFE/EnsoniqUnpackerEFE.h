//----------------------------------------------------------------------------
// EnsoniqUnpackerEFE plugin v1.21 for TotalCommander
// Plugin for reading Ensoniq instrument files
//
// MAIN H FILE
//----------------------------------------------------------------------------
//
// (c) 2006 Thoralt Franz
//
// This source code was written using Dev-Cpp 4.9.9.2
// If you want to compile it, get Dev-Cpp. Normally the code should compile
// with other IDEs/compilers too (with small modifications), but I did not
// test it.
//
//----------------------------------------------------------------------------
// License
//----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA  02110-1301, USA.
// 
// Alternatively, download a copy of the license here:
// http://www.gnu.org/licenses/gpl.txt
//----------------------------------------------------------------------------
#ifndef _ENSONIQUNPACKEREFE_H_
#define _ENSONIQUNPACKEREFE_H_

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

//----------------------------------------------------------------------------
// struct for file info
//----------------------------------------------------------------------------
typedef struct _FILE_LIST
{
	tHeaderData HeaderData;
	struct _FILE_LIST *pNext;
	unsigned char ucType;
	char cOriginalName[13];
	int iSampleRate;
	int iLen, iStart;
} FILE_LIST;

//----------------------------------------------------------------------------
// struct for open handles
//----------------------------------------------------------------------------
typedef struct _HANDLE_LIST
{
	HANDLE hHandle;
	FILE *pFile;
	int iImageOffset;
	struct _HANDLE_LIST *pNext, *pPrevious;
	FILE_LIST *pFileList, *pLastFile, *pCurrentFile;
	char cInfofile, *cInfotext, cArcName[260];
	unsigned char ucFiletype;
} HANDLE_LIST;

//----------------------------------------------------------------------------
// struct for wave file
//----------------------------------------------------------------------------
typedef struct _RIFF_WAVE
{
  char  RIFF[4];                // = "RIFF"
  DWORD Size;
  char  WAVE[4];                // = "WAVE"
  char  fmt[4];                 // = "fmt "
  DWORD fmtSize;                // = 0x10
  WORD  FormatTag;              // = 1
  WORD  Channels;               // = 1
  DWORD SamplesPerSec;         // = 44100
  DWORD AvgBytesPerSec;        // = 44100*2
  WORD  BlockAlign;             // = 2
  WORD  BitsPerSample;          // = 16
  char  data[4];                // = "data"
  DWORD dataSize;
} RIFF_WAVE;

//----------------------------------------------------------------------------
// DLL-Exports
//----------------------------------------------------------------------------
DLLIMPORT HANDLE __stdcall OpenArchive (tOpenArchiveData *ArchiveData);
DLLIMPORT int __stdcall ReadHeader (HANDLE hArcData, tHeaderData *HeaderData);
DLLIMPORT int __stdcall ProcessFile (HANDLE hArcData, int Operation, char *DestPath, char *DestName);
DLLIMPORT int __stdcall CloseArchive (HANDLE hArcData);
DLLIMPORT void __stdcall SetChangeVolProc (HANDLE hArcData, tChangeVolProc pChangeVolProc1);
DLLIMPORT void __stdcall SetProcessDataProc (HANDLE hArcData, tProcessDataProc pProcessDataProc);
DLLIMPORT int __stdcall GetPackerCaps();
DLLIMPORT BOOL __stdcall CanYouHandleThisFile (char *FileName);

#endif /* _ENSONIQUNPACKEREFE_H_ */
