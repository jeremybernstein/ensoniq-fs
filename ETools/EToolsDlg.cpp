// EToolsDlg.cpp : Implementierungsdatei
//


#include "stdafx.h"
#include <io.h>
#include "ETools.h"
#include "EToolsDlg.h"
#include "FormatDlg.h"
#include "BackupOptionsDlg.h"
#include "../EnsoniqFS/error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CAboutDlg
//----------------------------------------------------------------------------
// CAboutDlg
//----------------------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// Keine Nachrichten-Handler
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////
// CEToolsDlg Dialog
//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CEToolsDlg::CEToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEToolsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEToolsDlg)
		// HINWEIS: Der Klassenassistent fügt hier Member-Initialisierung ein
	//}}AFX_DATA_INIT
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hDLL = NULL;
	m_pDiskList = NULL;
	m_bCancel = FALSE;
	m_bWorking = FALSE;
}

//----------------------------------------------------------------------------
// DoDataExchange
//----------------------------------------------------------------------------
void CEToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEToolsDlg)
	DDX_Control(pDX, IDCANCEL, m_ctlButtonExit);
	DDX_Control(pDX, IDC_BUTTON_RESTORE, m_ctlButtonRestore);
	DDX_Control(pDX, IDC_BUTTON_FORMAT, m_ctlButtonFormat);
	DDX_Control(pDX, IDC_BUTTON_CHECKFS, m_ctlButtonCheckFS);
	DDX_Control(pDX, IDC_BUTTON_BACKUP, m_ctlButtonBackup);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_ctlButtonCancel);
	DDX_Control(pDX, IDC_COMBO_DEVICES, m_ctlComboDevices);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctlStaticStatus);
	DDX_Control(pDX, IDC_PROGRESS, m_ctlProgress);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------
// MESSAGE_MAP
//----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CEToolsDlg, CDialog)
	//{{AFX_MSG_MAP(CEToolsDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BACKUP, OnButtonBackup)
	ON_BN_CLICKED(IDC_BUTTON_RESTORE, OnButtonRestore)
	ON_BN_CLICKED(IDC_BUTTON_CHECKFS, OnButtonCheckfs)
	ON_BN_CLICKED(IDC_BUTTON_FORMAT, OnButtonFormat)
	ON_CBN_DROPDOWN(IDC_COMBO_DEVICES, OnDropdownComboDevices)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////(//////////////////////////////////////////////////////////////
// CEToolsDlg Nachrichten-Handler
//----------------------------------------------------------------------------
// OnInitDialog
//----------------------------------------------------------------------------
BOOL CEToolsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{	
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden
	


	char cModuleFilename[MAX_PATH];
	if(0!=GetModuleFileName(NULL, cModuleFilename, MAX_PATH))
	{
		m_csPathToDll = cModuleFilename;
		m_csPathToDll = m_csPathToDll.Left(m_csPathToDll.GetLength()-10);
		m_csPathToDll += "ensoniqfs.wfx";

		// if file doesn't exist, try to locate it "manually"
		if(0!=_access(m_csPathToDll, 0))
			m_csPathToDll = SearchEnsoniqFsWfx();
	}
	else
	{
		// try to locate EnsoniqFS.wfx inside the TotalCommander
		// installation (recursive search)
		m_csPathToDll = SearchEnsoniqFsWfx();
	}
	
	TRACE("PathToDll=\"%s\"\n", m_csPathToDll);

	if(""==m_csPathToDll)
	{
		MessageBox("Could not find the plugin \"EnsoniqFS.wfx\".\n\n"
			"Please make sure you have (a) successfully installed a recent\n"
			"version of TotalCommander and (b) installed \"EnsoniqFS.wfx\""
			"\nsomewhere inside the TotalCommander directory.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
	}
	else
	{
		// try to load EnsoniqFS
		m_hDLL = LoadLibrary(m_csPathToDll);
		if(NULL==m_hDLL)
		{
			MessageBox("Loading of EnsoniqFS.wfx failed (LoadLibrary()).\n\n"
			"Please make sure you have (a) successfully installed a recent\n"
			"version of TotalCommander and (b) installed \"EnsoniqFS.wfx\""
			"\ninto the default directory (TotalCommander\\plugins\\wfx).",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_OK);
		}
		else
		{
			this->ScanDevices = (tScanDevices)GetProcAddress(m_hDLL, "ScanDevices");
			this->FreeDiskList = (tFreeDiskList)GetProcAddress(m_hDLL, "FreeDiskList");
			this->ReadBlocks = (tReadBlocks)GetProcAddress(m_hDLL, "ReadBlocks");
			this->WriteBlocks = (tWriteBlocks)GetProcAddress(m_hDLL, "WriteBlocks");
			this->WriteBlocksUncached = (tWriteBlocksUncached)GetProcAddress(m_hDLL, "WriteBlocksUncached");
			this->GetFATEntry = (tGetFATEntry)GetProcAddress(m_hDLL, "GetFATEntry");
			this->SetFATEntry = (tSetFATEntry)GetProcAddress(m_hDLL, "SetFATEntry");
			this->CacheFlush = (tCacheFlush)GetProcAddress(m_hDLL, "CacheFlush");
			this->GetUsageCount = (tGetUsageCount)GetProcAddress(m_hDLL, "GetUsageCount");

			if((NULL==this->ScanDevices) ||
			   (NULL==this->FreeDiskList) ||
			   (NULL==this->ReadBlocks) ||
			   (NULL==this->WriteBlocks) ||
			   (NULL==this->WriteBlocksUncached) ||
			   (NULL==this->GetFATEntry) ||
			   (NULL==this->CacheFlush) ||
			   (NULL==this->GetUsageCount) ||
			   (NULL==this->SetFATEntry))
			{
				MessageBox("Loading of EnsoniqFS.wfx failed (GetProcAddress()).", 
					"Ensoniq Filesystem Tools · Warning", 
					MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}

	return TRUE;
}

#define LOGFILE	"C:\\ETools-LOG.txt"
void CEToolsDlg::LOG(CString cs)
{
	FILE *pDebug;
	pDebug = fopen(LOGFILE, "a+");
	if(NULL==pDebug) return;
	fprintf(pDebug, cs);
	fclose(pDebug);
}

//----------------------------------------------------------------------------
// OnSysCommand
//----------------------------------------------------------------------------
void CEToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

//----------------------------------------------------------------------------
// OnPaint
//----------------------------------------------------------------------------
void CEToolsDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext für Zeichnen

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Symbol in Client-Rechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//----------------------------------------------------------------------------
// OnQueryDragIcon
//----------------------------------------------------------------------------
HCURSOR CEToolsDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//----------------------------------------------------------------------------
// OnOK
//----------------------------------------------------------------------------
void CEToolsDlg::OnOK() 
{
//	CDialog::OnOK();
}

//----------------------------------------------------------------------------
// OnCancel
//----------------------------------------------------------------------------
void CEToolsDlg::OnCancel() 
{
	if(m_bWorking) return;

	CDialog::OnCancel();
}

//----------------------------------------------------------------------------
// OnButtonBackup
//----------------------------------------------------------------------------
void CEToolsDlg::OnButtonBackup() 
{
#define BUF_SIZE 512*128

	int iBlock, i, iMaxBlocks, iBlocksToRead, iSpeed, iLastBlock, iResult,
		iBackupFormat;
	DWORD dwTime, dwLastTime, dwBytesWritten, dwError;
	CString csDevice, csTemp, csSpeed, csTime;
	unsigned char ucBuf[BUF_SIZE];
	char cFN[MAX_PATH+1];
	DISK *pDisk = NULL;
	HANDLE h;


	// check function pointers
	if(NULL==this->ReadBlocks) return;

	// get device name
	m_ctlComboDevices.GetWindowText(csDevice);
	if(""==csDevice)
	{
		MessageBox("Please select a device in the device list above.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	csTemp = "";
	for(i=0; i<csDevice.GetLength(); i++)
	{
		// look for closing ':'
		if(':'==csDevice[i])
		{
			if(5==i)
			{
				// ignore ':' at position if floppy "A:" or "B:"
				if(('A'!=csDevice[i-1])&&('B'!=csDevice[i-1])) break;
			}
			else if(11==i)
			{
				// ignore ':' at position 11 if this is an image file
				if(0!=strncmp(csDevice, "\\\\.\\image=", 10)) break;
			}
			else break;
		}
		csTemp += csDevice[i];
	}
	csDevice = csTemp;

	pDisk = m_pDiskList;
	while(pDisk)
	{
		if(csTemp == pDisk->cMsDosName)
		{
			break;
		}
		pDisk = pDisk->pNext;
	}

	if(NULL==pDisk)
	{
		MessageBox("Could not find selected device.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		m_ctlStaticStatus.SetWindowText("Could not find device.");
		return;
	}

	CBackupOptionsDlg *dlg;
	dlg = new CBackupOptionsDlg();
	dlg->SetReturnValuePointers(&iBackupFormat, cFN, this, pDisk);
	if(IDCANCEL==dlg->DoModal())
	{
		delete dlg;
		return;
	}
	delete dlg;

	if(BACKUP_FILES==iBackupFormat)
	{
		if(IDNO==MessageBox("Before backing up a drive with the "
			"\"files only\" method it is recommended\n"
			"to run a filesystem check. Otherwise, possibly not "
			"all files will be processed.\n\n"
			"Do you want to continue?",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_YESNO))
		{
			return;
		}
	}
	else if(BACKUP_PARTITION==iBackupFormat) iMaxBlocks = pDisk->dwBlocks;
	else iMaxBlocks = (int)(pDisk->DiskGeometry.DiskSize.QuadPart/512);

	if(BACKUP_FILES==iBackupFormat)
	{
		MessageBox("BACKUP_FILES currently not supported");
		return;
	}

	h = CreateFile(cFN, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(INVALID_HANDLE_VALUE==h)
	{
		dwError = GetLastError();
		csTemp.Format("Unable to open destination file.\n\n%s", 
			GetErrorText(dwError));
		MessageBox(csTemp, "Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		m_ctlStaticStatus.SetWindowText("Error opening destination file.");
		return;
	}

	m_ctlStaticStatus.SetWindowText("Reading disk, writing to image...");
	iBlock = 0; m_bWorking = TRUE;
	m_bCancel = FALSE; ShowCancel(TRUE);
	dwTime = GetTickCount(); dwLastTime = dwTime;

	if(BACKUP_FILES==iBackupFormat)
	{
		MessageBox("BACKUP_FILES currently not supported");
		return;
	}
	else
	{
		// read all blocks from device
		while(iBlock<iMaxBlocks)
		{
			iBlocksToRead = 128;
			if((iMaxBlocks-iBlock)<128) iBlocksToRead = iMaxBlocks-iBlock;

			// read blocks
			if(ERR_OK!=ReadBlocks(pDisk, iBlock, iBlocksToRead, ucBuf))
			{
				MessageBox("Error reading from device.",
					"Ensoniq Filesystem Tools · Warning", 
					MB_ICONEXCLAMATION | MB_OK);
				m_ctlStaticStatus.SetWindowText("Error reading from device.");
				CloseHandle(h);
				ShowCancel(FALSE);
				m_bWorking = FALSE;
				return;
			}

			// write blocks
			iResult = WriteFile(h, ucBuf, iBlocksToRead*512, &dwBytesWritten, 0);
			if((512*iBlocksToRead!=(int)dwBytesWritten)||(iResult==0))
			{
				MessageBox("Error writing to destination file.",
					"Ensoniq Filesystem Tools · Warning", 
					MB_ICONEXCLAMATION | MB_OK);
				CloseHandle(h);
				m_ctlStaticStatus.SetWindowText("Error writing to "
					"destination file.");
				ShowCancel(FALSE);
				m_bWorking = FALSE;
				return;
			}

			// set progress bar
			iBlock += iBlocksToRead;
			m_ctlProgress.SetPos(iBlock*100/iMaxBlocks);

			if(GetTickCount()>(dwLastTime+1000))
			{
				if((GetTickCount()-dwLastTime)>0)
				{
					iSpeed = iBlock/2 - iLastBlock/2;
					iSpeed *= 1000;
					iSpeed /= GetTickCount()-dwLastTime;
					csSpeed.Format("%i", iSpeed);
					if(iSpeed>=1)
					{
						csTime.Format("%02i:%02i:%02i", 
							(((iMaxBlocks-iBlock)/2)/iSpeed)/3600,
							((((iMaxBlocks-iBlock)/2)/iSpeed)%3600)/60,
							(((iMaxBlocks-iBlock)/2)/iSpeed)%60);
					}
					else csTime = "---";
				}
				else
				{
					csSpeed = "---";
					csTime = "---";
				}

				csTemp.Format("Reading disk, writing to image...\n\n"
					"%i %% done (%i kB of %i kB), %s kB/sec, "
					"estimated time left: %s",
					iBlock*100/iMaxBlocks, iBlock/2, iMaxBlocks/2, csSpeed,
					csTime);

				m_ctlStaticStatus.SetWindowText(csTemp);
				dwLastTime = GetTickCount();
				iLastBlock = iBlock;
			}

			Pump();

			if(m_bCancel)
			{
				if(IDYES==MessageBox("Do you want to cancel the operation?",
					"Ensoniq Filesystem Tools",
					MB_ICONQUESTION | MB_YESNO))
				{
					m_ctlStaticStatus.SetWindowText("Backup cancelled.");
					CloseHandle(h);
					ShowCancel(FALSE);
					m_bWorking = FALSE;
					return;
				}
				else
				{
					m_bCancel = FALSE;
				}
			}
		}
	}

	CloseHandle(h);
	m_ctlStaticStatus.SetWindowText("Ensoniq Filesystem Tools ready.\n\n"
		"Backup successful.");
	ShowCancel(FALSE);
	m_bWorking = FALSE;

}

//----------------------------------------------------------------------------
// OnButtonRestore
//----------------------------------------------------------------------------
void CEToolsDlg::OnButtonRestore() 
{
	static char BASED_CODE szFilter[] = "ISO images (*.iso)|*.iso|"
										"All Files (*.*)|*.*||";
	CString csDevice, csFN, csTemp, csSpeed, csTime;
	unsigned char ucBuf[BUF_SIZE];
	DISK *pDisk = NULL;
	CFileDialog *dlg;
	int iBlock, i, iBlocksDst, iBlocksSrc, iBlocksToRead, iMaxBlocks,
		iResult, iIgnoreError = 0, iErrors = 0, iLastBlock = 0, iSpeed;
	HANDLE h;
	DWORD dwTime, dwLastTime, fsh = 0, fsl = 0, dwError, dwBytesRead;
	__int64 ii;

	// check function pointers
	if(NULL==this->WriteBlocksUncached) return;

	// get device name
	m_ctlComboDevices.GetWindowText(csDevice);
	if(""==csDevice)
	{
		MessageBox("Please select a device in the device list above.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	csTemp = "";
	for(i=0; i<csDevice.GetLength(); i++)
	{
		// look for closing ':'
		if(':'==csDevice[i])
		{
			if(5==i)
			{
				// ignore ':' at position if floppy "A:" or "B:"
				if(('A'!=csDevice[i-1])&&('B'!=csDevice[i-1])) break;
			}
			else if(11==i)
			{
				// ignore ':' at position 11 if this is an image file
				if(0!=strncmp(csDevice, "\\\\.\\image=", 10)) break;
			}
			else break;
		}
		csTemp += csDevice[i];
	}
	csDevice = csTemp;

	pDisk = m_pDiskList;
	while(pDisk)
	{
		if(csTemp == pDisk->cMsDosName)
		{
			break;
		}
		pDisk = pDisk->pNext;
	}

	if(NULL==pDisk)
	{
		MessageBox("Could not find selected device.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		m_ctlStaticStatus.SetWindowText("Could not find device.");
		return;
	}

	// show "Open..." dialog
 	dlg = new CFileDialog(TRUE, "iso", "c:\\imagefile.iso", 
		0, szFilter, this);
	if(NULL==dlg) return;
	if(IDCANCEL==dlg->DoModal()) return;
	csFN = dlg->GetPathName();
	delete dlg;

	// open file for reading
	h = CreateFile(csFN, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if(INVALID_HANDLE_VALUE==h)
	{
		dwError = GetLastError();
		csTemp.Format("Unable to open source file.\n\n%s", 
			GetErrorText(dwError));
		MessageBox(csTemp, "Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		m_ctlStaticStatus.SetWindowText("Error opening source file.");
		return;
	}

	m_ctlStaticStatus.SetWindowText("Reading image, writing to disk...");

	// get filesize
	fsl = GetFileSize(h, &fsh); dwError = GetLastError();
	if((0xFFFFFFFF==fsl)&&(NO_ERROR!=dwError))
	{
		csTemp.Format("Could not read file size.\n\n%s", 
			GetErrorText(dwError));
		MessageBox(csTemp, "Ensoniq Filesystem Tools · Error", 
			MB_ICONSTOP | MB_OK);
		m_ctlStaticStatus.SetWindowText("Could not read file size.");
		return;
	}

	// calculate number of source blocks from filesize
	ii = fsh; ii <<= 32; ii += fsl;
	iBlocksSrc = (int)((__int64)ii / (__int64)512);

	// number of destination blocks from physical disk geometry
	iBlocksDst = (int)(pDisk->DiskGeometry.DiskSize.QuadPart/512);

	if(iBlocksDst<iBlocksSrc) iMaxBlocks = iBlocksDst;
	else iMaxBlocks = iBlocksSrc;

	// check sizes
	if(iBlocksSrc>iBlocksDst)
	{
		if(IDNO==MessageBox("The image file is larger than the destination"
			" disk.\nWriting this image to that disk will most likely lead"
			"\nto a broken filesystem.\n\n"
			"Do you still want to write the image file to this disk?",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_YESNO))
		{
			CloseHandle(h);
			m_ctlStaticStatus.SetWindowText("Writing aborted.");
			return;
		}
	}

	// check sizes
	if(((float)iBlocksSrc/(float)iBlocksDst)<0.8f)
	{
		if(IDNO==MessageBox("The image file considerably smaller than the "
			"destination disk (more than 20 %).\n"
			"Writing this image to that disk will leave lots of space on"
			" the disk unused. After\n"
			"restore, the disk will have the same logical size as the "
			"image.\n\n"
			"This is no serious problem, especially if you want to use a "
			"large disk with\n"
			"Ensoniq samplers. It's like formatting the disk to a lower "
			"capacity.\n\n"
			"Do you still want to write the image file to this disk?",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_YESNO))
		{
			CloseHandle(h);
			m_ctlStaticStatus.SetWindowText("Writing aborted.");
			return;
		}
	}

	if(IDNO==MessageBox("Writing an image to a disk will overwrite ALL "
		"data stored on that disk.\n\n"
		"Do you still want to write the image file to this disk?",
		"Ensoniq Filesystem Tools · Warning", 
		MB_ICONEXCLAMATION | MB_YESNO))
	{
		CloseHandle(h);
		m_ctlStaticStatus.SetWindowText("Writing aborted.");
		return;
	}

	if(0==pDisk->iIsEnsoniq)
	{
		if(IDNO==MessageBox("The destination disk is not Ensoniq formatted."
			"\n\nIt could be that you have chosen one of your computer's\n"
			"harddisks. If this is one of your windows drives, you will\n"
			"definitely kill your Windows installation and all your private"
			"\ndata.\n\n"
			"This is your last chance to avoid a disaster.\n"
			"Do you really want to write the image file to this disk?",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_YESNO))
		{
			CloseHandle(h);
			m_ctlStaticStatus.SetWindowText("Writing aborted.");
			return;
		}
	}	

	iBlock = 0; m_bWorking = TRUE;
	m_bCancel = FALSE; ShowCancel(TRUE);
	dwTime = GetTickCount(); dwLastTime = dwTime;

	// write all blocks to device
	while(iBlock<iMaxBlocks)
	{
		iBlocksToRead = 128;
		if((iMaxBlocks-iBlock)<128) iBlocksToRead = iMaxBlocks-iBlock;

		// read blocks
		iResult = ReadFile(h, ucBuf, 512*iBlocksToRead, &dwBytesRead, 0);
		if((512*iBlocksToRead!=(int)dwBytesRead)||(0==iResult))
		{
			MessageBox("Error reading from source file.",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_OK);
			CloseHandle(h);
			m_ctlStaticStatus.SetWindowText("Error reading from "
				"source file.");
			ShowCancel(FALSE);
			m_bWorking = FALSE;
			return;
		}

		iResult = WriteBlocksUncached(pDisk, iBlock, iBlocksToRead, ucBuf);
		if(ERR_OK!=iResult)
		{
			TRACE("Write error.\n");

			// try to write the blocks one by one
			for(i=0; i<iBlocksToRead; i++)
			{
				iResult = WriteBlocksUncached(pDisk, iBlock+i, 1, 
					ucBuf+i*512);
				if(ERR_OK!=iResult)
				{
					iErrors++;
					TRACE("Writing single block %i: code=%i\n", iBlock+i,
						iResult);

					// was there an error before?
					if(0==iIgnoreError)
					{
						csTemp.Format("Error writing to device, code=%i.\n\n"
							"Do you want to ignore errors and continue?", 
							iResult);
						if(MessageBox(csTemp, 
							"Ensoniq Filesystem Tools · Warning", 
							MB_ICONEXCLAMATION | MB_YESNO)==IDYES)
						{
							// ignore all future errors
							TRACE("Continue with ignore.\n");
							iIgnoreError = 1;
						}
						else
						{
							// abort
							TRACE("Abort.\n");

							csTemp.Format("Error writing to device, "
								"code=%i.\n", iResult);
							m_ctlStaticStatus.SetWindowText(csTemp);

							CloseHandle(h);
							ShowCancel(FALSE);
							m_bWorking = FALSE;
							return;
						}
					}
				}
			}
		}

		// set progress bar
		iBlock += iBlocksToRead;
		m_ctlProgress.SetPos(iBlock*100/iMaxBlocks);

		if(GetTickCount()>(dwLastTime+1000))
		{
			if((GetTickCount()-dwLastTime)>0)
			{
				iSpeed = iBlock/2 - iLastBlock/2;
				iSpeed *= 1000;
				iSpeed /= GetTickCount()-dwLastTime;
				csSpeed.Format("%i", iSpeed);
				if(iSpeed>=1)
				{
					csTime.Format("%02i:%02i:%02i", 
						(((iMaxBlocks-iBlock)/2)/iSpeed)/3600,
						((((iMaxBlocks-iBlock)/2)/iSpeed)%3600)/60,
						(((iMaxBlocks-iBlock)/2)/iSpeed)%60);
				}
				else csTime = "---";
			}
			else
			{
				csSpeed = "---";
				csTime = "---";
			}

			csTemp.Format("Reading image, writing to disk...\n\n"
				"%i %% done (%i kB of %i kB), %s kB/sec, %i errors, "
				"estimated time left: %s",
				iBlock*100/iMaxBlocks, iBlock/2, iMaxBlocks/2, csSpeed,
				iErrors, csTime);

			m_ctlStaticStatus.SetWindowText(csTemp);
			dwLastTime = GetTickCount();
			iLastBlock = iBlock;
		}

		Pump();

		if(m_bCancel)
		{
			if(IDYES==MessageBox("Do you want to cancel the operation?",
				"Ensoniq Filesystem Tools",
				MB_ICONQUESTION | MB_YESNO))
			{
				m_ctlStaticStatus.SetWindowText("Restore cancelled.");
				CloseHandle(h);
				ShowCancel(FALSE);
				m_bWorking = FALSE;
				return;
			}
			else
			{
				m_bCancel = FALSE;
			}
		}
	}

	CloseHandle(h);
	m_ctlStaticStatus.SetWindowText("Ensoniq Filesystem Tools ready.\n\n"
		"Restore successful.");
	ShowCancel(FALSE);
	m_bWorking = FALSE;

	CacheFlush(pDisk);

	UpdateDeviceDropdown();
}

//----------------------------------------------------------------------------
// OnButtonCheckfs
//----------------------------------------------------------------------------
void CEToolsDlg::OnButtonCheckfs() 
{
	CString csDevice, csTemp, csFN, csPath;
	DISK *pDisk = NULL;
	int i, iFATSize, iFreeBlocks, iFreeBlocksOS, iBlock, iLostBlocks,
		iLostChains, iLastBlock, iResult;
	unsigned char ucDeviceIDBlock[512], ucOSBlock[512], *ucFAT, *ucFATCopy,
		ucBuf[512];
	FILE *f = NULL;

	// check function pointers
	if(NULL==this->ReadBlocks) return;
	if(NULL==this->WriteBlocks) return;
	if(NULL==this->CacheFlush) return;

	//........................................................................
	// init
	//........................................................................

	// get device name
	m_ctlComboDevices.GetWindowText(csDevice);
	if(""==csDevice)
	{
		MessageBox("Please select a device in the device list above.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	csTemp = "";
	for(i=0; i<csDevice.GetLength(); i++)
	{
		// look for closing ':'
		if(':'==csDevice[i])
		{
			if(5==i)
			{
				// ignore ':' at position if floppy "A:" or "B:"
				if(('A'!=csDevice[i-1])&&('B'!=csDevice[i-1])) break;
			}
			else if(11==i)
			{
				// ignore ':' at position 11 if this is an image file
				if(0!=strncmp(csDevice, "\\\\.\\image=", 10)) break;
			}
			else break;
		}
		csTemp += csDevice[i];
	}
	csDevice = csTemp;

	pDisk = m_pDiskList;
	while(pDisk)
	{
		if(csTemp == pDisk->cMsDosName)
		{
			break;
		}
		pDisk = pDisk->pNext;
	}

	if(NULL==pDisk)
	{
		m_ctlStaticStatus.SetWindowText("Could not find device.");
		MessageBox("Could not find selected device.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//........................................................................
	// read DeviceID block, OS block, check Ensoniq signature & disk geometry
	//........................................................................

	// read DeviceID block
	m_ctlStaticStatus.SetWindowText("");
	AddStatus("Reading DeviceID block: ");

	if(ERR_OK!=ReadBlocks(pDisk, 1, 1, ucDeviceIDBlock))
	{
		AddStatus("failed.");
		MessageBox("Unable to read DeviceID block.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// read OS block
	AddStatus("OK.\nReading OS block: ");

	if(ERR_OK!=ReadBlocks(pDisk, 2, 1, ucOSBlock))
	{
		AddStatus("failed.");
		MessageBox("Unable to read OS block.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// check Ensoniq signature
	AddStatus("OK.\nChecking Ensoniq signature: ");
	if(('I'!=ucDeviceIDBlock[38])||('D'!=ucDeviceIDBlock[39])||
	   ('O'!=ucOSBlock[28])||('S'!=ucOSBlock[29]))
	{
		AddStatus("failed.\n");
		MessageBox("There is no Ensoniq filesystem on this disk.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// check disk geometry
	AddStatus("OK.\nChecking disk geometry: ");
	m_iMaxPhysicalBlocks = (int)(pDisk->DiskGeometry.DiskSize.QuadPart/512);
	m_iMaxLogicalBlocks = (ucDeviceIDBlock[17])
					    + (ucDeviceIDBlock[16]<<8)
					    + (ucDeviceIDBlock[15]<<16)
					    + (ucDeviceIDBlock[14]<<24);

	if(m_iMaxLogicalBlocks>m_iMaxPhysicalBlocks)
	{
		AddStatus("failed. File system allocates more space than physically "
			"available.\n");

		if(MessageBox("The file system allocates more space than "
			"physically available.\n"
			"This is most likely due to wrong formatting or writing an "
			"image to\n"
			"this disk which is too big.\n\n"
			"At the moment the only way to correct this problem is to "
			"read each\n"
			"and every file from this disk (use EnsoniqFS and Total"
			"Commander).\n"
			"There may be read errors due to the reported problem. "
			"Then\nreformat "
			"the disk and write back all files you copied before.\n\n"
			"This error will probably cause more errors which will "
			"follow.\nContinue testing?",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_YESNO)==IDNO)
		{
			return;
		}
	}
	else if(m_iMaxLogicalBlocks>0xFFFFFF)
	{
		AddStatus("failed. File system allocates more than 8 GB.\n");

		if(MessageBox("The file system allocates more than 8 GB of disk "
			"space.\n"
			"The Ensoniq file system structure is internally limited to "
			"8 GB.\n"
			"This disk will produce errors when used in any Ensoniq compa-"
			"\ntible device.\n\n"
			"This is most likely due to wrong formatting or writing an "
			"image to\n"
			"this disk which is too big.\n\n"
			"At the moment the only way to correct this problem is to "
			"read each\n"
			"and every file from this disk (use EnsoniqFS and Total"
			"Commander).\n"
			"There may be read errors due to the reported problem. "
			"Then\nreformat "
			"the disk and write back all files you copied before.\n\n"
			"This error will probably cause more errors which will "
			"follow.\nContinue testing?",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_YESNO)==IDNO)
		{
			return;
		}
	}
	else
	{
		AddStatus("OK.\n");
	}

	//........................................................................
	// read FAT
	//........................................................................
	AddStatus("Reading FAT: ");

	iFATSize = m_iMaxLogicalBlocks/170; 
	if(m_iMaxLogicalBlocks%170) iFATSize++;
	ucFAT = (unsigned char*)malloc(iFATSize*512);
	ucFATCopy = (unsigned char*)malloc(iFATSize*512);
	if((NULL==ucFAT)||(NULL==ucFATCopy))
	{
		csTemp.Format("Unable to allocate FAT memory (%i kB).\n", iFATSize);
		AddStatus(csTemp);
		MessageBox("Could not allocate FAT memory.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		free(ucFAT); free(ucFATCopy);
		return;
	}

	// read FAT blocks
	m_ctlProgress.SetPos(0);
	for(i=0; i<iFATSize/64; i++)
	{
		if(ERR_OK!=ReadBlocks(pDisk, 5+i*64, 64, ucFAT+i*64*512))
		{
			AddStatus("failed."); MessageBox("Unable to read FAT.",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION|MB_OK);
			free(ucFAT); free(ucFATCopy); return;
		}
		m_ctlProgress.SetPos(100*64*i/iFATSize);
	}

	// read rest of FAT blocks
	if(iFATSize%64)
	{
		if(ERR_OK!=ReadBlocks(pDisk, 5+i*64, iFATSize%64, ucFAT+i*64*512))
		{
			AddStatus("failed."); MessageBox("Unable to read FAT.",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION|MB_OK);
			free(ucFAT); free(ucFATCopy); return;
		}
	}
	m_ctlProgress.SetPos(100);

	// make a working copy of the FAT
	memcpy(ucFATCopy, ucFAT, iFATSize*512);

	// test FAT integrity
	AddStatus("OK.\nChecking FAT integrity: ");
	iResult = 0;
	for(i=0; i<iFATSize; i++)
	{
		if((ucFAT[i*512+510]!='F')||(ucFAT[i*512+511]!='B'))
		{
			csTemp.Format(
				"FAT block %i has not the correct signature.\n"
				"This is a serious problem and can happen if this\n"
				"drive was formatted using Chickensys Translator.\n"
				"In this case the FAT is probably too small to hold\n"
				"all entries necessary for this drive.\n\n"
				"The problem is that if you fill up your drive with\n"
				"data, at some point the writing operations to the\n"
				"FAT overwrite your first files and directories on\n"
				"that disk. You will lose data (maybe all data).\n\n"
				"The only solution at the moment is: Create an ISO\n"
				"backup using ETools, re-format the drive with ETools\n"
				"and mount the ISO image with EnsoniqFS within Total\n"
				"Commander. Then copy the files from the ISO image\n"
				"to the newly formatted drive (using Total Commander).\n\n",
				"ETools will now try to continue with the assumption\n"
				"that the FAT has a smaller size. If any errors are\n"
				"found (especially \"lost blocks\"), please DO NOT try\n"
				"to fix them until you created a backup. ETools may\n"
				"unintentionally destroy your disk content while trying\n"
				"to repair the disk using this wrong FAT.",
				i);
			MessageBox(csTemp, "Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION);

			iFATSize = i; iResult = 1;
			m_iMaxLogicalBlocks = iFATSize * 170;
			break;
		}
	}
	if(iResult)
	{
		AddStatus("failed.\n");
	}
	else
	{
		AddStatus("OK.\n");
	}

	//........................................................................
	// lost block search
	//........................................................................

	// mark all used blocks recursively starting with
	// block 3 (root directory)
	m_iFiles = 0; m_iDirectories = 1;
	AddStatus("Reading file structure: ROOT");
	if(ERR_OK!=MarkUsedBlocks(pDisk, 3, ucFATCopy))
	{
		AddStatus("?Reading file structure: failed.\n");
		if(MessageBox("There was an error during check for lost blocks.\n"
				"\nContinue testing?",
				"Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_YESNO)==IDNO)
		{
			free(ucFAT); free(ucFATCopy);
			return;
		}
	}

	csTemp.Format("?Reading file structure: OK, %i files in %i "
		"directories.\n", m_iFiles, m_iDirectories); AddStatus(csTemp);
	AddStatus("Checking for lost blocks: ");

	// mark DeviceID block, OS block, FAT, root as used
	for(i=0; i<iFATSize + 5; i++) SetFATEntryRAM(ucFATCopy, i, 0xFFFFFF);
	
	// search for lost blocks
	iLostBlocks = 0; iLostChains = 0; iLastBlock = -1;
	for(i=0; i<m_iMaxLogicalBlocks; i++)
	{
		iBlock = GetFATEntryRAM(ucFATCopy, i);
		if((0xFFFFFF!=iBlock)&&(0x000000!=iBlock)&&(0x000002!=iBlock))
		{
			if(i!=iLastBlock)
			{
//				TRACE("---------new chain---------\n");
				iLostChains++;
			}

//			TRACE("Lost block %i=%i\n", i, iBlock);
			iLastBlock = iBlock;
			iLostBlocks++;
		}
	}

	// check result
	if(0==iLostBlocks)
	{
		AddStatus("OK\n");
	}
	else
	{
		csTemp.Format("%i lost blocks found, %i contiguous chains. ",
			iLostBlocks, iLostChains);
		AddStatus(csTemp);

		csTemp.Format(
			"%i lost blocks were found in %i contiguous chains. These\n"
			"are blocks which are marked as used in the FAT but are not\n"
			"linked to any file in any directory.\n\n"
			"This normally happens during incomplete file write operations."
			"\nRepairing this problem means to delete these blocks. You\n"
			"will be asked whether to save these files to PC files.\n\n"
			"Do you want to repair this?", iLostBlocks, iLostChains);
		if(MessageBox(csTemp, "Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_YESNO)==IDYES)
		{
			if(MessageBox(
				"Do you want to save the lost blocks to PC files?",
				"Ensoniq Filesystem Tools", 
				MB_ICONQUESTION | MB_YESNO)==IDYES)
			{
				// save lost blocks
				// show "Save as..." dialog
				static char BASED_CODE szFilter[] = 
					"Binary files (*.bin)|*.bin|All Files (*.*)|*.*||";
				csFN = "c:\\file00001.bin";
				CFileDialog *dlg = new CFileDialog(FALSE, "bin", csFN, 
					0, szFilter, this);
				if(NULL!=dlg)
				{
					char cTitle[]="Save lost blocks in directory"
						" (filename will be ignored)";
					dlg->m_ofn.lpstrTitle = cTitle;
					if(IDCANCEL!=dlg->DoModal())
					{
						csFN = dlg->GetPathName();
					}
					else
					{
						// use this as "cancel" flag
						csFN = "!";
						AddStatus("Cancelled.\n");
					}
				}

				delete dlg;

				// file open dialog cancelled?
				if(csFN=="!") csPath = "!";
				else
				{
					i = csFN.ReverseFind('\\');
					if(i!=-1) csPath = csFN.Left(i+1);
					else csPath = "C:\\";
				}
			
			}
			else
			{
				// use this as "do not save files" flag
				csPath = "?";
			}

			iLostChains = 1; iLastBlock = -1; i = 0;
			if(csPath!="!") for(i=0; i<m_iMaxLogicalBlocks; i++)
			{
				if(0==(i%100))
					m_ctlProgress.SetPos(i*100/m_iMaxLogicalBlocks);

				iBlock = GetFATEntryRAM(ucFATCopy, i);
				if((0xFFFFFF!=iBlock)&&(0!=iBlock)&&(2!=iBlock))
				{
					// new chain?
					if((i!=iLastBlock)&&(csPath!="?"))
					{
						// create new filename
						do
						{
							csFN.Format("%sfile%05i.bin", csPath, 
								iLostChains++);
						} while (0==_access(csFN, 0));

						// close old file
						if(NULL!=f) fclose(f);

						// open new file
						f = fopen(csFN, "wb");
						iLostChains++;
					}
					iLastBlock = iBlock;

					if(NULL!=f)
					{
						memset(ucBuf, 0, 512);
						ReadBlocks(pDisk, i, 1, ucBuf);
						fwrite(ucBuf, 1, 512, f);
					}

					// update FAT
					if(-1==SetFATEntry(pDisk, i, 0))
					{
						MessageBox("Error writing FAT entry.", 
							"Ensoniq Filesystem Tools · Warning", 
							MB_ICONEXCLAMATION | MB_OK);
						break;
					}
					SetFATEntryRAM(ucFATCopy, i, 0);
					SetFATEntryRAM(ucFAT, i, 0);
				}
			}
			m_ctlProgress.SetPos(100);

			if(NULL!=f)
			{
				fclose(f); f = NULL;
			}
			if(i==m_iMaxLogicalBlocks) AddStatus("Errors corrected.\n");
		}
		else
		{
			AddStatus("Errors ignored.\n");
		}
	}

	CacheFlush(pDisk);

	//........................................................................
	// check parent links
	//........................................................................
	AddStatus("Checking parent directory links: "); m_iTempDirectories = 0;
	iResult = CheckParentLinks(pDisk, FALSE, 3, 0, "");
	AddStatus("?Checking parent directory links: ");
	if(ERR_OK!=iResult)
	{
		csTemp.Format(
			"Problems were found in the linking of directories to their "
			"parent directories.\n"
			"Should this be corrected?");
		if(MessageBox(csTemp, "Ensoniq Filesystem Tools · Warning", 
				MB_ICONEXCLAMATION | MB_YESNO)==IDYES)
		{
			m_iTempDirectories = 0;
			iResult = CheckParentLinks(pDisk, TRUE, 3, 0, "");
			if(ERR_OK==iResult)
			{
				AddStatus("?Checking parent directory links: "
					"Error was corrected.\n");
			}
			else
			{
				AddStatus("?Checking parent directory links: "
					"Error could not be corrected.\n");
			}
		}
		else
		{
			AddStatus("Error was ignored.\n");
		}

	}
	else
	{
		AddStatus("OK.\n");
	}


	//........................................................................
	// re-calculate free space
	//........................................................................
	AddStatus("Checking free space: ");

	// count free blocks
	iFreeBlocks = 0;
	for(i=0; i<m_iMaxLogicalBlocks; i++)
	{
		if(0==GetFATEntryRAM(ucFAT, i)) iFreeBlocks++;
	}

	iFreeBlocksOS = (ucOSBlock[0]<<24)
				  + (ucOSBlock[1]<<16)
				  + (ucOSBlock[2]<<8)
				  + (ucOSBlock[3]);

	if(iFreeBlocks==iFreeBlocksOS)
	{
		csTemp.Format("%i blocks free.\n", iFreeBlocks);
		AddStatus(csTemp);
	}
	else
	{
		csTemp.Format("%i blocks (FAT), %i blocks (OS block, wrong).\n",
			iFreeBlocks, iFreeBlocksOS);
		AddStatus(csTemp);

		if(IDYES==MessageBox(
			"The free space calculated from the FAT is not equal to the\n"
			"free space reported by the OS block. This is not a serious\n"
			"problem.\n\n"
			"Should this be corrected?",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_YESNO))
		{
			ucOSBlock[0] = (iFreeBlocks>>24) & 0xFF;
			ucOSBlock[1] = (iFreeBlocks>>16) & 0xFF;
			ucOSBlock[2] = (iFreeBlocks>>8)  & 0xFF;
			ucOSBlock[3] = (iFreeBlocks)     & 0xFF;


			if(ERR_OK!=WriteBlocks(pDisk, 2, 1, ucOSBlock))
			{
				AddStatus("Unable to write corrected OS block.\n");

				if(MessageBox("Could not write corrected OS block.\n"
						"\nContinue testing?",
						"Ensoniq Filesystem Tools · Warning", 
						MB_ICONEXCLAMATION | MB_YESNO)==IDNO)
				{
					free(ucFAT); free(ucFATCopy);
					return;
				}
			}
			else
			{
				AddStatus("OS block was corrected.\n");
			}
		}
	}

	CacheFlush(pDisk);
	
	free(ucFAT); free(ucFATCopy);
	AddStatus("\nDisk check complete.\n");
}

//----------------------------------------------------------------------------
// OnButtonFormat
//----------------------------------------------------------------------------
void CEToolsDlg::OnButtonFormat() 
{
	CFormatDlg *dlg;

	CString csDevice, csTemp;
	DISK *pDisk = NULL;
	int i;

	// check function pointers
	if(NULL==this->ReadBlocks) return;
	if(NULL==this->WriteBlocks) return;
	if(NULL==this->CacheFlush) return;

	//........................................................................
	// init
	//........................................................................

	// get device name
	m_ctlComboDevices.GetWindowText(csDevice);
	if(""==csDevice)
	{
		MessageBox("Please select a device in the device list above.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	csTemp = "";
	for(i=0; i<csDevice.GetLength(); i++)
	{
		// look for closing ':'
		if(':'==csDevice[i])
		{
			if(5==i)
			{
				// ignore ':' at position if floppy "A:" or "B:"
				if(('A'!=csDevice[i-1])&&('B'!=csDevice[i-1])) break;
			}
			else if(11==i)
			{
				// ignore ':' at position 11 if this is an image file
				if(0!=strncmp(csDevice, "\\\\.\\image=", 10)) break;
			}
			else break;
		}
		csTemp += csDevice[i];
	}
	csDevice = csTemp;

	pDisk = m_pDiskList;
	while(pDisk)
	{
		if(csTemp == pDisk->cMsDosName)
		{
			break;
		}
		pDisk = pDisk->pNext;
	}

	if(NULL==pDisk)
	{
		m_ctlStaticStatus.SetWindowText("Could not find device.");
		MessageBox("Could not find selected device.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if((0==strcmp(pDisk->cMsDosName, "\\\\.\\A:"))||
		(0==strcmp(pDisk->cMsDosName, "\\\\.\\B:")))
	{
		MessageBox("Currently it is not possible to format floppy disks "
			"with\nEnsoniq Filesystem Tools. Please use your sampler to do\n"
			"so. This feature will be added in a future verion.",
			"Ensoniq Filesystem Tools · Warning", 
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// ask user if this is not an Ensoniq drive
	if(0==pDisk->iIsEnsoniq)
	{
		if(IDNO==MessageBox(
			"Read the following text carefully:\n\n"
			"The disk you selected does not yet contain an Ensoniq file\n"
			"system signature. This possibly means that you try to format\n"
			"a drive which contains your Windows operating system or other\n"
			"important data. Please double check that you REALLY want to\n"
			"format this drive, otherwise you may lose your Windows\n"
			"installation or important user data or both!\n\n"
			"\nDo you really want to continue? (You have been warned!)",
			"Ensoniq Filesystem Tools · Warning",
			MB_YESNO|MB_ICONWARNING)) return;
	}

	dlg = new CFormatDlg;
	dlg->SetParent(this, pDisk);
	dlg->DoModal();
	delete dlg;

	UpdateDeviceDropdown();
}

//----------------------------------------------------------------------------
// DestroyWindow
//----------------------------------------------------------------------------
BOOL CEToolsDlg::DestroyWindow() 
{
	if(NULL!=m_pDiskList) FreeDiskList(0, m_pDiskList);
	if(NULL!=m_hDLL)
	{
		FreeLibrary(m_hDLL);
		m_hDLL = NULL;
	}

	return CDialog::DestroyWindow();
}

//----------------------------------------------------------------------------
// OnDropdownComboDevices
//----------------------------------------------------------------------------
void CEToolsDlg::OnDropdownComboDevices() 
{
	UpdateDeviceDropdown();
}

//----------------------------------------------------------------------------
// UpdateDeviceDropdown
//----------------------------------------------------------------------------
void CEToolsDlg::UpdateDeviceDropdown()
{
	LOG("--------------------------------------------------------------\n");
	LOG("UpdateDeviceDropdown(): begin\n");

	CString csDiskName, csDiskLabel, csDiskSize, csPartitionSize, csName;
	DISK *pDisk;

	if(NULL==this->ScanDevices) return;
	if(NULL==this->FreeDiskList) return;
	if(NULL==this->GetUsageCount) return;

	// free disk list
	if(NULL!=m_pDiskList)
	{
		LOG("Freeing disk list: ");
		FreeDiskList(0, m_pDiskList);
		LOG("OK.\n");
	}
	m_pDiskList = NULL;

	// check if EnsoniqFS has locked devices
	if(GetUsageCount()>1)
	{
		MessageBox("It seems that you started Ensoniq Filesystem Tools "
			"from the\nStart menu while Total Commander is using the "
			"EnsoniqFS\nplugin. Therefore almost all devices containing "
			"Ensoniq data\nwill be locked by Total Commander and you will "
			"not be able\nto access them here.\n\n"
			"To avoid this warning,\n"
			"(a) quit Total Commander before running Ensoniq Filesystem Tools\n"
			"  or\n"
			"(b) call Ensoniq Filesystem Tools from within Total Commander by\n"
			"navigating to the \"Ensoniq filesystems\" folder and opening the\n"
			"entry named \"Run Ensoniq Filesystem Tools\".",
			"Ensoniq Filesystem Tools", MB_ICONEXCLAMATION | MB_OK);
	}

	// scan for devices
	LOG("ScanDevices(): ");
	m_pDiskList = this->ScanDevices(1);
	LOG("OK, ");
	if(NULL==m_pDiskList)
	{
		LOG("result=NULL.\n");
		MessageBox("An error occured during device scan. No devices were "
			"found.", "Ensoniq Filesystem Tools", MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	LOG("result!=NULL.\n");

	LOG("Combo.ResetContent(): ");
	m_ctlComboDevices.ResetContent();
	LOG("OK.\n");

	pDisk = m_pDiskList;
	while(pDisk)
	{
		LOG("Looping through disk list: next item.\nGetting disk label: ");

		// get disk name
		csName.Format("%s", pDisk->cMsDosName);
		if(csName.GetLength()>63)
		{
			csName = csName.Left(30) + "..." + csName.Right(30);
		}

		// get disk label
		if(pDisk->iIsEnsoniq)
		{
			csDiskLabel.Format("disk label=\"%s\"", pDisk->cDiskLabel);
		}
		else
		{
			csDiskLabel = "not Ensoniq formatted";
		}

		LOG("OK.\nCalculating disk size: ");

		// calculate disk size
		csDiskSize = 
			CapacityString((int)(pDisk->DiskGeometry.DiskSize.QuadPart/1024));
		csPartitionSize = CapacityString((int)(pDisk->dwBlocks)/2);
		csDiskName.Format("%s: %s (%s disk, %s partition)", 
			csName, csDiskLabel, csDiskSize, csPartitionSize);

		LOG("OK.\nAdding to list ("); LOG(csDiskName);
		LOG("): ");

		m_ctlComboDevices.AddString(csDiskName);
		LOG("OK. Taking next disk: ");
		pDisk = pDisk->pNext;
		LOG("OK.\n");
	}
	LOG("UpdateDeviceDropdown(): finished\n");
}

//----------------------------------------------------------------------------
// Pump
//
// Retrieve and dispatch any waiting messages.
//----------------------------------------------------------------------------
BOOL CEToolsDlg::Pump()
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
// OnButtonCancel
//----------------------------------------------------------------------------
void CEToolsDlg::OnButtonCancel() 
{
	m_bCancel = TRUE;
}

//----------------------------------------------------------------------------
// ShowCancel
//----------------------------------------------------------------------------
void CEToolsDlg::ShowCancel(BOOL bShow)
{
	if(bShow)
	{
		m_ctlButtonCancel.ShowWindow(SW_SHOW);
		m_ctlButtonBackup.EnableWindow(FALSE);
		m_ctlButtonRestore.EnableWindow(FALSE);
		m_ctlButtonFormat.EnableWindow(FALSE);
		m_ctlButtonCheckFS.EnableWindow(FALSE);
		m_ctlButtonExit.EnableWindow(FALSE);
		m_ctlComboDevices.EnableWindow(FALSE);
	}
	else
	{
		m_ctlButtonCancel.ShowWindow(SW_HIDE);
		m_ctlButtonBackup.EnableWindow(TRUE);
		m_ctlButtonRestore.EnableWindow(TRUE);
		m_ctlButtonFormat.EnableWindow(TRUE);
		m_ctlButtonCheckFS.EnableWindow(TRUE);
		m_ctlButtonExit.EnableWindow(TRUE);
		m_ctlComboDevices.EnableWindow(TRUE);
	}
}

//----------------------------------------------------------------------------
// AddStatus
// 
// Adds a line to the status field
// If csStatus starts with "?", the last line of the status field is
// exchanged with csStatus
// 
// -> csStatus = line to add
// <- --
//----------------------------------------------------------------------------
void CEToolsDlg::AddStatus(CString csStatus)
{
	CString csTemp;
	int i;

	// get old status text
	m_ctlStaticStatus.GetWindowText(csTemp);

	// modify
	if("?"==csStatus.Left(1))
	{
		if("\n"==csTemp.Right(1)) csTemp=csTemp.Left(csTemp.GetLength()-1);
		i = csTemp.ReverseFind('\n'); if(-1==i) i=0;
		csTemp = csTemp.Left(i+1);
		csTemp = csTemp + csStatus.Right(csStatus.GetLength()-1);
	}
	else
	{
		csTemp = csTemp + csStatus;
	}

	// set new status text
	m_ctlStaticStatus.SetWindowText(csTemp);
	Pump();
}

//----------------------------------------------------------------------------
// GetFATEntryRAM
// 
// Reads a FAT entry out of loaded FAT (in RAM)
// 
// -> ucFAT = pointer to FAT in RAM
//    iBlock = number of FAT entry to read
// <- content of FAT entry
//----------------------------------------------------------------------------
int CEToolsDlg::GetFATEntryRAM(unsigned char *ucFAT, int iBlock)
{
	int i;

	if(iBlock>=m_iMaxLogicalBlocks) return 0;
	if(iBlock<0) return 0;

	// read FAT entry
	i = ((iBlock/170)*512) + (iBlock%170)*3;
	return (ucFAT[i+0]<<16) + (ucFAT[i+1]<<8) + ucFAT[i+2];
}

//----------------------------------------------------------------------------
// SetFATEntryRAM
// 
// Writes a FAT entry into loaded FAT (in RAM)
// 
// -> ucFAT = pointer to FAT in RAM
//    iBlock = number of FAT entry to write
//    iNewValue = new value for FAT entry
// <- old value of FAT entry
//----------------------------------------------------------------------------
int CEToolsDlg::SetFATEntryRAM(unsigned char *ucFAT, int iBlock, 
							   int iNewValue)
{
	int i, j;
	
	if(iBlock>=m_iMaxLogicalBlocks) return 0;
	if(iBlock<0) return 0;

	// read FAT entry
	i = ((iBlock/170)*512) + (iBlock%170)*3;
	j = (ucFAT[i+0]<<16) + (ucFAT[i+1]<<8) + ucFAT[i+2];

	// write new value
	ucFAT[i+0] = (iNewValue>>16) & 0xFF;
	ucFAT[i+1] = (iNewValue>>8)  & 0xFF;
	ucFAT[i+2] = (iNewValue)     & 0xFF;

	// return old value
	return j;
}

//----------------------------------------------------------------------------
// MarkUsedBlocks
// 
// Follows the given directory recursively, marks every found file and
// directory in the FAT with entry 0xFFFFFF
// 
// -> pDisk = pointer to legal disk structure
//    iDirectoryBlock = starting block for directory
//    ucFAT = pointer to FAT in RAM (will be modified)
// <- ERR_OK
//    ERR_READ
//----------------------------------------------------------------------------
int CEToolsDlg::MarkUsedBlocks(DISK *pDisk, int iDirectoryBlock, 
							   unsigned char *ucFAT)
{
	unsigned char ucBuf[1024];
	int iResult, i, j, iBlock;
	CString csDir;

	// mark this directory as used
	SetFATEntryRAM(ucFAT, iDirectoryBlock, 0xFFFFFF);
	SetFATEntryRAM(ucFAT, iDirectoryBlock+1, 0xFFFFFF);

	// read current directory
	iResult = ReadBlocks(pDisk, iDirectoryBlock, 2, ucBuf);
	if(ERR_OK!=iResult) return iResult;

	for(i=0; i<39; i++)
	{
		// skip empty entries, skip link to parent directory
		if(0x00==ucBuf[i*26+1]) continue;
		if(0x08==ucBuf[i*26+1]) continue;

		// calculate starting block
		iBlock = (ucBuf[i*26+18]<<24) +
				 (ucBuf[i*26+19]<<16) +
				 (ucBuf[i*26+20]<<8)  +
				 (ucBuf[i*26+21]);

		
		// is this a subdirectory?
		if(0x02==ucBuf[i*26+1])
		{
			// go to next directory recursively
			m_iDirectories++; csDir = "?Reading file structure: ";
			for(j=0; j<12; j++) csDir += ucBuf[i*26+2+j];
			AddStatus(csDir);
			MarkUsedBlocks(pDisk, iBlock, ucFAT);
		}
		else // normal file
		{
			// mark this file
			m_iFiles++;
			while((1!=iBlock)&&(0!=iBlock)&&(2!=iBlock)&&(0xFFFFFF!=iBlock))
			{
				iBlock = SetFATEntryRAM(ucFAT, iBlock, 0xFFFFFF);
			}
		}
	}

	return ERR_OK;
}

//----------------------------------------------------------------------------
// CheckParentLinks
// 
// Checks every directory if the parent link is valid, repairs if necessary
// (recursively)
// 
// -> pDisk = pointer to disk to be checked
//    bRepair = TRUE: repair pointers
//              FALSE: do only checking
//    iDir = directory block to check (recursively), root = block 3
//    iParent = parent directory block
// <- ERR_OK
//    ERR_READ
//    ERR_WRITE
//----------------------------------------------------------------------------
int CEToolsDlg::CheckParentLinks(DISK *pDisk, BOOL bRepair, int iDir, 
								 int iParentDir, CString csDirName)
{
	unsigned char ucBuf[1024];
	int iResult, i, j, iBlock, iEntry;
	CString csDir, csTemp;
	BOOL bParentLinkFound = FALSE, bErrorFound = FALSE;

	// read current directory
	iResult = ReadBlocks(pDisk, iDir, 2, ucBuf);
	if(ERR_OK!=iResult) return iResult;

	m_iTempDirectories++; 
	m_ctlProgress.SetPos(m_iTempDirectories*100/m_iDirectories);

	for(i=0; i<39; i++)
	{
		// skip empty entries
		if(0x00==ucBuf[i*26+1]) continue;

		// calculate starting block
		iBlock = (ucBuf[i*26+18]<<24) +
				 (ucBuf[i*26+19]<<16) +
				 (ucBuf[i*26+20]<<8)  +
				 (ucBuf[i*26+21]);

		// check this parent link
		if(0x08==ucBuf[i*26+1])
		{
			bParentLinkFound = TRUE;
			if(iParentDir!=iBlock)
			{
				TRACE("Directory %s (%i) has invalid parent link (%i!=%i).\n",
					csDirName, iDir, iParentDir, iBlock);

				if(TRUE==bRepair)
				{
					AddStatus("?Checking parent directory links: Repairing"
						+ csDirName);

					ucBuf[i*26+18] = (iParentDir>>24)&0xFF;
					ucBuf[i*26+19] = (iParentDir>>16)&0xFF;
					ucBuf[i*26+20] = (iParentDir>>8) &0xFF;
					ucBuf[i*26+21] = (iParentDir)    &0xFF;

					// write back new entry
					iResult = WriteBlocks(pDisk, iDir, 2, ucBuf);
					CacheFlush(pDisk);
					if(iResult!=ERR_OK) return ERR_WRITE;
				}
				else
				{
					bErrorFound = TRUE;
				}
			}
			continue;
		}

		// is this a subdirectory?
		if(0x02==ucBuf[i*26+1])
		{
			// go to next directory recursively
			csDir = ""; for(j=0; j<12; j++) csDir += ucBuf[i*26+2+j];
			AddStatus("?Checking parent directory links: " + csDir);
			csDir.TrimRight(" "); csTemp = csDirName + "/" + csDir;

			// recursively check next directory
			iResult = CheckParentLinks(pDisk, bRepair, iBlock, iDir, csTemp);
			if((ERR_OK!=iResult)&&(ERR_INVALID_PARENT_LINKS!=iResult))
				return iResult;
			if(ERR_INVALID_PARENT_LINKS==iResult) bErrorFound = TRUE;
		}
	}

	// check results
	if(FALSE==bParentLinkFound)
	{
		// root does not need to have a parent
		if(3==iDir)
		{
			bParentLinkFound = TRUE;
		}
		else
		{
			TRACE("Dir %s (%i) has no parent.\n", csDirName, iDir);
			
			// try to repair
			if(TRUE==bRepair)
			{
				TRACE("Repairing.\n");

				// first entry free?
				iEntry = -1;
				if(0x00==ucBuf[1])
				{
					iEntry = 0;
				}
				else
				{
					// search for free entry
					for(i=0; i<39; i++)
					{
						if(0x00==ucBuf[i*26+1])
						{
							iEntry = i;
							break;
						}
					}
				}
				// swap first entry with free entry
				if(-1!=iEntry)
				{
					AddStatus("?Checking parent directory links: Repairing"
						+ csDirName);

					// copy first entry to new position
					memcpy(ucBuf + iEntry*26, ucBuf, 26);

					// construct a new first entry
					memset(ucBuf, 0, 26);
					ucBuf[ 1] = 0x08; // parent dir pointer
					csTemp = "PARENTDIR   ";
					for(j=0; j<12; j++) ucBuf[2+j] = csTemp[j];
					ucBuf[14] = 0x00;
					ucBuf[15] = 0x00; // file size
					ucBuf[16] = 0x00;
					ucBuf[17] = 0x02; // contiguous blocks
					ucBuf[18] = (iParentDir>>24)&0xFF;
					ucBuf[19] = (iParentDir>>16)&0xFF;
					ucBuf[20] = (iParentDir>>8) &0xFF;
					ucBuf[21] = (iParentDir)    &0xFF;
					
					// write back new entry
					iResult = WriteBlocks(pDisk, iDir, 2, ucBuf);
					CacheFlush(pDisk);
					if(iResult!=ERR_OK) return ERR_WRITE;
					bParentLinkFound = TRUE;
				}
				else
				{
					csTemp.Format("The directory \"%\" is full.\n\n"
						"Please delete at least one file from this "
						"directory and re-run this check.", csDirName);
					MessageBox(csTemp,
						"Ensoniq Filesystem Tools · Warning", 
						MB_ICONEXCLAMATION | MB_OK);
				}
			}
		}
	}

	if((FALSE==bParentLinkFound)||(TRUE==bErrorFound)) 
		return ERR_INVALID_PARENT_LINKS;

	return ERR_OK;
}

//----------------------------------------------------------------------------
// SearchRecursive
// 
// Searches for a specific file in the given directory
// This function calls itself recursively for all subdirectories
// 
// -> csWhat = file to find
//    csWhere = starting directory
// <- "": file not found
//    else: complete path and filename
//----------------------------------------------------------------------------
CString CEToolsDlg::SearchRecursive(CString csWhat, CString csWhere)
{
	CString csName, csFullName;
	_finddata_t FindData;
	long lFindHandle;
	int iResult = 0;

	// add trailing backslash if necessary
	if(csWhere.Right(1)!="\\") csWhere += "\\";

	// convert to lowercase because (a) Windows ignores case and (b) possibly
	// the file we look for was stored in different case
	csWhat.MakeLower(); csWhere.MakeLower();

	// add wildcard
	csWhere += "*";

	// start search
	lFindHandle = _findfirst(csWhere, &FindData);
	if(-1==lFindHandle) return "";

	// loop through all results
	while(-1!=iResult)
	{
		// get filename out of find structure, convert name
		csName = FindData.name;
		csFullName = csWhere.Left(csWhere.GetLength()-1) + csName;
		csName.MakeLower(); csFullName.MakeLower();

		// dig into subdirectory if needed
		if((0!=(FindData.attrib&_A_SUBDIR))&&("."!=csName)&&(".."!=csName))
		{
			// scan subdirectory
			csFullName = SearchRecursive(csWhat, csFullName);
			if(""!=csFullName)
			{
				// return successfully
				_findclose(lFindHandle);
				return csFullName;
			}
		}
		else
		{
			// found file?
			if(csName==csWhat)
			{
				// close find handle
				_findclose(lFindHandle);

				// return path without wildcard + filename
				return csFullName;
			}
		}

		// get next result
		iResult = _findnext(lFindHandle, &FindData);
	}

	// close find handle
	_findclose(lFindHandle);

	// return "not found"
	return "";
}

//----------------------------------------------------------------------------
// SearchEnsoniqFsWfx
// 
// Locate TotalCommander directory and build up path to EnsoniqFS.wfx
// 
// -> --
// <- Path to EnsoniqFS (including file name) or "" if failed
//----------------------------------------------------------------------------
CString CEToolsDlg::SearchEnsoniqFsWfx()
{
	// TODO: rekursive Suche in TC-Verzeichnis
	unsigned long ulType, ulDataSize = 0;
	CString csPath, csText;
	char *cData = NULL;
	HKEY h;
	DWORD dwError;
	LONG lReturn;

	// open registry entry
	if(RegOpenKeyEx(HKEY_CURRENT_USER, 
		"Software\\Ghisler\\Total Commander", 0, 
		KEY_QUERY_VALUE, &h)!=ERROR_SUCCESS)
	{
		dwError = GetLastError();
		csText.Format("Could not open registry entry for TotalCommander "
			"installation.\n"
			"Please make sure you have a working copy of TC installed.\n\n"
			"Extended error information:\n"
			"Error code = 0x%08X\nMessage = %s", dwError, 
			GetErrorText(dwError));

		MessageBox(csText, "Ensoniq Filesystem Tools · Warning",
			MB_ICONEXCLAMATION | MB_OK);
		return "";
	}

	// query buffer length, create buffer
	lReturn = RegQueryValueEx(h, "InstallDir", 0, &ulType, 
		(unsigned char*)cData, &ulDataSize);

	cData = new char[ulDataSize+1];
	if(NULL==cData)
	{
		csText.Format("Could not allocate memory (%i bytes) for path "
			"information.\n", ulDataSize);
		MessageBox(csText, "Ensoniq Filesystem Tools · Error",
			MB_ICONSTOP | MB_OK);
		return "";
	}

	// read path
	lReturn = RegQueryValueEx(h, "InstallDir", 0, &ulType, 
		(unsigned char*)cData, &ulDataSize);

	if(lReturn!=ERROR_SUCCESS)
	{
		csText.Format("Could not query registry entry for TotalCommander "
			"installation.\n"
			"Please make sure you have a working copy of TC installed.\n\n"
			"Extended error information:\n"
			"Error code = 0x%08X\nMessage = %s", lReturn, 
			GetErrorText(lReturn));

		MessageBox(csText, "Ensoniq Filesystem Tools · Warning",
			MB_ICONEXCLAMATION | MB_OK);

		delete cData;
		RegCloseKey(h);
		return "";
	}

	csPath = cData;
	delete cData;
	RegCloseKey(h);

	// search for EnsoniqFS.wfx in TC installation directory recursively
	csPath = SearchRecursive("EnsoniqFS.wfx", csPath);

	return csPath;
}

//----------------------------------------------------------------------------
// GetErrorText
// 
// Translates an error code from GetLastError() to its textual form
// 
// -> dwError = code from GetLastError()
// <- string describing the error
//----------------------------------------------------------------------------
CString CEToolsDlg::GetErrorText(DWORD dwError)
{
	CString csErr;
	char *lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL);
	csErr = lpMsgBuf;
	LocalFree(lpMsgBuf);

	return csErr;
}

//----------------------------------------------------------------------------
// CapacityString
// 
// Formats a string to contain a capacity value (kB, MB, GB)
// 
// -> iDiskCapacity = capacity in kBytes
// <- formatted string
//----------------------------------------------------------------------------
CString CEToolsDlg::CapacityString(int iDiskCapacity)
{
	CString csDiskSize;
	double dDiskSize = iDiskCapacity;

	if(dDiskSize>1024)
	{
		dDiskSize /= 1024;
		if(dDiskSize>1024)
		{
			dDiskSize /= 1024;
			csDiskSize.Format("%.1f GB", dDiskSize);
		}
		else
		{
			csDiskSize.Format("%.1f MB", dDiskSize);
		}
	}
	else
	{
		csDiskSize.Format("%.1f kB", dDiskSize);
	}

	return csDiskSize;
}

/*
TODO:

filesystem check:
- "repair" too small FAT issue by changing logical disk size
- check every file's FAT chain if it contains a "free" (=0x00) block
  (must be terminated with EOF (0x01)) and if every part of the chain
  is within logical disk space
- check if file size from directory entry is equal to file size from FAT
- check every directory entry to point to a valid FAT entry (this means
  =0x01, !=0x00, !=0x02, <=last block on disk)
- search for cross-linked files, display them, resolve them
- check "size" argument of every directory (should represent the number
  of entries contained in this directory)
- disable buttons during file system check, activate "cancel", check for
  cancellation

backup/restore:
- introduce option to create backup from (a) full disk (physical size),
  (b) full disk (logical partition size), (c) used space only (following
  FAT -> special backup format needed, ISO not suitable)

format:
- disable buttons during format, activate "cancel", check for cancellation
- formatting floppy disks using OmniFlop
- create default directories after formatting
- copy OS after formatting

general:
- allow to change disk name

resolved:
- check if every subdirectory points back to parent
- find source of exceptions during device scan
- make backup/restore safe against 2gig/4gig border
*/
