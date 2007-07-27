//----------------------------------------------------------------------------
// EnsoniqLister plugin for TotalCommander
// Plugin for visualizing and hearing Ensoniq instruments (*.efe)
//
// MAIN C FILE
//----------------------------------------------------------------------------
//
// (c) 2007 Thoralt Franz
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

//----------------------------------------------------------------------------
// includes
//----------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "listplug.h"
#include "EnsoniqLister.h"
#include "../ensoniqfs/ini.h"

// If you want to have logging built in, uncomment the line below
#define LOGGING 1
#define LOGFILE	"C:\\EnsoniqLister-LOG.txt"

//----------------------------------------------------------------------------
// globals
//----------------------------------------------------------------------------

// variables to receive initialization parameters from Total Commander
ListDefaultParamStruct g_DefaultParams;

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
// ListLoad
//
// -> ParentWin
//    FileToLoad
//    ShowFlags
// <- window handle if loading succeeded, otherwise NULL
//----------------------------------------------------------------------------
DLLEXPORT HWND __stdcall ListLoad(HWND ParentWin, char* FileToLoad,
	int ShowFlags)
{
	return NULL;
}

//----------------------------------------------------------------------------
// ListCloseWindow
//
// -> ListWin
// <- --
//----------------------------------------------------------------------------
DLLEXPORT void __stdcall ListCloseWindow(HWND ListWin)
{
	return;
}

//----------------------------------------------------------------------------
// ListGetDetectString
//
// -> DetectString
//    maxlen
// <- --
//----------------------------------------------------------------------------
DLLEXPORT void __stdcall ListGetDetectString(char* DetectString, int maxlen)
{
	strncpy(DetectString, "EXT=\"EFE\" & FIND(\"Eps File:\")", maxlen);
}

//----------------------------------------------------------------------------
// FsSetDefaultParams
//
//----------------------------------------------------------------------------
DLLEXPORT void __stdcall ListSetDefaultParams(ListDefaultParamStruct* dps)
{
	char *cName, cValue[3];

	LOG("FsSetDefaultParams(): DefaultIniName=\"");
	LOG(dps->DefaultIniName); LOG("\", PluginInterfaceVersionHi=");
	LOG_INT(dps->PluginInterfaceVersionHi);
	LOG(", PluginInterfaceVersionLow=");
	LOG_INT(dps->PluginInterfaceVersionLow); LOG(", size="); 
	LOG_INT(dps->size); LOG("\n");
	
	// save default param struct
	memcpy(&g_DefaultParams, dps, sizeof(ListDefaultParamStruct));
	cName = g_DefaultParams.DefaultIniName;
	
	// parse INI file
	GetIniValue(cName, "[EnsoniqLister]", "TestOption1", cValue, 2, "1");
//	g_iTestOption1 = (cValue[0]=='0')?0:1;
	
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
