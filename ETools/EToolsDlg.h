// EToolsDlg.h : Header-Datei
//

#if !defined(AFX_ETOOLSDLG_H__5D51097C_4A8E_4B1F_8EA9_7D297948EA5B__INCLUDED_)
#define AFX_ETOOLSDLG_H__5D51097C_4A8E_4B1F_8EA9_7D297948EA5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FAT_CACHED	0
#define FAT_WRITE	1

#include "diskstructure.h"

//----------------------------------------------------------------------------
// external DLL functions
//----------------------------------------------------------------------------
typedef void (__stdcall *tFreeDiskList)(int iShowProgress, DISK *pRoot);
typedef int (__stdcall *tCacheFlush)(DISK *pDisk);
typedef int (__stdcall *tReadBlocks)(DISK *pDisk, DWORD dwBlock,
	DWORD dwNumBlocks, unsigned char *ucBuf);
typedef int (__stdcall *tWriteBlocks)(DISK *pDisk, DWORD dwBlock,
	DWORD dwNumBlocks, unsigned char *ucBuf);
typedef int (__stdcall *tWriteBlocksUncached)(DISK *pDisk, DWORD dwBlock,
	DWORD dwNumBlocks, unsigned char *ucBuf);
typedef int (__stdcall *tGetUsageCount)(void);
typedef int (__stdcall *tGetFATEntry)(DISK *pDisk, DWORD dwBlock);
typedef int (__stdcall *tSetFATEntry)(DISK *pDisk, DWORD dwBlock,
	DWORD dwNewValue);
typedef DISK* (__stdcall *tScanDevices)(DWORD dwAllowNonEnsoniqFilesystems);



/////////////////////////////////////////////////////////////////////////////
// CEToolsDlg Dialogfeld

class CEToolsDlg : public CDialog
{
// Konstruktion
public:
	int SetFATEntryRAM(unsigned char *ucFAT, int iBlock, int iNewValue);
	int GetFATEntryRAM(unsigned char *ucFAT, int iBlock);
	CString CapacityString(int iDiskCapacity);
	CString SearchRecursive(CString csWhat, CString csWhere);
	void LOG(CString cs);
	CString GetErrorText(DWORD dwError);
	CEToolsDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

	tFreeDiskList FreeDiskList;
	tGetUsageCount GetUsageCount;
	tReadBlocks ReadBlocks;
	tWriteBlocks WriteBlocks;
	tWriteBlocksUncached WriteBlocksUncached;
	tGetFATEntry GetFATEntry;
	tSetFATEntry SetFATEntry;
	tScanDevices ScanDevices;
	tCacheFlush CacheFlush;

	// Dialogfelddaten
	//{{AFX_DATA(CEToolsDlg)
	enum { IDD = IDD_ETOOLS_DIALOG };
	CButton	m_ctlButtonExit;
	CButton	m_ctlButtonRestore;
	CButton	m_ctlButtonFormat;
	CButton	m_ctlButtonCheckFS;
	CButton	m_ctlButtonBackup;
	CButton	m_ctlButtonCancel;
	CComboBox	m_ctlComboDevices;
	CStatic	m_ctlStaticStatus;
	CProgressCtrl	m_ctlProgress;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CEToolsDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	CString SearchEnsoniqFsWfx(void);
	int CheckParentLinks(DISK *pDisk, BOOL bRepair, int iDir, int iParentDir, CString csDirName);
	int m_iMaxLogicalBlocks, m_iMaxPhysicalBlocks, m_iFiles, m_iDirectories, 
		m_iTempDirectories;
	int MarkUsedBlocks(DISK *pDisk, int iDirectoryBlock, unsigned char *ucFAT);
	void AddStatus(CString csStatus);
	void ShowCancel(BOOL bShow);
	void UpdateDeviceDropdown();
	HICON m_hIcon;
	BOOL m_bCancel;
	BOOL m_bWorking;
	CString m_csPathToDll;

	HINSTANCE m_hDLL;
	BOOL Pump();
	DISK *m_pDiskList;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CEToolsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnButtonBackup();
	afx_msg void OnButtonRestore();
	afx_msg void OnButtonCheckfs();
	afx_msg void OnButtonFormat();
	afx_msg void OnDropdownComboDevices();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonCreateIso();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_ETOOLSDLG_H__5D51097C_4A8E_4B1F_8EA9_7D297948EA5B__INCLUDED_)
