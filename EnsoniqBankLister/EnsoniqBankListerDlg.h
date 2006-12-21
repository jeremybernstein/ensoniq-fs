// EnsoniqBankListerDlg.h : Header-Datei
//

#if !defined(AFX_ENSONIQBANKLISTERDLG_H__6CFEF67D_6839_40AB_81EB_D378E409F51A__INCLUDED_)
#define AFX_ENSONIQBANKLISTERDLG_H__6CFEF67D_6839_40AB_81EB_D378E409F51A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEnsoniqBankListerDlg Dialogfeld

class CEnsoniqBankListerDlg : public CDialog
{
// Konstruktion
public:
	CEnsoniqBankListerDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CEnsoniqBankListerDlg)
	enum { IDD = IDD_ENSONIQBANKLISTER_DIALOG };
	CEdit	m_ctlEditDetails;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CEnsoniqBankListerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon;
	CFont m_EditFont;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CEnsoniqBankListerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_ENSONIQBANKLISTERDLG_H__6CFEF67D_6839_40AB_81EB_D378E409F51A__INCLUDED_)
