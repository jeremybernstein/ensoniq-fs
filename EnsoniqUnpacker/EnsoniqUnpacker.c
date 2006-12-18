//----------------------------------------------------------------------------
// EnsoniqUnpacker plugin v1.23 for TotalCommander
//----------------------------------------------------------------------------
// (c) 2006 thoralt@thoralt.de
// 
// Plugin for reading various Ensoniq formatted disk image files
//----------------------------------------------------------------------------
#define VERSION_STRING "1.23"

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
#include "EnsoniqUnpacker.h"

// If you want to have logging built in, uncomment the line below
//#define LOGGING 1
#define LOGFILE	"C:\\EnsoniqUnpacker-LOG.txt"

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
// ReadBlock
//
// Reads one block (512 bytes) from opened image file
//
// -> ucBuf = Pointer to target buffer (512 bytes)
//    iBlock = which block
//    pHandle = pointer to valid handle structure
// <- 0: OK
//    1: error
//----------------------------------------------------------------------------
int ReadBlock(unsigned char *ucBuf, int iBlock, HANDLE_LIST *pHandle)
{
	int iOffset = -1, i, iCount;
	
	// error checking
	if(!pHandle) return 1;
	if(!pHandle->pFile) return 1;
	
	// position file pointer to beginning of block depending on file format
	if(FILETYPE_GIEBLER==pHandle->iFileType)
	{
		iCount = 0;
		for(i=0; i<pHandle->iGieblerBitmapLen; i++)
		{
			// get the corresponding bit from the Giebler bitmap table
			// if the bit == 0, then this block is included in the file
		    if(((pHandle->ucGieblerBitmap[i>>3] >> (7-(i&0x07))) & 0x01)==0)
			{
			    // block found?
			    if(i==iBlock)
			    {
					iOffset = iCount*512 + pHandle->iImageOffset;
					break;
				}

				iCount++;
			}
		}
		
		// block not found?
		if(iOffset==-1)
		{
			memset(ucBuf, 0, 512);
			return 0;
		}
	}
	else if(FILETYPE_MODE1CD==pHandle->iFileType)	// treat Mode1 CDROM extra
	{
		iOffset = (iBlock>>2)*2352 + 16 + ((iBlock&0x03)*512);
	}
	else
	{
		iOffset = pHandle->iImageOffset + iBlock*512;
	}

	if(fseek(pHandle->pFile, iOffset, SEEK_SET)) return 1;
	
	// read one block
	if(512!=fread(ucBuf, 1, 512, pHandle->pFile)) return 1;
	
	return 0;
}

/*
//----------------------------------------------------------------------------
// ExpandGieblerImage
//
// Expands, if necessary, a Giebler formatted image to its maximum size
//
// -> pHandle = pointer to valid handle structure
// <- 0: OK
//    1: error
//----------------------------------------------------------------------------
int ExpandGieblerImage(HANDLE_LIST *pHandle)
{
	int i, iGieblerBitmapLen, iGieblerMapOffset;
	char cTempPath[260], cTempName[260];
	unsigned char ucBuf[512];
	FILE *f;
	
	if(!pHandle) return 1;
	if(!pHandle->pFile) return 1;
	
	// is the file already expanded?
	if(pHandle->ucGieblerIsExpanded) return 1;
	
	// check whether the file needs to be expanded
	for(i=0; i<(pHandle->iGieblerBitmapLen/8); i++)
	{
		if(pHandle->ucGieblerBitmap[i]) break;
	}
	
	// all bytes in bitmap = "0" -> no blocks are left out
	if(i==(pHandle->iGieblerBitmapLen/8)) return 0;

	LOG("ExpandGieblerImage(): File has to be expanded.\n");
	LOG("pHandle->cArcName=\""); LOG(pHandle->cArcName); LOG("\"\n");
	
	// create temporary file
	GetTempPath(260, cTempPath);
	GetTempFileName(cTempPath, "ENS", 0, cTempName);
	LOG("Created temp file \""); LOG(cTempName); LOG("\".\n");
	
	f = fopen(cTempName, "wb");
	if(NULL==f)
	{
		LOG("Unable to open temp file for writing.\n");
		return 1;
	}
	
	// copy header
	fseek(pHandle->pFile, 0, SEEK_SET);
	if(512!=fread(ucBuf, 1, 512, pHandle->pFile))
	{
		LOG("Unable to read header from source file.\n");
		fclose(f);
		return 1;
	}
	switch(ucBuf[0x1FF])
	{
		case 0x03:    // EDE DD
		case 0x07:    // EDT DD
			iGieblerMapOffset = 0xA0;
			iGieblerBitmapLen = 1600;
			break;
		
		case 0xCB:    // EDA HD
		case 0xCC:    // EDT HD
			iGieblerMapOffset = 0x60;
			iGieblerBitmapLen = 3200;
			break;
	}
	
	// mark all sectors in header as available
	for(i=0; i<(iGieblerBitmapLen/8); i++) ucBuf[iGieblerMapOffset+i] = 0;
	
	if(512!=fwrite(ucBuf, 1, 512, f))
	{
		LOG("Unable to write header to temp file.\n");
		fclose(f);
		return 1;
	}
	
	// copy all blocks
	for(i=0; i<(pHandle->iGieblerBitmapLen); i++)
	{
		if(ReadBlock(ucBuf, i, pHandle))
		{
			LOG("Unable to read block "); LOG_INT(i); 
			LOG(" from source file.\n");
			fclose(f);
			return 1;
		}
		
		if(512!=fwrite(ucBuf, 1, 512, f))
		{
			LOG("Unable to write block "); LOG_INT(i); 
			LOG(" to temp file.\n");
			fclose(f);
			return 1;
		}
	}
	fclose(f);

	LOG("Temp file written OK.\n");
	
	// close original file
	fclose(pHandle->pFile);
	
	// overwrite source file with temp file
	LOG("Moving \""); LOG(cTempName); LOG("\" to \"");
	LOG(pHandle->cArcName); LOG("\"...\n");
	if(0==MoveFileEx(cTempName, pHandle->cArcName, 
		MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED))
	{
		LOG("Unable to move temp file to source file.\n");
		
		// restore open handle
		pHandle->pFile = fopen(pHandle->cArcName, "rb");
		return 1;
	}
	
	// open newly written file
	pHandle->pFile = fopen(pHandle->cArcName, "rb");
	if(NULL==pHandle->pFile)
	{
		LOG("Unable to re-open file.\n");
		return 1;
	}
	
	// mark all sectors as available
	for(i=0; i<(pHandle->iGieblerBitmapLen/8); i++)
		pHandle->ucGieblerBitmap[i] = 0;

	return 0;
}
*/

//----------------------------------------------------------------------------
// GetFATEntry
//
// Reads a the FAT entry for a given block
// recursively
//
// -> iBlock = block to look for
//    pHandle = pointer to valid handle structure
// <- -1: error during FAT read
//    -2: invalid requested block
//     0: empty FAT entry
//     1: end of file FAT entry
//     2: bad sector FAT entry
//     every other value: pointer to next block of file
//----------------------------------------------------------------------------
int GetFATEntry(int iBlock, HANDLE_LIST *pHandle)
{
	int i;
	DWORD dwBlock;
	
	// error checking
	if((iBlock<0)||(iBlock>=pHandle->iNumBlocks)) return -2;

	dwBlock = iBlock/170 + 5;
	if(dwBlock!=pHandle->dwFATBufBlock)
	{
		// read FAT block containing the requested FAT entry
		if(ReadBlock(pHandle->ucFATBuf, dwBlock, pHandle)) return -1;
	}
	
	// get entry from FAT block
	i = (pHandle->ucFATBuf[(iBlock%170)*3+0]<<16)
	  + (pHandle->ucFATBuf[(iBlock%170)*3+1]<<8)
	  + (pHandle->ucFATBuf[(iBlock%170)*3+2]);

	return i;
}

//----------------------------------------------------------------------------
// ReadDirectory
//
// Reads a directory, adds the contents to name list, follows subdirectories
// recursively
//
// -> iBlock = starting block for current directory
//    cPath = prefix of this directory
//    pHandle = pointer to valid handle structure
// <- 0: OK
//    1: error
//----------------------------------------------------------------------------
int ReadDirectory(int iBlock, char *cPath, char *cOriginalPath,
				  HANDLE_LIST *pHandle)
{
	char cFN[17], cOriginalFN[13], cSubPath[260], cOriginalSubPath[260],
		cText[128];
	int i, j, iLen, iStart, iContiguous;
	unsigned char ucBuf[1024], ucType;
	FILE_LIST *pListEntry;
	FILE *f;

	LOG("ReadDirectory("); LOG_INT(iBlock); LOG(", \""); LOG(cPath);
	LOG(", "); LOG_HEX8((int)pHandle); LOG("\n");
	
	// read directory (2 blocks)
	LOG("Reading directory blocks: ");
	if(ReadBlock(ucBuf, iBlock, pHandle))
	{
		LOG("failed.\n");
		return 1;
	}
	if(ReadBlock(ucBuf+512, iBlock+1, pHandle))
	{
		LOG("failed.\n");
		return 1;
	}
	LOG("OK.\n");
	
	// loop through all entries
	for(i=0; i<39; i++)
	{
		ucType = ucBuf[i*26+1];
		if(0x00==ucType) continue;	// ignore blank entry
		if(0x08==ucType) continue;	// ignore link to parent directory

		// read filename		
		memset(cFN, 0, 17);
		memset(cOriginalFN, 0, 13);
		strncpy(cFN, ucBuf+i*26+2, 12);
		strncpy(cOriginalFN, ucBuf+i*26+2, 12);
		
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
			
			// fill up original file name with spaces if necessary
			if(cOriginalFN[j]<32) cOriginalFN[j] = 32;
		}
		
		// calculate file properties
		iLen        = (ucBuf[i*26 + 14]<<8)  +  ucBuf[i*26 + 15];
		iContiguous = (ucBuf[i*26 + 16]<<8)  +  ucBuf[i*26 + 17];
		iStart      = (ucBuf[i*26 + 18]<<24) + (ucBuf[i*26 + 19]<<16)
				    + (ucBuf[i*26 + 20]<<8)  +  ucBuf[i*26 + 21];

		LOG("Found entry: \""); LOG(cFN);
		LOG("\", type "); LOG_HEX2(ucType);
		LOG(", start "); LOG_HEX8(iStart);
		LOG(", len "); LOG_HEX8(iLen); LOG("\n");
		
		// create new file list entry
		pListEntry = malloc(sizeof(FILE_LIST));
		if(NULL==pListEntry) return 1;
		memset(pListEntry, 0, sizeof(FILE_LIST));
		
		// copy path and name
		strcpy(pListEntry->HeaderData.FileName, cPath);
		strcat(pListEntry->HeaderData.FileName, cFN);
		sprintf(cText, ".[%02i].efe", ucType);
		strcat(pListEntry->HeaderData.FileName, cText);	// add type
		strncpy(pListEntry->cOriginalName, cOriginalFN, 12);
		strcpy(pListEntry->cOriginalPath, cOriginalPath);
		pListEntry->cOriginalName[12] = 0;
		
		// copy file data
		pListEntry->iLen = iLen;
		pListEntry->iStart = iStart;
		pListEntry->iContiguous = iContiguous;
		pListEntry->ucType = ucType;
		
		pListEntry->HeaderData.FileAttr = 0x20;
		pListEntry->HeaderData.PackSize = iLen * 512;

		// add extra EFE header
		pListEntry->HeaderData.UnpSize = iLen * 512 + 512;
		
		// is this the first entry?
		if(NULL==pHandle->pFileList)
		{
			// set this file as root of the file list
			pHandle->pFileList = pListEntry;
			
			// set this file to be the first file for ReadHeader
			pHandle->pCurrentFile = (FILE_LIST*)0xFFFFFFFF;
			
			pHandle->cInfofile = INFOFILE_SUMMARY;
		}
		else
		{
			pHandle->pLastFile->pNext = pListEntry;
		}
		
		pHandle->pLastFile = pListEntry;

		if(0x02==ucType)	// subdirectory
		{
			LOG("----recursive read----\n");
			
			// construct new path
			strcpy(cSubPath, cPath);
			strcat(cSubPath, cFN);
			strcat(cSubPath, "\\");
			strcpy(cOriginalSubPath, cOriginalPath);
			strcat(cOriginalSubPath, cOriginalFN);

			// trim right spaces
			for(j=strlen(cOriginalSubPath); j>=0; j--)
			{
				if(0==cOriginalSubPath[j]) continue;
				else if(32==cOriginalSubPath[j])
				{
					cOriginalSubPath[j] = 0;
					continue;
				}
				else break;
			}
			strcat(cOriginalSubPath, "\\");

			// recursive call
			if(ReadDirectory(iStart, cSubPath, cOriginalSubPath, pHandle))
				return 1;
		}
	}
	
	return 0;
}

//----------------------------------------------------------------------------
// DetectGKH
//
// Try to detect the open file as GKH formatted image
//
// -> ucBuf: pointer to first 512 bytes of file
// <-  1: GKH detected
//     0: GKH not detected
//----------------------------------------------------------------------------
int DetectGKH(unsigned char *ucBuf)
{
	// check tag
	if(0==strncmp(ucBuf, "TDDFI", 5))
	{
		LOG("File identified as GKH.\n");
		return 1;
	}
	
	return 0;
}

//----------------------------------------------------------------------------
// DetectMode1CD
//
// Try to detect the open file as Mode1CD formatted image
//
// -> ucBuf: pointer to first 512 bytes of file
// <-  1: Mode1CD detected
//     0: Mode1CD not detected
//----------------------------------------------------------------------------
int DetectMode1CD(unsigned char *ucBuf)
{
	// check tag
	if((0x00==ucBuf[ 0])&&
	   (0xFF==ucBuf[ 1])&&
	   (0xFF==ucBuf[ 2])&&
	   (0xFF==ucBuf[ 3])&&
	   (0xFF==ucBuf[ 4])&&
	   (0xFF==ucBuf[ 5])&&
	   (0xFF==ucBuf[ 6])&&
	   (0xFF==ucBuf[ 7])&&
	   (0xFF==ucBuf[ 8])&&
	   (0xFF==ucBuf[ 9])&&
	   (0xFF==ucBuf[10])&&
	   (0x00==ucBuf[11]))
	{
		LOG("File identified as Mode1CD.\n");
		return 1;
	}
	
	return 0;
}

//----------------------------------------------------------------------------
// DetectGiebler
//
// Try to detect the open file as Giebler formatted image (EDE, EDA, EDT...)
//
// -> ucBuf: pointer to first 512 bytes of file
// <-  1: Giebler detected
//     0: Giebler not detected
//----------------------------------------------------------------------------
int DetectGiebler(unsigned char *ucBuf)
{
	int iMapOffset = 0;
	

	if((ucBuf[0x00]!=0x0D) || (ucBuf[0x01]!=0x0A) ||
	   (ucBuf[0x4E]!=0x0D) || (ucBuf[0x4F]!=0x0A)) return 0;

	switch(ucBuf[0x1FF])
	{
		case 0x03:    // EDE DD
		case 0x07:    // EDT DD
			iMapOffset = 0xA0;
		break;
		
		case 0xCB:    // EDA HD
		case 0xCC:    // EDT HD
			iMapOffset = 0x60;
		break;
		
		default:
			return 0;
	}

	if((ucBuf[iMapOffset-3]!=0x0D) ||
	   (ucBuf[iMapOffset-2]!=0x0A) ||
	   (ucBuf[iMapOffset-1]!=0x1A)) return 0;
	
	LOG("File identified as Giebler image.\n");
	return 1;
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
DLLIMPORT HANDLE __stdcall OpenArchive (tOpenArchiveData *ArchiveData)
{
	HANDLE_LIST *pHandle, *pTemp;
	unsigned char ucBuf[512], ucDetectOnly = 0;
	int iFileType = FILETYPE_UNKNOWN, iImageOffset = 0, i, iNumTags,
		iGKHFormatTagFound = 0, iGKHImageTypeTagFound = 0,
		iGKHSubjectOffset = 0, iGKHSubjectLen = 0, iGKHAuthorOffset = 0,
		iGKHAuthorLen = 0, iGieblerMapOffset = 0, iGieblerBitmapLen = 0;
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

	// detect GKH
	if(1==DetectGKH(ucBuf))
	{
		iFileType = FILETYPE_GKH;
		fseek(pFile, 8, SEEK_SET);
		
		// loop throug all available tags
		iNumTags = (ucBuf[0x06]) | (ucBuf[0x07]<<8);
		for(i=0; i<iNumTags; i++)
		{
			// read one tag
			if(10!=fread(ucBuf, 1, 10, pFile))
			{
				LOG("fread(10) (reading one tag) failed.\n");
				ArchiveData->OpenResult = E_EREAD;
				fclose(pFile);
				return 0;	// Error
			}

			// format tag?			
			if((0x01 == ucBuf[0])&&(0x04 == ucBuf[1])&&(0x01 == ucBuf[2])&&
			   (0x00 == ucBuf[3])&&(0x00 == ucBuf[4])&&(0x00 == ucBuf[5])&&
			   (0x01 == ucBuf[6])&&(0x00 == ucBuf[7])&&(0x00 == ucBuf[8])&&
			   (0x00 == ucBuf[9]))
			{
				iGKHFormatTagFound = 1;
			}
			
			// image type tag?
			if((0x0A == ucBuf[0])&&(0x05 == ucBuf[1])&&(0x50 == ucBuf[2])&&
			   (0x00 == ucBuf[3])&&(0x02 == ucBuf[4])&&(0x00 == ucBuf[5]))
			{
				iGKHImageTypeTagFound = 1;
			}
	
			// image location tag?
			if((0x0B == ucBuf[0])&&(0x0B == ucBuf[1]))
			{
				iImageOffset = (ucBuf[6])|(ucBuf[7]<<8)|
							   (ucBuf[8]<<16)|(ucBuf[9]<<24);
			}
			
			// author tag?
			if(0x14 == ucBuf[0])
			{
				iGKHAuthorLen    = (ucBuf[2])|(ucBuf[3]<<8)|
							   	   (ucBuf[4]<<16)|(ucBuf[5]<<24);
				iGKHAuthorOffset = (ucBuf[6])|(ucBuf[7]<<8)|
							 	   (ucBuf[8]<<16)|(ucBuf[9]<<24);
			}
			
			// subject tag?
			if(0x15 == ucBuf[0])
			{
				iGKHSubjectLen    = (ucBuf[2])|(ucBuf[3]<<8)|
							   	    (ucBuf[4]<<16)|(ucBuf[5]<<24);
				iGKHSubjectOffset = (ucBuf[6])|(ucBuf[7]<<8)|
							 	    (ucBuf[8]<<16)|(ucBuf[9]<<24);
			}
		}

		// check for reasonable values
		// (ImageTypeTag and FormatTag must be there)	
		if((0==iGKHImageTypeTagFound)||(0==iGKHFormatTagFound))
		{
			LOG("Missing ImageTypeTag or FormatTag. Aborting.\n");
			ArchiveData->OpenResult = E_UNKNOWN_FORMAT;
			fclose(pFile);
			return 0;	// Error
		}
	}
	
	// try to detect Mode1 CDROM
	else if(1==DetectMode1CD(ucBuf))
	{
		iFileType = FILETYPE_MODE1CD;
		iImageOffset = 0;
	}

	// try to detect Giebler format
	else if(1==DetectGiebler(ucBuf))
	{
		iFileType = FILETYPE_GIEBLER;
		iImageOffset = 512;

		// copy Giebler bitmap			
		switch(ucBuf[0x1FF])
		{
			case 0x03:    // EDE DD
			case 0x07:    // EDT DD
				iGieblerMapOffset = 0xA0;
				iGieblerBitmapLen = 1600;
				break;
			
			case 0xCB:    // EDA HD
			case 0xCC:    // EDT HD
				iGieblerMapOffset = 0x60;
				iGieblerBitmapLen = 3200;
				break;
		}
	}
	
	// handle as plain image file
	else
	{
		// must be plain file
		iFileType = FILETYPE_PLAIN;
		LOG("Trying plain image format.\n");
		iImageOffset = 0;
	}
	
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
	
	// fill in values
	pHandle->dwFATBufBlock = 0xFFFFFFFF;
	pHandle->pFile = pFile;
	pHandle->iFileType = iFileType;
	pHandle->iImageOffset = iImageOffset;
	pHandle->iGKHSubjectOffset = iGKHSubjectOffset;
	pHandle->iGKHSubjectLen = iGKHSubjectLen;
	pHandle->iGKHAuthorOffset = iGKHAuthorOffset;
	pHandle->iGKHAuthorLen = iGKHAuthorLen;
	pHandle->iGieblerBitmapLen = iGieblerBitmapLen;
	strcpy(pHandle->cArcName, ArchiveData->ArcName);
	
	// create Giebler bitmap if necessary
	if(FILETYPE_GIEBLER==iFileType)
	{
		pHandle->ucGieblerBitmap = malloc(iGieblerBitmapLen/8);
		if(NULL==pHandle->ucGieblerBitmap)
		{
			ArchiveData->OpenResult = E_NO_MEMORY;
			LOG("Unable to allocate ucGieblerBitmap.\n");
			fclose(pFile);
			return 0;	// Error
		}
		memcpy(pHandle->ucGieblerBitmap,
			   ucBuf + iGieblerMapOffset,
			   iGieblerBitmapLen/8);
	}
	
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

	// first check volume ID
	LOG("Checking volume ID: ");
	if(ReadBlock(ucBuf, 1, pHandle))
	{
		LOG("read error.\n");
		ArchiveData->OpenResult = E_EREAD;
		fclose(pFile);
		return 0;	// Error
	}
	if(('I'!=ucBuf[38])||('D'!=ucBuf[39]))
	{
		LOG("failed.\n");
		ArchiveData->OpenResult = E_BAD_DATA;
		fclose(pFile);
		return 0;	// Error
	}
	LOG("OK.\n");

	// get disk info
	for(i=0; i<7; i++) pHandle->cDisklabel[i] = ucBuf[i+31];
	pHandle->cDisklabel[7] = 0;
	pHandle->iSectorsPerTrack = (ucBuf[4]<<8) + ucBuf[5];
	pHandle->iHeads           = (ucBuf[6]<<8) + ucBuf[7];
	pHandle->iTracks          = (ucBuf[8]<<8) + ucBuf[9];
	pHandle->iBytesPerBlock   = (ucBuf[10]<<24) + (ucBuf[11]<<8) 
							  + (ucBuf[12]<<8) + ucBuf[13];
	pHandle->iNumBlocks       = (ucBuf[14]<<24) + (ucBuf[15]<<16) 
							  + (ucBuf[16]<<8) + ucBuf[17];
	
	// read files only if being called by TotalCommander directly
	if(!ucDetectOnly)
	{
		// read main directory recursively and add contents to name list
		LOG("Reading files...\n");
		if(ReadDirectory(3, "", "", pHandle))
		{
			LOG("failed.\n");
			ArchiveData->OpenResult = E_EREAD;
			fclose(pFile);
			return 0;	// Error
		}
		LOG("OK.\n");
	}
	
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
	
	LOG("ReadHeader()\n");
	
	GetArcNameWithoutPath(pHandle, cArcName);
	
	// is this the first call to ReadHeader?
	if((FILE_LIST*)0xFFFFFFFF==pHandle->pCurrentFile)
	{
		if(INFOFILE_SUMMARY==pHandle->cInfofile)
		{
			// display summary infofile first
			strcpy(HeaderData->FileName, cArcName);
			strcat(HeaderData->FileName, "-summary.txt");
			HeaderData->FileAttr = 0x20;
			HeaderData->PackSize = 333;
			HeaderData->UnpSize = 333;
			pHandle->cInfofile = INFOFILE_COMPLETE;
			pHandle->cInfofileExtract = INFOFILE_SUMMARY;
			return 0; // OK
		}
		else if(INFOFILE_COMPLETE==pHandle->cInfofile)
		{
			// display complete infofile second
			strcpy(HeaderData->FileName, cArcName);
			strcat(HeaderData->FileName, "-treeview.txt");
			HeaderData->FileAttr = 0x20;
			HeaderData->PackSize = 333;
			HeaderData->UnpSize = 333;
			pHandle->cInfofile = INFOFILE_NONE;
			pHandle->cInfofileExtract = INFOFILE_COMPLETE;

			return 0; // OK
		}
		else
		{
			// display first file
			pHandle->pCurrentFile = pHandle->pFileList;
			pHandle->cInfofileExtract = INFOFILE_NONE;
		}
	}
	else
	{
		pHandle->pCurrentFile = pHandle->pCurrentFile->pNext;
	}
	
	// skip directory names
	while(pHandle->pCurrentFile)
	{
		if(0x02!=pHandle->pCurrentFile->ucType) break;
		pHandle->pCurrentFile = pHandle->pCurrentFile->pNext;
	}

	// did we reach the end?
	if(NULL==pHandle->pCurrentFile) return E_END_ARCHIVE;

	// copy data	
	memcpy(HeaderData, &pHandle->pCurrentFile->HeaderData,
		sizeof(tHeaderData));
	
	return 0; // OK
}

int GetPathLevel(char *cPath)
{
	int i, iPathLevel = 0;

	// count backslashes
	for(i=0; i<strlen(cPath); i++) if(cPath[i]=='\\') iPathLevel++;
	
	return iPathLevel;
}

void GetDetailedEnsoniqFiletype(unsigned char ucType, char *cType)
{
	switch(ucType)
	{
/*
// 00       01          02           03            04          05
"empty", "EPS O.S.", "DIRECTORY", "INSTRUMENT", "EPS BANK", "EPS SEQUENCE",
// 06          07           08           09           10          11
"EPS SONG", "EPS SYSEX", "DIRECTORY", "EPS MACRO", "VFX1PROG", "VFX6PROG",
// 12           13           14            15
"VFX30PROG", "VFX60PROG", "VFX1PRESET", "VFX10PRESET",
// 16             17          18           19           20          21
"VFX20PRESET", "VFX1SONG", "VFX30SONG", "VFX60SONG", "VFX SYSEX", "VFXSETUP",
// 22   23             24
"VFX O.S.", "EPS16+ BANK", "EPS16+ EFFECT",
// 25                 26             27             28              29
"EPS16+ SEQUENCE", "EPS16+ SONG", "EPS16+ O.S.", "ASR SEQUENCE", "ASR SONG",
// 30          31           32          33            34   35   36   37   38
"ASR BANK", "ASR TRACK", "ASR O.S.", "ASR EFFECT", "?", "?", "?", "?", "?",
// 39
"?"
*/		case  0:
			strcpy(cType, "empty"); break;
		case  1:
			strcpy(cType, "EPS O.S."); break;
		case  2:
			strcpy(cType, "DIRECTORY"); break;
		case  3:
			strcpy(cType, "INSTRUMENT"); break;
		case  4:
			strcpy(cType, "EPS BANK"); break;
		case  5:
			strcpy(cType, "EPS SEQUENCE"); break;
		case  6:
			strcpy(cType, "EPS SONG"); break;
		case  7:
			strcpy(cType, "EPS SYSEX"); break;
		case  8:
			strcpy(cType, "DIRECTORY"); break;
		case  9:
			strcpy(cType, "EPS MACRO"); break;
		case 10:
			strcpy(cType, "VFX1PROG"); break;
		case 11:
			strcpy(cType, "VFX6PROG"); break;
		case 12:
			strcpy(cType, "VFX30PROG"); break;
		case 13:
			strcpy(cType, "VFX60PROG"); break;
		case 14:
			strcpy(cType, "VFX1PRESET"); break;
		case 15:
			strcpy(cType, "VFX10PRESET"); break;
		case 16:
			strcpy(cType, "VFX20PRESET"); break;
		case 17:
			strcpy(cType, "VFX1SONG"); break;
		case 18:
			strcpy(cType, "VFX30SONG"); break;
		case 19:
			strcpy(cType, "VFX60SONG"); break;
		case 20:
			strcpy(cType, "VFX SYSEX"); break;
		case 21:
			strcpy(cType, "VFX SETUP"); break;
		case 22:
			strcpy(cType, "VFX O.S."); break;
		case 23:
			strcpy(cType, "EPS16+ BANK"); break;
		case 24:
			strcpy(cType, "EPS16+ EFFECT"); break;
		case 25:
			strcpy(cType, "EPS16+ SEQUENCE"); break;
		case 26:
			strcpy(cType, "EPS16+ SONG"); break;
		case 27:
			strcpy(cType, "EPS16+ O.S."); break;
		case 28:
			strcpy(cType, "ASR SEQUENCE"); break;
		case 29:
			strcpy(cType, "ASR SONG"); break;
		case 30:
			strcpy(cType, "ASR BANK"); break;
		case 31:
			strcpy(cType, "ASR TRACK"); break;
		case 32:
			strcpy(cType, "ASR O.S."); break;
		case 33:
			strcpy(cType, "ASR EFFECT"); break;
		case 34:
			strcpy(cType, "ASR MACRO"); break;

		default:
			sprintf(cType, "unknown type (%i)", ucType);
			break;
	}
}

void GetShortEnsoniqFiletype(unsigned char ucType, char *cType)
{
	switch(ucType)
	{
		case 0x03:
			strcpy(cType, "Instrument   ");
			break;

		case 0x04:
		case 0x17:
		case 0x1E:
			strcpy(cType, "Bank         ");
			break;

		case 0x18:
		case 0x21:
			strcpy(cType, "Effect       ");
			break;

		case 0x06:
		case 0x1A:
		case 0x1D:
			strcpy(cType, "Song/Seq     ");
			break;

		case 0x05:
		case 0x19:
		case 0x1C:
		case 0x1F:
			strcpy(cType, "Sequence     ");
			break;

		case 0x09:
		case 34:
			strcpy(cType, "Macro        ");
			break;

		case 0x07:
			strcpy(cType, "System Ex.   ");
			break;

		case 0x01:
		case 0x1B:
		case 0x20:
			strcpy(cType, "O.S.         ");
			break;

		default:
			strcpy(cType, "unknown type ");
			break;
	}
}

void PrintFooter(FILE *f)
{
			fprintf(f, "\r\n\r\n"
"   ____                   _        __  __                   __          \r\n"
"  / __/__  ___ ___  ___  (_)__ _  / / / /__  ___  ___ _____/ /_____ ____\r\n"
" / _// _ \\(_-</ _ \\/ _ \\/ / _ `/ / /_/ / _ \\/ _ \\/ _ `/ __/  '_/ -_) __/\r\n"
"/___/_//_/___/\\___/_//_/_/\\_, /  \\____/_//_/ .__/\\_,_/\\__/_/\\_\\\\__/_/   \r\n"
"                           /_/            /_/                           \r\n"
"EnsoniqUnpacker plugin v" VERSION_STRING " for TotalCommander\r\n"
"(c) 2006 thoralt@thoralt.de\r\n");
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
	FILE_LIST *pFile;
	int i, iBlock, iPathLevel, iLastPathLevel=0, j, iFolders, iFiles, iBlocks;
	char cFN[260], cFiletype[512], cArcName[260];
	FILE *f;
	
	LOG("Process file()\n");
	
	// skip or test files
	if(PK_SKIP==Operation) return 0;
	if(PK_TEST==Operation) return 0;
	
	// extract file
	if(PK_EXTRACT!=Operation) return 0;
	
	
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
		GetArcNameWithoutPath(pHandle, cArcName);

		if(INFOFILE_SUMMARY==pHandle->cInfofileExtract)
		{
			LOG("ProcessFile/Extract: infofile-summary open ");
			
			// count folders, files, blocks
			iFiles = 0; iFolders = 1; iBlocks = 0;
			pFile = pHandle->pFileList;
			while(pFile)
			{
				if(0x02==pFile->ucType)
				{
					iFolders++;
				}
				else
				{
					iFiles++;
					iBlocks += pFile->iLen;
				}
				pFile = pFile->pNext;
			}

			f = fopen(cFN, "wb");
			if(NULL==f)
			{
				LOG("failed.\n");
				return E_ECREATE;
			}
			LOG("OK.\n");
	
			fprintf(f, "File name:       \"%s\"\r\n", cArcName);
			fprintf(f, "Disk label:      \"%s\"\r\n", pHandle->cDisklabel);
			fprintf(f, "Disk statistics: %i sectors per track\r\n",
				pHandle->iSectorsPerTrack);
			fprintf(f, "                 %i heads\r\n", pHandle->iHeads);
			fprintf(f, "                 %i tracks\r\n", pHandle->iTracks);
			fprintf(f, "                 %i bytes per block\r\n",
				pHandle->iBytesPerBlock);
			fprintf(f, "                 %i total blocks\r\n",
				pHandle->iNumBlocks);
			fprintf(f, "                 ");
			if(iBlocks==1) fprintf(f, "1 used block\r\n");
			else fprintf(f, "%i used blocks\r\n", iBlocks);
			fprintf(f, "                 ");
			if(iFiles==1) fprintf(f, "1 file\r\n");
			else fprintf(f, "%i files\r\n", iFiles);
			fprintf(f, "                 ");
			if(iFolders==1) fprintf(f, "1 directory\r\n");
			else fprintf(f, "%i directories\r\n", iFolders);
			
			PrintFooter(f);
			fclose(f);
			
			return 0; // OK
		}
		else if(INFOFILE_COMPLETE==pHandle->cInfofileExtract)
		{
			LOG("ProcessFile/Extract: infofile-complete open ");
			
			f = fopen(cFN, "wb");
			if(NULL==f)
			{
				LOG("failed.\n");
				return E_ECREATE;
			}
			LOG("OK.\n");
	
			
			fprintf(f, "[%s (Ensoniq name=\"%s\")]\r\n", cArcName,
				pHandle->cDisklabel);

			pFile = pHandle->pFileList;
			while(pFile)
			{
				// output vertical lines
				iPathLevel = GetPathLevel(pFile->cOriginalPath);
				for(i=0; i<iPathLevel; i++) fprintf(f, "|   ");

				if(0x02==pFile->ucType)
				{
					fprintf(f, "|\r\n");
					for(i=0; i<iPathLevel; i++) fprintf(f, "|   ");
					fprintf(f, "|___[%s] %i ", pFile->cOriginalName,
						pFile->iLen);
					if(1==(pFile->iLen)) fprintf(f, "file\r\n");
					else fprintf(f, "files\r\n");
				}
				else
				{
					GetDetailedEnsoniqFiletype(pFile->ucType, cFiletype);
					fprintf(f, "|___%s ", pFile->cOriginalName);
					if(1==pFile->iLen)
						fprintf(f, "1 block, %s\r\n", cFiletype);
					else
						fprintf(f, "%6i blocks, %s\r\n", pFile->iLen, cFiletype);
				}
				
				pFile = pFile->pNext;
			}

			PrintFooter(f);
			fclose(f);
			
			return 0; // OK
		}

	}

	LOG("ProcessFile/Extract: \""); LOG(cFN); LOG("\" open ");
	f = fopen(cFN, "wb");
	if(NULL==f)
	{
		LOG("failed.\n");
		return E_ECREATE;
	}
	LOG("OK.\n");
	
	LOG("Writing header... ");
	
	memset(ucBuf, 0, 512);
	sprintf(ucBuf, "\r\nEps File:       %s    ", pFile->cOriginalName);
	GetShortEnsoniqFiletype(pFile->ucType, cFiletype);
	strcat(ucBuf, cFiletype);	
	strcat(ucBuf, "\r\n");
	ucBuf[0x31] = 0x1A;
	ucBuf[0x32] = pFile->ucType;
	ucBuf[0x34] = (pFile->iLen>>8)&0xFF;
	ucBuf[0x35] = (pFile->iLen)&0xFF;
	ucBuf[0x36] = (pFile->iContiguous>>8)&0xFF;
	ucBuf[0x37] = (pFile->iContiguous)&0xFF;
	ucBuf[0x38] = (pFile->iStart>>8)&0xFF;
	ucBuf[0x39] = (pFile->iStart)&0xFF;
	if(512!=fwrite(ucBuf, 1, 512, f))
	{
		LOG("Error writing to destination file.\n");
		fclose(f);
		return E_EWRITE;
	}
	
	LOG("Extracting file... ");
	
	// set starting block
	iBlock = pFile->iStart;
	
	// loop through all blocks
	for(i=0; i<pFile->iLen; i++)
	{
		// read next block from disk image
		if(ReadBlock(ucBuf, iBlock, pHandle))
		{
			LOG("Error reading block from image file.\n");
			fclose(f);
			return E_EREAD;
		}
		
		// write next block to file
		if(512!=fwrite(ucBuf, 1, 512, f))
		{
			LOG("Error writing to destination file.\n");
			fclose(f);
			return E_EWRITE;
		}
		
		// get next FAT entry
		iBlock = GetFATEntry(iBlock, pHandle);

		// either end of file reached or error getting FAT entry occured
		if(iBlock<3) break;
	}
	LOG("OK.\n");
	fclose(f);
	
	// notify TotalCmd of progress
	if(0==ProgressCallback(NULL, pFile->HeaderData.UnpSize))
		return E_EABORTED;
	
	return 0;
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

	// close associated file
	if(pHandle->pFile) fclose(pHandle->pFile);
	
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
	
	// remove Giebler bitmap
	if(pHandle->ucGieblerBitmap)
	{
		free(pHandle->ucGieblerBitmap);
	}

	// free allocated memory
	free(pHandle);
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
	LOG("SetChangeVolProc()\n");
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
	LOG("SetProcessDataProc()\n");
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
	LOG("GetPackerCaps()\n");
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
			LOG("DLL_PROCESS_ATTACH\n");
	        break;

		case DLL_PROCESS_DETACH:
			LOG("DLL_PROCESS_DETACH\n");
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
			LOG("DLL_THREAD_ATTACH\n");
    	    break;

      	case DLL_THREAD_DETACH:
			LOG("DLL_THREAD_DETACH\n");
        	break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
