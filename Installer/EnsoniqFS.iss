; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=EnsoniqFS
AppVerName=EnsoniqFS 0.54-BETA
AppPublisher=Thoralt Franz
AppPublisherURL=http://sourceforge.net/projects/ensoniqfs/
AppSupportURL=http://sourceforge.net/projects/ensoniqfs/
AppUpdatesURL=http://sourceforge.net/projects/ensoniqfs/
DefaultDirName={pf}\EnsoniqFS
DefaultGroupName=EnsoniqFS
LicenseFile=..\ensoniqfs\LICENSE
OutputBaseFilename=ensoniqfs-setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\ETools\Release\ETools.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ensoniqfs\EnsoniqFS.wfx"; DestDir: "{reg:HKCU\Software\Ghisler\Total Commander,InstallDir|{app}}\plugins\wfx"; Flags: ignoreversion
Source: "..\ensoniqfs\CREDITS"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ensoniqfs\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ensoniqfs\README"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Ensoniq Filesystem Tools"; Filename: "{app}\ETools.exe"
Name: "{group}\License"; Filename: "{app}\LICENSE"
Name: "{group}\Readme"; Filename: "{app}\README"
Name: "{group}\Credits"; Filename: "{app}\CREDITS"
Name: "{group}\{cm:ProgramOnTheWeb,EnsoniqFS}"; Filename: "http://sourceforge.net/projects/ensoniqfs/"
Name: "{group}\{cm:UninstallProgram,EnsoniqFS}"; Filename: "{uninstallexe}"

[INI]
Filename: "{win}\wincmd.ini"; Section: "FileSystemPlugins"; Key: "Ensoniq filesystems"; String: "{reg:HKCU\Software\Ghisler\Total Commander,InstallDir|{app}}\plugins\wfx\EnsoniqFS.wfx"; Flags: uninsdeleteentry createkeyifdoesntexist
Filename: "{win}\fsplugin.ini"; Section: "EnsoniqFS"; Key: "InstallPath"; String: "{app}"; Flags: uninsdeleteentry createkeyifdoesntexist

