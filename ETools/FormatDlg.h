#if !defined(AFX_FORMATDLG_H__63646E29_107D_4EBF_8AA0_80420818C9B4__INCLUDED_)
#define AFX_FORMATDLG_H__63646E29_107D_4EBF_8AA0_80420818C9B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FormatDlg.h : Header-Datei
//

#include "EToolsDlg.h"
#include "diskstructure.h"

#define UNIT_BYTE	0
#define UNIT_KB		1
#define UNIT_MB		2
#define UNIT_GB		3

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CFormatDlg 

class CFormatDlg : public CDialog
{
// Konstruktion
public:
	void SetParent(CEToolsDlg *pParent, DISK *pDisk);
	CFormatDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CFormatDlg)
	enum { IDD = IDD_DLG_FORMAT };
	CStatic	m_ctlStaticMaxCapacity;
	CComboBox	m_ctlComboUnit;
	CStatic	m_ctlStaticStatus;
	CProgressCtrl	m_ctlProgress;
	CEdit	m_ctlEditDiskLabel;
	CEdit	m_ctlEditCapacity;
	BOOL	m_ctlCheckQuickformat;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	void SetProgress(int iProgress);
	void SetStatus(CString csStatus);
	BOOL Pump();
	int m_iLastUnit;
	CEToolsDlg *m_pParent;
	DISK *m_pDisk;
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CFormatDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeComboUnit();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_FORMATDLG_H__63646E29_107D_4EBF_8AA0_80420818C9B4__INCLUDED_
