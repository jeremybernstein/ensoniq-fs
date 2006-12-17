// ETools.h : Haupt-Header-Datei für die Anwendung ETOOLS
//

#if !defined(AFX_ETOOLS_H__A001983C_6577_4E59_AC37_FEA26C8F8540__INCLUDED_)
#define AFX_ETOOLS_H__A001983C_6577_4E59_AC37_FEA26C8F8540__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CEToolsApp:
// Siehe ETools.cpp für die Implementierung dieser Klasse
//

class CEToolsApp : public CWinApp
{
public:
	CEToolsApp();

// Überladungen
	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CEToolsApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CEToolsApp)
		// HINWEIS - An dieser Stelle werden Member-Funktionen vom Klassen-Assistenten eingefügt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VERÄNDERN!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_ETOOLS_H__A001983C_6577_4E59_AC37_FEA26C8F8540__INCLUDED_)
