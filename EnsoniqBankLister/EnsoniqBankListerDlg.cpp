// EnsoniqBankListerDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "EnsoniqBankLister.h"
#include "EnsoniqBankListerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// CEnsoniqBankListerDlg
//----------------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------------
CEnsoniqBankListerDlg::CEnsoniqBankListerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEnsoniqBankListerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnsoniqBankListerDlg)
	//}}AFX_DATA_INIT
	// Beachten Sie, dass LoadIcon unter Win32 keinen nachfolgenden DestroyIcon-Aufruf benötigt
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//----------------------------------------------------------------------------
// data exchange
//----------------------------------------------------------------------------
void CEnsoniqBankListerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnsoniqBankListerDlg)
	DDX_Control(pDX, IDC_EDIT_DETAILS, m_ctlEditDetails);
	//}}AFX_DATA_MAP
}

//----------------------------------------------------------------------------
// MESSAGE_MAP
//----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CEnsoniqBankListerDlg, CDialog)
	//{{AFX_MSG_MAP(CEnsoniqBankListerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//----------------------------------------------------------------------------
// WM_INITDIALOG
//----------------------------------------------------------------------------
BOOL CEnsoniqBankListerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
		
	m_EditFont.CreateFont(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 
		FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, "Courier New");
	m_ctlEditDetails.SetFont(&m_EditFont);

	return TRUE;
}

//----------------------------------------------------------------------------
// WM_PAINT
//----------------------------------------------------------------------------
void CEnsoniqBankListerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//----------------------------------------------------------------------------
// WM_QUERYDRAGICON
//----------------------------------------------------------------------------
HCURSOR CEnsoniqBankListerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//----------------------------------------------------------------------------
// OnOK
//----------------------------------------------------------------------------
void CEnsoniqBankListerDlg::OnOK() 
{
#define TYPE_UNKNOWN	0
#define TYPE_EPS		1
#define TYPE_EPS16		2
#define TYPE_ASR		3

	static char BASED_CODE szFilter[] = "Ensoniq files (*.efe)|*.efe|"
										"All Files (*.*)|*.*||";
	CString csFN, csText, csTemp;
	int i, j, iFileSize, iType, iVolumeTableOffset, iPanningTableOffset;
	unsigned char *ucBuf;
	CFileDialog *dlg;
	char c;
	FILE *f;

	// show "Open..." dialog
 	dlg = new CFileDialog(TRUE, "efe", "", 
		0, szFilter, this);
	if(NULL==dlg) return;
	if(IDCANCEL==dlg->DoModal()) return;
	csFN = dlg->GetPathName();
	delete dlg;

	// open file
	f = fopen(csFN, "rb");
	if(NULL==f)
	{
		MessageBox("Could not open file.", "Error", MB_ICONSTOP);
		return;
	}

	// get file size (this works only for files < 4 GB)
	fseek(f, 0, SEEK_END);
	iFileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	// limit file size
	if(iFileSize>65536)
	{
		iFileSize=65536;
		MessageBox("The file is greater than 64k. Only the beginning\n"
			"of the file will be read.", "Warning", MB_ICONWARNING);
	}
	if(iFileSize<1024)
	{
		MessageBox("The file is too small.", "Error", MB_ICONSTOP);
		fclose(f);
		return;
	}

	// allocate mem
	ucBuf = (unsigned char*)malloc(iFileSize-512);
	if(NULL==ucBuf)
	{
		MessageBox("Could not open allocate memory.", "Error", MB_ICONSTOP);
		fclose(f);
		return;
	}

	// read header (will be ignored)
	if(512!=(int)fread(ucBuf, 1, 512, f))
	{
		MessageBox("There was an error during file read (ignoring).",
			"Warning", MB_ICONWARNING);
	}

	// read data
	if((iFileSize-512)!=(int)fread(ucBuf, 1, iFileSize-512, f))
	{
		MessageBox("There was an error during file read (ignoring).",
			"Warning", MB_ICONWARNING);
	}

	m_ctlEditDetails.SetWindowText("");
	
	// first eight bytes

	// check signature
	if((0x89==ucBuf[4])&&(0x40==ucBuf[5])&&
		(0xFF==ucBuf[6])&&(0xC0==ucBuf[7]))
	{
		csTemp = "EPS";
		iType = TYPE_EPS;
		iVolumeTableOffset = 0x00b2; iPanningTableOffset = 0x00c2;
	}
	else if((0xA8==ucBuf[4])&&(0x20==ucBuf[5])&&
		(0x00==ucBuf[6])&&(0x00==ucBuf[7]))
	{
		csTemp = "EPS16+";
		iType = TYPE_EPS16;
		iVolumeTableOffset = 0x00b2; iPanningTableOffset = 0x00c2;
	}
	else if((0x34==ucBuf[4])&&(0xC0==ucBuf[5])&&
		(0x00==ucBuf[6])&&(0x00==ucBuf[7]))
	{
		csTemp = "ASR";
		iType = TYPE_ASR; 
		iVolumeTableOffset = 0x011e; iPanningTableOffset = 0x013e;
	}
	else
	{
		csTemp = "unknown";
		iType = TYPE_UNKNOWN;
		iVolumeTableOffset = 0x0000; iPanningTableOffset = 0x0000;
	}

	// calculate size
	i = (ucBuf[1] | (ucBuf[0]<<8) | (ucBuf[3]<<16) | (ucBuf[2]<<24))>>4;
	csText.Format(" 4 0x0000: %02X %02X %02X %02X                       "
		"              | size 0x%04X (%i)"
		"\r\n 4 0x0004: %02X %02X %02X %02X                             "
		"        | %s signature", ucBuf[0], ucBuf[1], ucBuf[2],
		ucBuf[3], i, i, ucBuf[4], ucBuf[5], ucBuf[6], ucBuf[7], csTemp);
	m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
	m_ctlEditDetails.SetWindowText(csTemp);

	// bank name 0x0008
	csText = "\r\n24 0x0008:";
	for(i=8; i<32; i++)
	{
		csText.Format("%s %02X", csText, ucBuf[i]);
		if(23==i) csText += " |\r\n   0x0018:";
	}
	csText +="                         | bank: \"";
	for(i=8; i<32; i+=2) csText+=ucBuf[i];
	csText += "\"";
	m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
	m_ctlEditDetails.SetWindowText(csTemp);

	// instrument mask 0x0020, 0x0021
	csText.Format("\r\n 1 0x0020: %02X                              "
		"                | track mask = %02X (%i)"
		"\r\n 1 0x0021: %02X                              "
		"                | (unknown)",
		ucBuf[0x20], ucBuf[0x20], ucBuf[0x20], ucBuf[0x21]);
	m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
	m_ctlEditDetails.SetWindowText(csTemp);

	if(TYPE_ASR!=iType)
	{
		// EPS/EPS16+ file info blocks
		for(i=0; i<9; i++)
		{
			// output hex bytes
			csText.Format("\r\n16 0x%04X:", i*16+0x22);
			for(j=0; j<16; j++)
			{
				csText.Format("%s %02X", csText, ucBuf[i*16+0x22+j]);
			}

			// decode disk name
			csTemp = ""; for(j=0; j<7; j++) csTemp += ucBuf[i*16+0x22+3+j*2];

			// is this an instrument?
			if(0==(ucBuf[i*16+0x22]&0x80))
			{
				csText.Format("%s | file %i, disk \"%s\", location",
					csText, i, csTemp);
				switch(ucBuf[i*16+0x24])
				{
					case 0:
						csText += " floppy"; break;
					case 1:
						csText += " SCSI0"; break;
					case 2:
						csText += " SCSI1"; break;
					case 3:
						csText += " SCSI2"; break;
					case 4:
						csText += " SCSI3"; break;
					case 5:
						csText += " SCSI4"; break;
					case 6:
						csText += " SCSI5"; break;
					case 7:
						csText += " SCSI6"; break;
					case 8:
						csText += " SCSI7"; break;
					default:
						csText += " unknown"; break;
				}

				for(j=0; j<6; j++)
				{
					csText.Format("%s %02X", csText, ucBuf[i*16+0x22+4+j*2]);
				}
			}
			else // this is a copy
			{
				csText.Format("%s | file %i, copy of track %i", 
					csText, i, ucBuf[i*16+0x22]&7);
			}
			m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
			m_ctlEditDetails.SetWindowText(csTemp);
		}
	}
	else
	{
		// ASR file info blocks
	
		for(i=0; i<15; i++)
		{
			// output hex bytes
			csText.Format("\r\n16 0x%04X:", i*16+0x22); csTemp = "";
			for(j=0; j<16; j++)
			{
				csText.Format("%s %02X", csText, ucBuf[i*16+0x22+j]);
				if(ucBuf[i*16+0x22+j]>31) csTemp += ucBuf[i*16+0x22+j];
				else csTemp += " ";
			}
			csText.Format("%s | \"%s\" (unknown ASR file info blocks)",
				csText, csTemp);
			m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
			m_ctlEditDetails.SetWindowText(csTemp);
		}

		// output hex bytes
		csText.Format("\r\n12 0x%04X:", i*16+0x22);
		for(j=0; j<12; j++)
		{
			csText.Format("%s %02X", csText, ucBuf[i*16+0x22+j]);
		}
		csText += "             | ASR file info blocks (unknown)";
		m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
		m_ctlEditDetails.SetWindowText(csTemp);
	}

	// volume/track output table
	for(i=0; i<8; i++)
	{
		// volume
		j = ucBuf[i*2+iVolumeTableOffset];

		// track output
		switch(ucBuf[i*2+iVolumeTableOffset+1])
		{
			case 0:
				csTemp = "WS"; break;
			case 1:
				csTemp = "BUS1"; break;
			case 2:
				csTemp = "BUS2"; break;
			case 3:
				csTemp = "BUS3"; break;
			case 4:
				csTemp = "AUX1"; break;
			case 5:
				csTemp = "ROTATE"; break;
			default:
				csTemp = "unknown"; break;
		}

		csText.Format("\r\n 2 0x%04X: %02X %02X                    "
			"                       | track %i volume %i, output %s",
			i*2+iVolumeTableOffset,
			j, ucBuf[i*2+iVolumeTableOffset+1], i, j*99/0x7F, csTemp);
		m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
		m_ctlEditDetails.SetWindowText(csTemp);
	}

	// panning/fx control table
	for(i=0; i<8; i++)
	{
		// panning
		if(0x80==ucBuf[i*2+iPanningTableOffset])
		{
			csTemp = "WS";
		}
		else
		{
			c = ucBuf[i*2+iPanningTableOffset]; j = c;
			csTemp.Format("%i", j*99/0x7F);
		}

		csText.Format("\r\n 2 0x%04X: %02X %02X                    "
			"                       | track %i panning %s, fx control %i",
			i*2+iPanningTableOffset,
			ucBuf[i*2+iPanningTableOffset],
			ucBuf[i*2+iPanningTableOffset+1], i, csTemp,
			ucBuf[i*2+iPanningTableOffset+1]);
		m_ctlEditDetails.GetWindowText(csTemp); csTemp += csText;
		m_ctlEditDetails.SetWindowText(csTemp);
	}

	fclose(f);
	free(ucBuf);
}

//----------------------------------------------------------------------------
// OnCancel
//----------------------------------------------------------------------------
void CEnsoniqBankListerDlg::OnCancel() 
{
	

	CDialog::OnCancel();
}
