//----------------------------------------------------------------------------
// EnsoniqUnpacker plugin v1.23 for TotalCommander
// Plugin for reading various Ensoniq formatted disk image files
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
#ifndef _ENSONIQUNPACKER_H_
#define _ENSONIQUNPACKER_H_

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

#define INFOFILE_NONE		0
#define INFOFILE_SUMMARY	1
#define INFOFILE_COMPLETE	2

//----------------------------------------------------------------------------
// struct for file info
//----------------------------------------------------------------------------
typedef struct _FILE_LIST
{
	tHeaderData HeaderData;
	struct _FILE_LIST *pNext;
	unsigned char ucType;
	char cOriginalName[13];
	char cOriginalPath[260];
	int iLen, iStart, iContiguous;
} FILE_LIST;

//----------------------------------------------------------------------------
// struct for open handles
//----------------------------------------------------------------------------
typedef struct _HANDLE_LIST
{
	HANDLE hHandle;
	FILE *pFile;
	int iFileType;
	int iImageOffset;
	int iGKHSubjectOffset, iGKHSubjectLen, iGKHAuthorOffset, iGKHAuthorLen;
	struct _HANDLE_LIST *pNext, *pPrevious;
	FILE_LIST *pFileList, *pLastFile, *pCurrentFile;
	char cInfofile, cInfofileExtract;
	int iSectorsPerTrack, iHeads, iTracks, iBytesPerBlock, iNumBlocks,
		iFATLen;
	int iGieblerBitmapLen;
	unsigned char *ucGieblerBitmap, ucGieblerIsExpanded;
	unsigned char ucFATBuf[512];
	DWORD dwFATBufBlock;
	char cDisklabel[9], cArcName[260];
} HANDLE_LIST;

//----------------------------------------------------------------------------
// file type constants
//----------------------------------------------------------------------------
#define FILETYPE_UNKNOWN	0
#define FILETYPE_GKH		1
#define FILETYPE_PLAIN		2
#define FILETYPE_MODE1CD	3
#define FILETYPE_GIEBLER	4

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

#endif /* _ENSONIQUNPACKER_H_ */
