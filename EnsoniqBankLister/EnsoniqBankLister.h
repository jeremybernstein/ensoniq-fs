// EnsoniqBankLister.h : Haupt-Header-Datei f�r die Anwendung ENSONIQBANKLISTER
//

#if !defined(AFX_ENSONIQBANKLISTER_H__9A992523_0D25_400E_A34B_510F6D77D317__INCLUDED_)
#define AFX_ENSONIQBANKLISTER_H__9A992523_0D25_400E_A34B_510F6D77D317__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CEnsoniqBankListerApp:
// Siehe EnsoniqBankLister.cpp f�r die Implementierung dieser Klasse
//

class CEnsoniqBankListerApp : public CWinApp
{
public:
	CEnsoniqBankListerApp();

// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CEnsoniqBankListerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung

	//{{AFX_MSG(CEnsoniqBankListerApp)
		// HINWEIS - An dieser Stelle werden Member-Funktionen vom Klassen-Assistenten eingef�gt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VER�NDERN!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_ENSONIQBANKLISTER_H__9A992523_0D25_400E_A34B_510F6D77D317__INCLUDED_)
