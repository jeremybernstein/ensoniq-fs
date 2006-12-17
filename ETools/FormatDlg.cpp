// FormatDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "ETools.h"
#include "FormatDlg.h"
#include "../EnsoniqFS/error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialog CFormatDlg 
//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CFormatDlg::CFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFormatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFormatDlg)
	m_ctlCheckQuickformat = FALSE;
	//}}AFX_DATA_INIT
}


//----------------------------------------------------------------------------
// DataExchange
//----------------------------------------------------------------------------
void CFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormatDlg)
	DDX_Control(pDX, IDC_STATIC_MAX_CAPACITY, m_ctlStaticMaxCapacity);
	DDX_Control(pDX, IDC_COMBO_UNIT, m_ctlComboUnit);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctlStaticStatus);
	DDX_Control(pDX, IDC_PROGRESS_FORMAT, m_ctlProgress);
	DDX_Control(pDX, IDC_EDIT_DISK_LABEL, m_ctlEditDiskLabel);
	DDX_Control(pDX, IDC_EDIT_CAPACITY, m_ctlEditCapacity);
	DDX_Check(pDX, IDC_CHECK_QUICK, m_ctlCheckQuickformat);
	//}}AFX_DATA_MAP
}


//----------------------------------------------------------------------------
// MESSAGE_MAP
//----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CFormatDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIT, OnSelchangeComboUnit)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//----------------------------------------------------------------------------
// OnOK
//----------------------------------------------------------------------------
void CFormatDlg::OnOK() 
{
	int i, j, iFATSize, iBlocksOnDisk, iFreeBlocks, iResult;
	unsigned char ucDiskHeader[5*512], *ucFAT;
	CString csText, csDiskSize, csSize;
	__int64 iiBytes, iiPhysical;
	double dSize, dDiskSize;
	char cSize[128];

	// convert current value to <bytes>
	m_ctlEditCapacity.GetWindowText(cSize, 127);
	dSize = atof(cSize);
	switch(m_iLastUnit)
	{
		case UNIT_BYTE:
			break;
		case UNIT_KB:
			dSize *= 1024; break;
		case UNIT_MB:
			dSize *= 1024*1024; break;
		case UNIT_GB:
			dSize *= 1024*1024*1024; break;
		default:
			break;
	}
	
	// calculate disk statistics
	iiBytes = (__int64)dSize;
	iiBytes -= iiBytes % 512;	// round off to sector size
	iBlocksOnDisk = (int)((__int64)iiBytes >> (__int64)9);
	iFATSize = iBlocksOnDisk/170; if(iBlocksOnDisk%170) iFATSize++;
	iFreeBlocks = iBlocksOnDisk - iFATSize - 5;

	// calculate physical disk size
	iiPhysical = m_pDisk->DiskGeometry.DiskSize.QuadPart;

	dDiskSize = (int)(iiPhysical/1024);
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

	// calculate logical disk size
	dSize /= 1024;
	if(dSize>1024)
	{
		dSize /= 1024;
		if(dSize>1024)
		{
			dSize /= 1024;
			csSize.Format("%.1f GB", dSize);
		}
		else
		{
			csSize.Format("%.1f MB", dSize);
		}
	}
	else
	{
		csSize.Format("%.1f kB", dSize);
	}

	// check size
	if(iiBytes>iiPhysical)
	{
		csText.Format("The capacity value must not be greater than the "
			"maximum disk capacity\n(which is %s).", csDiskSize);
		MessageBox(csText, "Ensoniq Filesystem Tools · Warning", 
			MB_ICONWARNING);
		return;
	}

	// last warning
	csText.Format("Do you really want to format this disk? All "
		"data on this\ndisk will be overwritten and there is no way back!\n"
		"\nThe disk you have chosen is %s.\nIt will be formatted to %s."
		"\n\nThis is your last chance to cancel this process.\n\nContinue?",
		csDiskSize, csSize);
	if(IDNO==MessageBox(csText, "Ensoniq Filesystem Tools · Warning",
		MB_YESNO|MB_ICONWARNING)) return;

	// allocate FAT
	SetStatus("Preparing FAT...");
	ucFAT = (unsigned char*)malloc(iFATSize*512); // can be ~48 MB max
	if(NULL==ucFAT)
	{
		csText.Format("Could not allocate FAT buffer (%i bytes).", 
			iFATSize*512);
		MessageBox(csText, "Ensoniq Filesystem Tools · Error", 
			MB_ICONSTOP);
		SetStatus("Preparing FAT: failed.");
		return;
	}

	// init every FAT block
	memset(ucFAT, 0, iFATSize*512);
	for(i=0; i<iFATSize; i++)
	{
		ucFAT[i*512+510] = 'F';
		ucFAT[i*512+511] = 'B';
	}

	// mark first FAT blocks as used (device ID block, OS block, dir, FAT)
	for(i=0; i<iFATSize+5; i++)
	{
		j = ((i/170)*512) + (i%170)*3;
		ucFAT[j+0] = 0x00;
		ucFAT[j+1] = 0x00;
		ucFAT[j+2] = 0x01;
	}

	// prepare disk header
	SetStatus("Preparing disk header...");
	memset(ucDiskHeader, 0, 5*512);

	// unused block 0
	for(i=0; i<512; i++)
	{
		ucDiskHeader[i++] = 0x6D;
		ucDiskHeader[i] =   0xB6;
	}
	strcpy((char*)ucDiskHeader, "This disk has been formatted using Ensoniq"
		"Filesystem Tools (c) 2006 thoralt@thoralt.de \r\n");

	// device ID block 1
	ucDiskHeader[512*1+ 0] = 0x00; // 0x00: type=disk
	ucDiskHeader[512*1+ 1] = 0x00; // 0x80: removable disk, 0x00: fixed disk
	ucDiskHeader[512*1+ 2] = 0x02; // "various standards version #" (?)
	ucDiskHeader[512*1+ 3] = 0x00; // "reserved for SCSI" (?)
	ucDiskHeader[512*1+ 4] = 0x00; // 
	ucDiskHeader[512*1+ 5] = 0x00; // sectors (only filled in for floppy)
	ucDiskHeader[512*1+ 6] = 0x00; // 
	ucDiskHeader[512*1+ 7] = 0x00; // heads (only filled in for floppy)
	ucDiskHeader[512*1+ 8] = 0x00; // 
	ucDiskHeader[512*1+ 9] = 0x00; // cylinders (only filled in for floppy)
	ucDiskHeader[512*1+10] = 0x00; //
	ucDiskHeader[512*1+11] = 0x00; //
	ucDiskHeader[512*1+12] = 0x02; //
	ucDiskHeader[512*1+13] = 0x00; // bytes per sector (0x00000200=512)
	ucDiskHeader[512*1+14] = (iBlocksOnDisk>>24)&0xFF;
	ucDiskHeader[512*1+15] = (iBlocksOnDisk>>16)&0xFF;
	ucDiskHeader[512*1+16] = (iBlocksOnDisk>> 8)&0xFF;
	ucDiskHeader[512*1+17] = (iBlocksOnDisk    )&0xFF;
	ucDiskHeader[512*1+18] = 0x00; // "SCSI medium type" (?)
	ucDiskHeader[512*1+19] = 0x00; // "SCSI density code" (?)
	// 512*1+20...512*1+29: reserved
	ucDiskHeader[512*1+30] = 0xFF; // Disk label start

	// disk label
	m_ctlEditDiskLabel.GetWindowText(csText);
	for(i=0; i<7; i++) ucDiskHeader[512*1+31+i] = ' '; // fill with spaces
	j = csText.GetLength(); if(j>7) j = 7;
	for(i=0; i<j; i++) ucDiskHeader[512*1+31+i] = csText[i];

	ucDiskHeader[512*1+38] = 'I'; //
	ucDiskHeader[512*1+39] = 'D'; // Ensoniq Disk ID

	// OS block 2
	ucDiskHeader[512*2+ 0] = (iFreeBlocks>>24)&0xFF;
	ucDiskHeader[512*2+ 1] = (iFreeBlocks>>16)&0xFF;
	ucDiskHeader[512*2+ 2] = (iFreeBlocks>> 8)&0xFF;
	ucDiskHeader[512*2+ 3] = (iFreeBlocks    )&0xFF;

	ucDiskHeader[512*2+28] = 'O'; //
	ucDiskHeader[512*2+29] = 'S'; // Ensoniq OS block ID

	// main directory block 3+4
	ucDiskHeader[512*4+510] = 'D';
	ucDiskHeader[512*4+511] = 'R';

	// write disk header
	SetStatus("Writing disk header...");
	iResult = m_pParent->WriteBlocks(m_pDisk, 0, 5, ucDiskHeader);
	if(ERR_OK!=iResult)
	{
		m_pParent->CacheFlush(m_pDisk);

		csText.Format("Could not write to disk, error code=%i.", iResult);
		MessageBox(csText, "Ensoniq Filesystem Tools · Error", 
			MB_ICONSTOP);
		SetStatus("Writing disk header: failed.");
		free(ucFAT);
		return;
	}

	SetStatus("Writing FAT...");
	for(i=0; i<iFATSize/128; i++)
	{
		iResult = m_pParent->WriteBlocks(m_pDisk, i*128+5, 128, 
			ucFAT+i*128*512);
		if(ERR_OK!=iResult)
		{
			m_pParent->CacheFlush(m_pDisk);

			csText.Format("Could not write to disk, error code=%i.",
				iResult);
			MessageBox(csText, "Ensoniq Filesystem Tools · Error", 
				MB_ICONSTOP);
			SetStatus("Writing FAT: failed.");
			free(ucFAT);
			return;
		}
		SetProgress(i*100/(iFATSize/128));
	}
	iResult = m_pParent->WriteBlocks(m_pDisk, i*128+5, iFATSize%128, 
		ucFAT+i*128*512);
	if(ERR_OK!=iResult)
	{
		m_pParent->CacheFlush(m_pDisk);

		csText.Format("Could not write to disk, error code=%i.",
			iResult);
		MessageBox(csText, "Ensoniq Filesystem Tools · Error", 
			MB_ICONSTOP);
		SetStatus("Writing FAT: failed.");
		free(ucFAT);
		return;
	}
	m_pParent->CacheFlush(m_pDisk);
	SetProgress(100);

	MessageBox("The disk has been successfully formatted.", 
		"Ensoniq Filesystem Tools · Warning", MB_ICONINFORMATION);
	SetStatus("Format successful.");
	free(ucFAT);
	
//	CDialog::OnOK();
}

//----------------------------------------------------------------------------
// OnCancel
//----------------------------------------------------------------------------
void CFormatDlg::OnCancel() 
{
	
	CDialog::OnCancel();
}

//----------------------------------------------------------------------------
// Unit dropdown changed
// -> size conversion
//----------------------------------------------------------------------------
void CFormatDlg::OnSelchangeComboUnit() 
{
	char cSize[128], cTemp[128];
	int dec, sign, i, j;
	double dSize;

	m_ctlEditCapacity.GetWindowText(cSize, 127);
	dSize = atof(cSize);

	// convert current value to <bytes>
	switch(m_iLastUnit)
	{
		case UNIT_BYTE:
			break;
		case UNIT_KB:
			dSize *= 1024; break;
		case UNIT_MB:
			dSize *= 1024*1024; break;
		case UNIT_GB:
			dSize *= 1024*1024*1024; break;
		default:
			break;
	}

	// convert current value to new unit
	switch(m_ctlComboUnit.GetCurSel())
	{
		case UNIT_BYTE:
			break;
		case UNIT_KB:
			dSize /= 1024; break;
		case UNIT_MB:
			dSize /= 1024*1024; break;
		case UNIT_GB:
			dSize /= 1024*1024*1024; break;
		default:
			break;
	}

	strcpy(cTemp, _fcvt(dSize, 3, &dec, &sign)); j = 0;
	if(dec<1)
	{
		strcpy(cSize, "0."); j = 2;
		for(i=0; i<abs(dec); i++) cSize[j++] = '0';
	}
	for(i=0; i<(int)strlen(cTemp); i++)
	{
		if(i==dec) cSize[j++] = '.';
		cSize[j++] = cTemp[i];
	}
	cSize[j] = 0;

	m_ctlEditCapacity.SetWindowText(cSize);
	m_iLastUnit = m_ctlComboUnit.GetCurSel();
}

//----------------------------------------------------------------------------
// WM_INITDIALOG
//----------------------------------------------------------------------------
BOOL CFormatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return TRUE;
}

//----------------------------------------------------------------------------
// SetParent
//----------------------------------------------------------------------------
void CFormatDlg::SetParent(CEToolsDlg *pParent, DISK *pDisk)
{
	m_pParent = pParent;
	m_pDisk = pDisk;
}

//----------------------------------------------------------------------------
// WM_SHOWWINDOW
//----------------------------------------------------------------------------
void CFormatDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CString csText;
	int iSize;

	// show max capacity
	iSize = m_pDisk->dwPhysicalBlocks; iSize /= 2;
	if(iSize>8192*1024) iSize = 8192*1024;
	csText.Format("%i kB max", iSize);
	m_ctlStaticMaxCapacity.SetWindowText(csText);
	
	csText.Format("%i", iSize); m_ctlEditCapacity.SetWindowText(csText);
	m_ctlEditDiskLabel.SetWindowText("DISK000");
	m_ctlEditCapacity.SendMessage(EM_LIMITTEXT, 10, 0);
	m_ctlEditDiskLabel.SendMessage(EM_LIMITTEXT, 7, 0);
	m_ctlComboUnit.SetCurSel(1); m_iLastUnit = UNIT_KB;

	CDialog::OnShowWindow(bShow, nStatus);
}

//----------------------------------------------------------------------------
// SetStatus
// 
// Update status line
// 
// -> csStatus = line to show
// <- --
//----------------------------------------------------------------------------
void CFormatDlg::SetStatus(CString csStatus)
{
	// set new status text
	m_ctlStaticStatus.SetWindowText(csStatus);
	Pump();
}

//----------------------------------------------------------------------------
// SetProgress
// 
// Update progress bar
// 
// -> iProgress = new value (0...100)
// <- --
//----------------------------------------------------------------------------
void CFormatDlg::SetProgress(int iProgress)
{
	m_ctlProgress.SetPos(iProgress);
	Pump();
}

//----------------------------------------------------------------------------
// Pump
//
// Retrieve and dispatch any waiting messages.
//----------------------------------------------------------------------------
BOOL CFormatDlg::Pump()
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
