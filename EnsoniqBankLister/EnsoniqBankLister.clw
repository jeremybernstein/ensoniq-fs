; CLW-Datei enthält Informationen für den MFC-Klassen-Assistenten

[General Info]
Version=1
LastClass=CEnsoniqBankListerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "EnsoniqBankLister.h"

ClassCount=2
Class1=CEnsoniqBankListerApp
Class2=CEnsoniqBankListerDlg

ResourceCount=4
Resource2=IDR_MAINFRAME
Resource3=IDD_ENSONIQBANKLISTER_DIALOG
Resource4=IDD_ENSONIQBANKLISTER_DIALOG (Englisch (USA))

[CLS:CEnsoniqBankListerApp]
Type=0
HeaderFile=EnsoniqBankLister.h
ImplementationFile=EnsoniqBankLister.cpp
Filter=N

[CLS:CEnsoniqBankListerDlg]
Type=0
HeaderFile=EnsoniqBankListerDlg.h
ImplementationFile=EnsoniqBankListerDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_STATIC_OUTPUT



[DLG:IDD_ENSONIQBANKLISTER_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CEnsoniqBankListerDlg

[DLG:IDD_ENSONIQBANKLISTER_DIALOG (Englisch (USA))]
Type=1
Class=CEnsoniqBankListerDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_EDIT_DETAILS,edit,1353781444

