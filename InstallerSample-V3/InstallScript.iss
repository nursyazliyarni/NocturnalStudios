; NocturnalGame Installer Script

[Setup]
AppId={{99612BF2-3DA1-45E8-9086-75D4E4760CBF}}
AppName=NocturnalGame
AppVerName=NocturnalGame Version 0.1
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

DefaultDirName={pf}\DigiPen\NocturnalGame
DefaultGroupName=DigiPen\NocturnalGame

DisableDirPage=no
DisableProgramGroupPage=no
AlwaysShowDirOnReadyPage=yes
AlwaysShowGroupOnReadyPage=yes

UsePreviousAppDir=no
UsePreviousGroup=no
UsePreviousTasks=no

OutputDir=.\INSTALLER
OutputBaseFilename=NocturnalGame_Setup

LicenseFile=.\INSTALLERFILES\DigiPen_EULA.txt
SetupIconFile=.\GAMEDIRECTORY\gameicon.ico
UninstallDisplayIcon={app}\gameicon.ico

Compression=lzma
SolidCompression=yes
WizardStyle=modern

PrivilegesRequired=admin
RestartIfNeededByRun=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: ".\GAMEDIRECTORY\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\REDIST\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion

[Icons]
Name: "{group}\NocturnalGame"; Filename: "{app}\NocturnalGame.exe"; WorkingDir: "{app}"; IconFilename: "{app}\gameicon.ico"
Name: "{group}\{cm:UninstallProgram,NocturnalGame}"; Filename: "{uninstallexe}"; IconFilename: "{app}\gameicon.ico"
Name: "{autodesktop}\NocturnalGame"; Filename: "{app}\NocturnalGame.exe"; Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\gameicon.ico"

[Run]
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Visual C++ 2022 Redistributable..."; Flags: waituntilterminated
Filename: "{app}\NocturnalGame.exe"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,NocturnalGame}"; Flags: nowait postinstall skipifsilent