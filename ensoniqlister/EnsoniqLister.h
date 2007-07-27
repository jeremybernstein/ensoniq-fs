//----------------------------------------------------------------------------
// EnsoniqLister plugin for TotalCommander
// Plugin for visualizing and hearing Ensoniq instruments (*.efe)
//
// MAIN H FILE
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
#ifndef _ENSONIQLISTER_H_
#define _ENSONIQLISTER_H_

//----------------------------------------------------------------------------
// constants
//----------------------------------------------------------------------------
#define VERSION_STRING "0.1"
#define DLLEXPORT __declspec (dllexport)

//----------------------------------------------------------------------------
// struct for file info
//----------------------------------------------------------------------------
typedef struct _FILE_LIST
{
	struct _FILE_LIST *pNext;
} FILE_LIST;

//----------------------------------------------------------------------------
// structs for open handles
//----------------------------------------------------------------------------
typedef struct _HANDLE_LIST
{
} HANDLE_LIST;

//----------------------------------------------------------------------------
// DLL exports
//----------------------------------------------------------------------------
DLLEXPORT HWND __stdcall ListLoad(HWND ParentWin, char* FileToLoad,
	int ShowFlags);
DLLEXPORT void __stdcall ListCloseWindow(HWND ListWin);
DLLEXPORT void __stdcall ListGetDetectString(char* DetectString, int maxlen);
DLLEXPORT int __stdcall ListSearchText(HWND ListWin, char* SearchString, 
	int SearchParameter);
DLLEXPORT int __stdcall ListSendCommand(HWND ListWin, int Command,
	int Parameter);
DLLEXPORT int __stdcall ListPrint(HWND ListWin, char* FileToPrint, 
	char* DefPrinter, int PrintFlags, RECT* Margins);
DLLEXPORT int __stdcall ListNotificationReceived(HWND ListWin, 
	int Message, WPARAM wParam, LPARAM lParam);
DLLEXPORT void __stdcall ListSetDefaultParams(ListDefaultParamStruct* dps);
DLLEXPORT HBITMAP __stdcall ListGetPreviewBitmap(char* FileToLoad,
	int width, int height, char* contentbuf,int contentbuflen);

#endif /* _ENSONILISTER_H_ */
