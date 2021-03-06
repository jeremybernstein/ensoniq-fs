//----------------------------------------------------------------------------
// EnsoniqFS plugin for TotalCommander
//
// DISK structure header file
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
#ifndef _DISKSTRUCTURE_H_
#define _DISKSTRUCTURE_H_


#define TYPE_DISK	0
#define TYPE_CDROM	1
#define TYPE_FILE	2
#define TYPE_FLOPPY	3

#define IMAGE_FILE_UNKNOWN	0
#define IMAGE_FILE_ISO		1
#define IMAGE_FILE_MODE1	2
#define IMAGE_FILE_GKH		3
#define IMAGE_FILE_GIEBLER	4

//----------------------------------------------------------------------------
// disk descriptor
//----------------------------------------------------------------------------
typedef struct _DISK
{
	char cMsDosName[260];	// device name starting with "\\.\" or image file
	char cLongName[260];	// long device name or image file
	char cDiskLabel[8];		// Ensoniq disk label
	char cLegalDiskLabel[8];// DOS disk label made out of cDiskLabel
	DWORD dwBlocks;			// number of logical blocks on this disk
	DWORD dwBlocksFree;		// number of free blocks on this disk
	DWORD dwPhysicalBlocks; // number of physical blocks on this disk
	HANDLE hHandle;			// handle for direct access
	int iType;				// TYPE_DISK | TYPE_CDROM | TYPE_FILE | TYPE_FLOPPY
	int iImageType;			// subtype if disk is an image file
	DWORD dwDataOffset;		// for image files: offset of data in image file
	unsigned char *ucGieblerMap;	// for Giebler images only
	DWORD dwGieblerMapOffset;
	unsigned char *ucCache;	// pointer to cache memory
	DWORD *dwCacheTable;	// which blocks are in cache?
	DWORD *dwCacheAge;		// the age of each cache entry
	unsigned char *ucCacheFlags;	// flags (dirty flag)
	DWORD dwCacheHits;
	DWORD dwCacheMisses;
	DWORD dwFATCacheBlock;	// number of block in FAT cache
	DWORD dwFATMiss, dwFATHit;
	DWORD dwLastFreeFATEntry;	// marks the last free FAT entry to speedup
								// the search for the next free one
	unsigned char ucFATCache[512];	// FAT cache (one block)
	DWORD dwReadCounter;	// counter of read accesses (for cache age)
	DISK_GEOMETRY_EX DiskGeometry;
	int iIsEnsoniq;			// flag for filesystem type
	struct _DISK *pNext;	// pointer to next disk descriptor
} DISK;

#endif
