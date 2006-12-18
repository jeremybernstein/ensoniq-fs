//----------------------------------------------------------------------------
// EnsoniqUnpacker plugin for TotalCommander
//----------------------------------------------------------------------------
// (c) 2006 thoralt@thoralt.de
// 
// Plugin for reading various Ensoniq formatted disk image files
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
