; TCR compliant Sample install script.
; Basically put this file in the root for what you want included in the installer,
;   then put everything into the subdirectories as listed in the installer below.
;   Remember to change the AppId
;   Thanks to Dan Weiss (dweiss@digipen.edu) for the original version.

[Setup]
; AppId uniquely identifies this application.
; You can keep this GUID or generate a new one in Visual Studio if you want.
AppId={{C914AD44-643B-43A4-9334-213AD0C43EF3}}

AppName=NocturnalGame
AppVerName=NocturnalGame Version 0.1
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

; Default install folder, under Program Files
DefaultDirName={userdocs}\DigiPen\NocturnalGame

; Start menu folder
DefaultGroupName=DigiPen\NocturnalGame

; Where the installer EXE will be created
OutputDir=.\INSTALLER
OutputBaseFilename=NocturnalGame_Setup

; DigiPen EULA + installer icon (from the sample)
LicenseFile=INSTALLERFILES\DigiPen_EULA.txt
SetupIconFile=.\INSTALLERFILES\gameicon.ico

; Compression + no admin rights needed
Compression=lzma
SolidCompression=yes
PrivilegesRequired=none

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; Creates an installer option to allow/disallow desktop shortcut
; Checked by default
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

;
; Files to include in the installer
;
[Files]
; Game files
Source: .\GAMEDIRECTORY\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs

; Copy the game icon into the install folder so shortcuts can use it
Source: .\INSTALLERFILES\gameicon.ico; DestDir: {app}; Flags: ignoreversion
; Redistributable
Source: .\REDIST\VC_redist.x64.exe; DestDir: {tmp}; Flags: ignoreversion

; Shortcuts to create
[Icons]
; Start menu shortcut to the game
Name: {group}\NocturnalGame; Filename: {app}\NocturnalGame.exe; WorkingDir: {app}; IconFilename: {app}\gameicon.ico

; Start menu shortcut to uninstall
Name: {group}\{cm:UninstallProgram,NocturnalGame}; Filename: {uninstallexe}

; Optional desktop shortcut for this user
Name: {userdesktop}\NocturnalGame; Filename: {app}\NocturnalGame.exe; Tasks: desktopicon; WorkingDir: {app}; IconFilename: {app}\gameicon.ico


; Things to run after installation
[Run]
Filename: {tmp}\VC_redist.x64.exe; Parameters: /q; StatusMsg: Installing Visual C++ 2022 Redistributable...
Filename: {app}\NocturnalGame.exe; WorkingDir: {app}; Description: {cm:LaunchProgram,NocturnalGame}; Flags: nowait postinstall skipifsilent
