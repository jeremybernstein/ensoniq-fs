; CLW-Datei enthält Informationen für den MFC-Klassen-Assistenten

[General Info]
Version=1
LastClass=CBackupOptionsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ETools.h"

ClassCount=6
Class1=CEToolsApp
Class2=CEToolsDlg
Class3=CAboutDlg

ResourceCount=7
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_ETOOLS_DIALOG
Resource4=IDD_DLG_FORMAT
Resource5=IDD_ETOOLS_DIALOG (Englisch (USA))
Class4=CFormatDlg
Class5=CDriveSelect
Resource6=IDD_ABOUTBOX (Englisch (USA))
Class6=CBackupOptionsDlg
Resource7=IDD_DLG_BACKUP_OPTIONS

[CLS:CEToolsApp]
Type=0
HeaderFile=ETools.h
ImplementationFile=ETools.cpp
Filter=N

[CLS:CEToolsDlg]
Type=0
HeaderFile=EToolsDlg.h
ImplementationFile=EToolsDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CEToolsDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=EToolsDlg.h
ImplementationFile=EToolsDlg.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CAboutDlg


[DLG:IDD_ETOOLS_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CEToolsDlg

[DLG:IDD_ETOOLS_DIALOG (Englisch (USA))]
Type=1
Class=CEToolsDlg
ControlCount=17
Control1=IDOK,button,1208025089
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,button,1342177287
Control5=IDC_PROGRESS,msctls_progress32,1350565889
Control6=IDC_STATIC_STATUS,static,1342308352
Control7=IDC_BUTTON_BACKUP,button,1342242816
Control8=IDC_BUTTON_RESTORE,button,1342242816
Control9=IDC_BUTTON_CHECKFS,button,1342242816
Control10=IDC_BUTTON_FORMAT,button,1342242816
Control11=IDC_COMBO_DEVICES,combobox,1344340227
Control12=IDC_STATIC,button,1342177287
Control13=IDC_BUTTON_CANCEL,button,1073807360
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352

[DLG:IDD_ABOUTBOX (Englisch (USA))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_DLG_FORMAT]
Type=1
Class=CFormatDlg
ControlCount=13
Control1=IDC_EDIT_CAPACITY,edit,1350631552
Control2=IDC_COMBO_UNIT,combobox,1344339971
Control3=IDC_CHECK_QUICK,button,1476460547
Control4=IDC_EDIT_DISK_LABEL,edit,1350631560
Control5=IDOK,button,1342242817
Control6=IDCANCEL,button,1342242816
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,button,1342177287
Control10=IDC_PROGRESS_FORMAT,msctls_progress32,1350565888
Control11=IDC_STATIC_STATUS,static,1342308352
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC_MAX_CAPACITY,static,1342308352

[CLS:CFormatDlg]
Type=0
HeaderFile=FormatDlg.h
ImplementationFile=FormatDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CFormatDlg
VirtualFilter=dWC

[CLS:CDriveSelect]
Type=0
HeaderFile=DriveSelect.h
ImplementationFile=DriveSelect.cpp
BaseClass=CDialog
Filter=D
LastObject=CDriveSelect
VirtualFilter=dWC

[DLG:IDD_DLG_BACKUP_OPTIONS]
Type=1
Class=CBackupOptionsDlg
ControlCount=9
Control1=IDC_COMBO_BACKUP_FORMAT,combobox,1075904515
Control2=IDC_EDIT_FN,edit,1484849280
Control3=IDC_BUTTON_BROWSE,button,1342242816
Control4=IDCANCEL,button,1342242816
Control5=IDOK,button,1342242817
Control6=IDC_STATIC,button,1342177287
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC_INFO,static,1342308352
Control9=IDC_PROGRESS,msctls_progress32,1350565888

[CLS:CBackupOptionsDlg]
Type=0
HeaderFile=BackupOptionsDlg.h
ImplementationFile=BackupOptionsDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_PROGRESS
VirtualFilter=dWC

