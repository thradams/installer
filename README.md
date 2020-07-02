# Embedded Install System - Zero dependencies


This project creates source file (installer_src.zip) that contains source code with points of customization
were you can create the installer for yor windows project without any other dependency.

The objetive of this project is help the creation and test of the sources. installer_src.zip. In other words
if you want to inprove the installer (adding more features etc) and help other people.

If you only need use the installer then all you need is installer_src.zip.

## Using it

First you need to copy the folder installer into your project.

![1](img1.png)
![2](img2.png)
![3](img3.png)
![4](img4.png)
![5](img5.png)
![6](img6.png)
![7](img7.png)



## Solutions

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
   
