#if !defined(AFX_BACKUPOPTIONSDLG_H__A88A286B_F17A_48C9_8DDF_187970287CBB__INCLUDED_)
#define AFX_BACKUPOPTIONSDLG_H__A88A286B_F17A_48C9_8DDF_187970287CBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BackupOptionsDlg.h : Header-Datei
//

#include "EToolsDlg.h"
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBackupOptionsDlg 

#define BACKUP_FULL			0
#define BACKUP_PARTITION	1
#define BACKUP_FILES		2

class CBackupOptionsDlg : public CDialog
{
// Konstruktion
public:
	int ReadFAT();
	void SetReturnValuePointers(int *pFormatSelection, char *pFN, CEToolsDlg *pParent, DISK *pDisk);
	CBackupOptionsDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	~CBackupOptionsDlg();

// Dialogfelddaten
	//{{AFX_DATA(CBackupOptionsDlg)
	enum { IDD = IDD_DLG_BACKUP_OPTIONS };
	CProgressCtrl	m_ctlProgress;
	CComboBox	m_ctlComboBackupFormat;
	CEdit	m_ctlEditFN;
	CStatic	m_ctlStaticInfo;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CBackupOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	BOOL Pump();
	int *m_pFormatSelection, m_iFATSize, m_iValidFAT;
	char *m_pFN;
	CEToolsDlg *m_pParent;
	DISK *m_pDisk;
	unsigned char *m_ucFAT;
	int GetFATEntry(int iBlock);

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CBackupOptionsDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioBackup();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboBackupFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_BACKUPOPTIONSDLG_H__A88A286B_F17A_48C9_8DDF_187970287CBB__INCLUDED_
