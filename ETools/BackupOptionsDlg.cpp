// BackupOptionsDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "ETools.h"
#include "BackupOptionsDlg.h"
#include "../EnsoniqFS/error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CBackupOptionsDlg::CBackupOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBackupOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBackupOptionsDlg)
	//}}AFX_DATA_INIT

	m_pFormatSelection = NULL;
	m_pFN = NULL;
	m_pParent = NULL;
	m_iValidFAT = 0;
	m_ucFAT = NULL;
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
CBackupOptionsDlg::~CBackupOptionsDlg()
{
	if(m_ucFAT) free(m_ucFAT);
}

//----------------------------------------------------------------------------
// Data exchange
//----------------------------------------------------------------------------
void CBackupOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackupOptionsDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_ctlProgress);
	DDX_Control(pDX, IDC_COMBO_BACKUP_FORMAT, m_ctlComboBackupFormat);
	DDX_Control(pDX, IDC_EDIT_FN, m_ctlEditFN);
	DDX_Control(pDX, IDC_STATIC_INFO, m_ctlStaticInfo);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------
// MESSAGE_MAP
//----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CBackupOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CBackupOptionsDlg)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_BACKUP_FORMAT, OnSelchangeComboBackupFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------
// OnOK
//----------------------------------------------------------------------------
void CBackupOptionsDlg::OnOK() 
{
	CString csFN;

	m_ctlEditFN.GetWindowText(csFN);
	if("choose a file"==csFN)
	{
		MessageBox("You did not choose a name for the backup file.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	m_ctlEditFN.GetWindowText(m_pFN, MAX_PATH);
	*m_pFormatSelection = m_ctlComboBackupFormat.GetCurSel();
	CDialog::OnOK();
}

//----------------------------------------------------------------------------
// OnCancel
//----------------------------------------------------------------------------
void CBackupOptionsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

//----------------------------------------------------------------------------
// SetReturnValuePointers
//
// Sets internal pointers to return value storage places
//
// -> pFormatSelection: Pointer to an integer to receive backup format 
//                      selection
//                      (BACKUP_FULL, BACKUP_PARTITION, BACKUP_FILES)
//    cFN: pointer to buffer for file name (261 characters including trailing
//         zero max.)
//    pParent: pointer to parent class
//    pDisk: pointer to disk structure
// <- --
//----------------------------------------------------------------------------
void CBackupOptionsDlg::SetReturnValuePointers(int *pFormatSelection,
											   char *pFN,
											   CEToolsDlg *pParent,
											   DISK *pDisk)
{
	m_pFormatSelection = pFormatSelection;
	m_pFN = pFN;
	m_pParent = pParent;
	m_pDisk = pDisk;
}

//----------------------------------------------------------------------------
// OnButtonBrowse
//----------------------------------------------------------------------------
void CBackupOptionsDlg::OnButtonBrowse() 
{
	static char BASED_CODE szFilter[] = "ISO images (*.iso)|*.iso|"
										"All Files (*.*)|*.*||";
	CFileDialog *dlg;
	CString csFN;

	// show "Save as..." dialog
	dlg = new CFileDialog(FALSE, "iso", "c:\\imagefile.iso", 
		OFN_OVERWRITEPROMPT, szFilter, this);
	if(NULL==dlg) return;
	if(IDCANCEL==dlg->DoModal()) return;
	csFN = dlg->GetPathName();
	delete dlg;

	m_ctlEditFN.SetWindowText(csFN);
}

//----------------------------------------------------------------------------
// WM_INITDIALOG
//----------------------------------------------------------------------------
BOOL CBackupOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;
}

//----------------------------------------------------------------------------
// WM_SHOWWINDOW
//----------------------------------------------------------------------------
void CBackupOptionsDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	m_ctlProgress.ShowWindow(SW_HIDE);
	m_ctlComboBackupFormat.ShowWindow(SW_SHOW);

	m_ctlEditFN.SetWindowText("choose a file");
	m_ctlComboBackupFormat.SetCurSel(BACKUP_FULL);
	OnSelchangeComboBackupFormat();
}

//----------------------------------------------------------------------------
// Pump
//
// Retrieve and dispatch any waiting messages.
//----------------------------------------------------------------------------
BOOL CBackupOptionsDlg::Pump()
{
	MSG msg;

	while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(!AfxGetApp()->PumpMessage())
		{
			::PostQuitMessage(0);
			return FALSE;
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------------
// OnSelchangeComboBackupFormat
//----------------------------------------------------------------------------
void CBackupOptionsDlg::OnSelchangeComboBackupFormat() 
{
	CString csText, csSize;
	int iResult, iSize, i;

	m_ctlStaticInfo.SetWindowText("");

	switch(m_ctlComboBackupFormat.GetCurSel())
	{
		case BACKUP_FULL:
			csSize = m_pParent->CapacityString((int)(m_pDisk->DiskGeometry.DiskSize.QuadPart/1024));
			csText.Format("This will create a full backup of the drive "
				"(size = %s).", csSize);
			break;

		case BACKUP_PARTITION:
			if(0==m_pDisk->iIsEnsoniq)
			{
				MessageBox("This feature will only work on Ensoniq "
					"filesystems.\n\n"
					"The drive you selected contains an unknown "
					"file-\nsystem, only full backups are supported.",
					"Ensoniq Filesystem Tools · Warning", 
					MB_ICONEXCLAMATION | MB_OK);
				m_ctlComboBackupFormat.SetCurSel(BACKUP_FULL);
				break;
			}
			csSize = m_pParent->CapacityString(m_pDisk->dwBlocks/2);
			csText.Format("This will create a backup of the partition "
				"(size = %s).", csSize);
			break;

		case BACKUP_FILES:
			if(0==m_pDisk->iIsEnsoniq)
			{
				MessageBox("This feature will only work on Ensoniq "
					"filesystems.\n\n"
					"The drive you selected contains an unknown "
					"file-\nsystem, only full backups are supported.",
					"Ensoniq Filesystem Tools · Warning", 
					MB_ICONEXCLAMATION | MB_OK);
				m_ctlComboBackupFormat.SetCurSel(BACKUP_FULL);
				break;
			}

			// read FAT
			if(0==m_iValidFAT)
			{
				m_ctlProgress.ShowWindow(SW_SHOW);
				m_ctlComboBackupFormat.ShowWindow(SW_HIDE);
				m_ctlStaticInfo.SetWindowText("Reading FAT...");
				iResult = ReadFAT();
				m_ctlProgress.ShowWindow(SW_HIDE);
				m_ctlComboBackupFormat.ShowWindow(SW_SHOW);
				if(0==iResult)
				{
					break;
				}
				m_iValidFAT = 1;
			}

			// count used blocks
			iSize = 0;
			for(i=0; i<(int)m_pDisk->dwBlocks; i++)
			{
				iResult = GetFATEntry(i);
				if((2!=iResult)&&(0!=iResult)) iSize++;
			}

			// subtract old FAT size (from whole partition)
			iSize -= m_iFATSize;

			// new FAT needs minimum "iSize" entries
			// add new FAT space
			iSize += (iSize/170)+1;

			csSize = m_pParent->CapacityString(iSize/2);
			csText.Format("This will backup all files on this disk "
				"(size = %s) into a new ISO file.", csSize);
			break;

		default:
			csText = "";
			break;
	}

	m_ctlStaticInfo.SetWindowText(csText);
}

//----------------------------------------------------------------------------
// GetFATEntry
// 
// Reads a FAT entry out of loaded FAT
// 
// -> iBlock = number of FAT entry to read
// <- content of FAT entry
//----------------------------------------------------------------------------
int CBackupOptionsDlg::GetFATEntry(int iBlock)
{
	int i;

	if(iBlock>=(int)m_pDisk->dwBlocks) return 0;
	if(iBlock<0) return 0;

	// read FAT entry
	i = ((iBlock/170)*512) + (iBlock%170)*3;
	return (m_ucFAT[i+0]<<16) + (m_ucFAT[i+1]<<8) + m_ucFAT[i+2];
}

int CBackupOptionsDlg::ReadFAT()
{
	int i;

	m_iFATSize = m_pDisk->dwBlocks/170; 
	if(m_pDisk->dwBlocks%170) m_iFATSize++;
	if(m_ucFAT) free(m_ucFAT);
	m_ucFAT = (unsigned char*)malloc(m_iFATSize*512);
	if(NULL==m_ucFAT)
	{
		MessageBox("Could not allocate FAT memory.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// read FAT blocks
	m_ctlProgress.SetPos(0);
	for(i=0; i<m_iFATSize/64; i++)
	{
		if(ERR_OK!=m_pParent->ReadBlocks(m_pDisk, 5+i*64, 64, 
			m_ucFAT+i*64*512))
		{
			MessageBox("Unable to read FAT.",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION|MB_OK);
			free(m_ucFAT); m_ucFAT = NULL; return 0;
		}
		m_ctlProgress.SetPos(100*64*i/m_iFATSize);
	}

	// read rest of FAT blocks
	if(m_iFATSize%64)
	{
		if(ERR_OK!=m_pParent->ReadBlocks(m_pDisk, 5+i*64, m_iFATSize%64, 
			m_ucFAT+i*64*512))
		{
			MessageBox("Unable to read FAT.",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION|MB_OK);
			free(m_ucFAT); m_ucFAT = NULL; return 0;
		}
	}
	m_ctlProgress.SetPos(100);

	return 1;
}
