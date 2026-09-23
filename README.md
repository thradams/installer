# installer

Minimal Windows installer in a single C file. No .rc, no zip library, no separate uninstall project.

## Files (`installer/`)

| File | |
|---|---|
| `win_installer.c` | the installer (generic, no need to edit) |
| `win_installer.h` | product details, files and events — the only file you edit |
| `win_installer.ico` | icon of setup.exe and uninstall.exe |
| `win_license.rtf` | license shown by setup.exe (optional) |

## Build

In a Developer Command Prompt, inside `installer/`:

```
cl win_installer.c
win_installer.exe setup.exe
```

`win_installer.exe setup.exe` creates `setup.exe`: a copy of `win_installer.exe` with
`INSTALLER_FILES` (LZMS compressed with the Windows Compression API), license, icon and a
manifest (requireAdministrator, common controls 6, dpiAware) added as resources with
`UpdateResource`.

## Configuration (`win_installer.h`)

| Macro | |
|---|---|
| `INSTALLER_PRODUCT_VERSION` | e.g. `L"1.2.3"` |
| `INSTALLER_PRODUCT_NAME` | |
| `INSTALLER_PRODUCT_PUBLISHER` | |
| `INSTALLER_PRODUCT_WEB_SITE` | |
| `INSTALLER_PRODUCT_CODE` | GUID, create a new one for each product |
| `INSTALLER_DISPLAY_NAME` | name shown in the dialog and in Settings > Apps |
| `INSTALLER_INSTALL_SUBDIR` | default folder inside Program Files |
| `INSTALLER_FILES` | list of `{source, destination}` (see below) |
| `INSTALLER_UNINST_KEY` | uninstall registry key (under HKLM) |
| `INSTALLER_LICENSE_FILE` | optional: shows "I agree with the license terms" |
| `INSTALLER_RUN_PROGRAM_AT_END` | optional: asks to run this program at the end |

Optional macros are checked with `#ifdef`; comment them out to disable.

`INSTALLER_FILES` sources are read when setup.exe is built (relative to the current directory):

```c
#define INSTALLER_FILES \
    {L"..\program.exe", L"program.exe"}, /* a file */                  \
    {L"..\samples",     L"samples"},     /* a folder (recursive) */    \
    {L"..\include\*.h", L"include"}     /* a wildcard */
```

Destinations are relative to the installation folder.

**Side by side versions:** include the version in the uninstall key (and in `INSTALLER_INSTALL_SUBDIR`):

```c
#define INSTALLER_UNINST_KEY L"Software\Microsoft\Windows\CurrentVersion\Uninstall\\" INSTALLER_PRODUCT_CODE L"_" INSTALLER_PRODUCT_VERSION
```

## Events

Static functions in `win_installer.h`:

```c
static void on_installed(const wchar_t* install_dir); /* after files and registry key were written */
static void on_uninstall(const wchar_t* install_dir); /* before files are removed */
```

Example: add/remove `install_dir` in the system PATH.

## setup.exe

Shows the destination folder (with browse), the optional license link (opens the RTF) and a
progress bar. It extracts the files and writes:

- `uninstall.lst` — list of installed files
- `uninstall.exe` — setup.exe without the payload/license, marked as uninstaller
- the uninstall registry key (DisplayName, DisplayVersion, Publisher, URLInfoAbout,
  InstallLocation, UninstallString, DisplayIcon)

Then calls `on_installed` and, if `INSTALLER_RUN_PROGRAM_AT_END` is defined, asks to run it.

## uninstall.exe

Run it directly or from Settings > Apps. It asks for confirmation, calls `on_uninstall`,
deletes the files in `uninstall.lst` and empty folders, deletes the registry key and then
removes `uninstall.exe` and the installation folder.

## Debugging

Use a Developer Command Prompt started as Administrator (setup.exe and uninstall.exe require it):

```
cl /Zi win_installer.c
devenv /DebugExe win_installer.exe setup.exe       (debug the build step)
win_installer.exe setup.exe
devenv /DebugExe setup.exe                         (debug the installer)
devenv /DebugExe "C:\Program Files\...\uninstall.exe"  (debug the uninstaller)
```

setup.exe and uninstall.exe are copies of win_installer.exe, so `win_installer.pdb` is used for both.
