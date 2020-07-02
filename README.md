# Embedded Install System - Zero dependencies


This project creates source file (installer_src.zip) that contains source code with points of customization
were you can create the installer for yor windows project without any other dependency.

The objetive of this project is help the creation and test of the sources. installer_src.zip. In other words
if you want to inprove the installer (adding more features etc) and help other people.

If you only need use the installer then all you need is installer_src.zip.

## Using it

First you need to copy the folder installer into your project. This folder is inside **installer_src.zip**

![1](i1.png)

Then ADD into your project installer , make_install_zip and unistall

![2](i2.png)

![3](i3.png)

You will see this
![4](i4.png)

You can put these files inside a folder
![5](i5.png)

![6](i6.png)

Then edit the config.h you product details like names, id.
Run the project in a especific order. First build zip files then unistall then install. Add your exe and all other files you need.

![7](i7.png)


You can especify the build order in your project and also compilation step to run make_install_zip.

Build order
```
 1) Your program
 2) unistall (creates the unistall program)
 3) make_install_zip (creates the files.zip with your program the unistall program and all your files)
   You also needs to run make_install_zip
 4) installer (this will create the intaller)

```

## Solutions

In case you contribute for this project then you need to understand what each project do:

### build_amalgamation
 This solution builds the amalgamated (installer_.h and installer_.c) versions of all files needed.
 
 ### make_instaler_zip_src
 This solution creates the installer_src.zip that is the source code that the user must  include in their project to create installers.
 
 ### make_install_zip
 This solution creates the files.zip that contains all files that are copied during instalation. We must create files.zip before run installer
 
 ### installer
   This project creates the instaler. It uses the files.zip created by make_install_zip. It alo needs uninstall.exe created bt uninstall solution
      
 ### installer
   This project create uninstall.exe that is included inside the instalation and it is used when the program is unistalled.
   
