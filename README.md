# Embedded Install System - Zero dependencies

WARNING : THIS PROJECT NEEDS MORE DEVELOPMENT TO BECOME ROBUST.

Generate windows installer inside your normal VC++ build.

If you already use C/C++ and Visual Studio it means you can have
an installer without external dependencies.

You can debug your installer and you don't need a crazy macro language 
to customize your actions.


## Using it

 * Download [installer_src.zip](installer/installer_src.zip)
 * **Copy** the folder **installer** into your project. 
   (This folder is inside installer_src.zip)

![1](i1.png)

 **ADD** these 3 projects in your solution
 
 * installer (This is the instaler project)
 * make_install_zip  (This project creates the zip file that is embedded as a resource inside the installer)
 * unistall (This is the uninstall program)

![2](i2.png)

![3](i3.png)

Result:

![4](i4.png)

I use this projects inside a folder

![5](i5.png)

![6](i6.png)


* **EDIT the config.h** with yout product details

Sample:

```cpp

#define PRODUCT_VERSION   L"1.2.3"
#define DISPLAY_NAME L"Castle " PRODUCT_VERSION
#define PRODUCT_NAME L"Castle"

#define PRODUCT_PUBLISHER L"thradams"
#define PRODUCT_WEB_SITE L"https://github.com/thradams/castle"

#define PRODUCT_CODE L"{B1393753-AE0C-41D0-AC75-94C40DE17989}"

/*
  These files {source_path, dest_path} will be copied to the instalation dir
*/
#define FILES\
    {"release/uninstall.exe", "uninstall.exe" },\
    { "release/castle.exe", "castle.exe" },\
    \
    { "default/core.min.js", "default/core.min.js" },\
    { "default/declarations.d.ts", "default/declarations.d.ts" },\
    { "default/Home.html", "default/Home.html" },\
    { "default/Home.ts", "default/Home.ts" },\
    { "default/htmlcore.min.js", "default/htmlcore.min.js" },\
    { "default/index.template.html", "default/index.template.html" },\
    { "default/manifest.json", "default/manifest.json" },\
    { "default/manifest.webmanifest", "default/manifest.webmanifest" },\
    { "default/style.template.css", "default/style.template.css" },\
    { "default/svgcore.min.js", "default/svgcore.min.js" },\
    { "default/tsconfig.json", "default/tsconfig.json" },\
    { "default/.vscode/launch.json", "default/.vscode/launch.json" },\
    { "default/.vscode/settings.json", "default/.vscode/settings.json" }
 ```
 
![7](i7.png)


Required build order.

 1) Your program

 2) **unistall** 
   Creates the **uninstall.exe** program

 3) **make_install_zip** 
    Creates the **make_files_zip.exe** programs that runs (Event After Build) and creates the **files.zip** that is a embedded inside the installer.exe.

    
 4) **installer**
    This will create the intaller.exe

File script.c is used for customizations. 
There is only one event today **OnFilesExtracted** that happens after files have been copied. 

Once you understand the mechanics of the build you can use different approaches. Let's say
you already have an build script then you can move the installer build to another solution.

## Sample project (I was testing it)

![7](screenshot.png)

![8](screenshot2.png)


script.c

I want to add the install dir into the path. So I put some extra code
at OnFilesExtracted event.

```c
/*
     THIS IS THE CUSTOMIZATION FILE
*/

#include "installer_.h"
#include "config.h"


void AddSystemVariablesPath(const wchar_t* pathToAdd);

void OnFilesExtracted()
{
  /*
    This function is called after all files and common registry keys
    were writtem    
  */
    AddSystemVariablesPath(INSTDIR);
}
```
## Contributing

This section is necessary only if you want to contribute with the project and understand
how to use the projects/solutions.

### build_amalgamation
 This solution builds the amalgamated (installer_.h and installer_.c) versions of all files needed.
 
 ### make_instaler_zip_src
 This solution creates the installer_src.zip that is the source code that the user must  include in their project to create installers.
 
 ### make_install_zip
 This solution creates the files.zip that contains all files that are copied during instalation. We must create files.zip before run installer
 
 ### installer
   This project creates the instaler. It uses the files.zip created by make_install_zip. It alo needs uninstall.exe created bt uninstall solution
 
 ### installer_debug
   The same of installer but we can use the not almagamated version for easy edit.
 
 ### installer
   This project create uninstall.exe that is included inside the instalation and it is used when the program is unistalled.
   
