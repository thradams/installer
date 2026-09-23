#pragma once

/*
   Product details, files and events of the Windows installer (win_installer.c).
   This is the only file you need to edit.
   https://github.com/thradams/installer
*/

#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#define INSTALLER_PRODUCT_VERSION   L"1.2.3"
#define INSTALLER_PRODUCT_NAME      L"Install Product Name"
#define INSTALLER_PRODUCT_PUBLISHER L"Install Test Company"
#define INSTALLER_PRODUCT_WEB_SITE  L"https://github.com/thradams/installer"

/* Your product id. Create a new GUID for each product. */
#define INSTALLER_PRODUCT_CODE L"{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}"

#define INSTALLER_DISPLAY_NAME INSTALLER_PRODUCT_NAME L" " INSTALLER_PRODUCT_VERSION

/* default installation folder, inside Program Files */
#define INSTALLER_INSTALL_SUBDIR INSTALLER_PRODUCT_PUBLISHER L"\\" INSTALLER_PRODUCT_NAME

/*
   {source, destination}
   source is read when setup.exe is built (relative to the current directory):
     - a file           destination is the file path
     - a folder         copied recursively; destination is the folder
     - a wildcard *.h   matching files; destination is the folder
   destination is relative to the installation folder.
   uninstall.exe and uninstall.lst are created by setup.exe.
*/
#define INSTALLER_FILES \
    {L"program.exe", L"program.exe"}

/* optional: shows "I agree with the license terms"; the file is read when setup.exe is built */
#define INSTALLER_LICENSE_FILE L"win_license.rtf"

/* optional: at the end, asks to run this program (relative to the installation folder) */
/* #define INSTALLER_RUN_PROGRAM_AT_END L"program.exe" */

/*
   Uninstall registry key. To install versions side by side, include the version:
   ... INSTALLER_PRODUCT_CODE L"_" INSTALLER_PRODUCT_VERSION
*/
#define INSTALLER_UNINST_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" INSTALLER_PRODUCT_CODE

/*
   Called after all files and the uninstall registry key were written.
   Example: add install_dir to the system PATH (HKLM ...\Session Manager\Environment)
   and broadcast WM_SETTINGCHANGE.
*/
static void on_installed(const wchar_t* install_dir)
{
    (void)install_dir;
}

/*
   Called before the files are removed.
   Example: undo what on_installed did (remove install_dir from PATH).
*/
static void on_uninstall(const wchar_t* install_dir)
{
    (void)install_dir;
}
