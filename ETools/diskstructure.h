#ifndef _DISKSTRUCTURE_ETOOLS_H_
#define _DISKSTRUCTURE_ETOOLS_H_

#include <winioctl.h>

// some definitions which are missing for some reason in Visual Studio 6
// (maybe I'll need another service pack :))
#ifdef _MSC_VER
	#define INVALID_SET_FILE_POINTER	((DWORD)-1)
	#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX CTL_CODE(IOCTL_DISK_BASE,0x28,\
											 METHOD_BUFFERED,FILE_ANY_ACCESS)
	typedef struct _DISK_GEOMETRY_EX {
	  DISK_GEOMETRY Geometry;
	  LARGE_INTEGER DiskSize;
	  BYTE Data[1];
	} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;
#endif

#include "../EnsoniqFS/diskstructure.h"

#endif

