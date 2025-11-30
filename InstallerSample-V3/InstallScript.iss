; TCR compliant Sample install script.
; Basically put this file in the root for what you want included in the installer,
;   then put everything into the subdirectories as listed in the installer below.
;   Remember to change the AppId
;   Thanks to Dan Weiss (dweiss@digipen.edu) for the original version.

[Setup]
; !!NOTE!!: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click "Tools" | "Create GUID" inside the Visual Studio IDE.)
AppId={{C914AD44-643B-43A4-9334-213AD0C43EF3}}

; =======================
;   GAME INFO
; =======================
AppName=NocturnalGame
AppVerName=NocturnalGame Version 0.1
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

; =======================
;   INSTALL LOCATION
; =======================
; Default path:
;   C:\Program Files (x86)\DigiPen\NocturnalGame
; {pf32} = Program Files (x86) on 64-bit Windows
DefaultDirName={pf}\DigiPen\NocturnalGame

; Do NOT reuse some old Documents path from registry
UsePreviousAppDir=no

; Show the "Select Destination Location" page
DisableDirPage=no

; Start menu directory
DefaultGroupName=DigiPen\NocturnalGame

; Output directory for the installer.
OutputDir=.\INSTALLER

; Setup executable name
OutputBaseFilename=NocturnalGame_Setup



; Path to the icon for the installer (TCR check requires custom icon)
SetupIconFile=.\INSTALLERFILES\gameicon.ico

; Compression scheme for the installer. Check Inno Setup help files for more options.
Compression=lzma
SolidCompression=yes

; If you really want Program Files (x86), you normally need admin.
; Change to "none" only if you go back to installing under user folders.
PrivilegesRequired=none

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; Creates an installer option to allow/disallow desktop shortcut
; Checked by default
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

;
; FILES
;
; Destination Directory Notes
;   {app} is the root directory that you will be installing to.
;   {tmp}  is a temporary directory that Windows sets that gets deleted after the
;          installer quits.
[Files]
; The game directory is exactly what you want your install directory in Program Files to look like
; Make sure NocturnalGame.exe and all DLLs/assets live inside .\GAMEDIRECTORY\
Source: .\GAMEDIRECTORY\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs

; Include the redistributable programs and install them to the temp directory
Source: .\REDIST\VC_redist.x64.exe; DestDir: {tmp}; Flags: ignoreversion
; If you have DX/web installers etc, you can add them here too.

;
; SHORTCUTS
;
;   {group}        = Start menu location
;   {commondesktop}= Public desktop
[Icons]
; Start menu shortcut to the game
Name: {group}\NocturnalGame; Filename: {app}\NocturnalGame.exe; WorkingDir: {app}; IconFilename: "{app}\gameicon.ico"

; Start menu shortcut to uninstall
Name: {group}\{cm:UninstallProgram,NocturnalGame}; Filename: {uninstallexe}
Name: {group}\{cm:UninstallProgram,NocturnalGame}; Filename: {uninstallexe}; IconFilename: "{app}\gameicon.ico"

; Desktop shortcut
Name: {commondesktop}\NocturnalGame; Filename: {app}\NocturnalGame.exe; Tasks: desktopicon; WorkingDir: {app}; IconFilename: "{app}\gameicon.ico"

;
; POST-INSTALL RUN
;
[Run]
Filename: {tmp}\VC_redist.x64.exe; Parameters: /q; StatusMsg: Installing Visual C++ 2022 Redistributable...
Filename: {app}\NocturnalGame.exe; Description: {cm:LaunchProgram,NocturnalGame}; Flags: nowait postinstall skipifsilent

; To be able to run and generate the NocturnalGame_Setup.exe you need to download and install
; "Inno Setup" application, then open this file with the application.
