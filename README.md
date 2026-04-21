# CCP Module - PDM

Platform Detection Module (PDM) is an OS agnostic library for gathering metrics about the current machine.

## Building

### Windows - Visual Studio 2019
Select `Open in Visual Studio` under the `Clone or download` menu in the top right corner.  
Visual studio 2019 should show up and give you the option to clone the repo.  
Once the repo is cloned and cmake has finished initializing, select `pdmCLI.exe` under the `Select startup item` dropdown menu.  
You should now be able to compile and run the CLI application.  

Note that PDM depends on the [C++ 2017 redistibutable package](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads).  
In other words `vcruntime140.dll`, `vcruntime140_1.dll`, and `msvcp140.dll`.

### macOS - make
Open a terminal and git clone the repo, and CD into the root of the cloned repo.  
Make a new folder called build and CD into it.  
Run following cmake command: `cmake ..`  
Run make with: `make`  
Run the command line application with `./pdmCLI`

### macOS - Xcode
Open a terminal, git clone the repo, and CD into the root of the cloned repo.  
Make a new folder called build and CD into it.  
Run following cmake command: `cmake -G Xcode ..`  
Use Xcode to open `pdm.xcodeproj` which was created in the build folder.  
Select `pdmCLI` under the build target dropbown menu.  
You should now be able to compile and run the CLI application.

## 🤝 Contributing
Contribution follows the standard GIT PR model.

By submitting a pull request or otherwise contributing to this project, you agree to license your contribution under the (to be determined) License, and you confirm that you have the right to do so.

## 📄 License and Legal Notices

© 2026 CCP Games 

This software is provided by CCP Games and does not include or distribute any third-party libraries or frameworks. 

This software is a Platform Detection Module, an OS agnostic library for data collection

Trademark Notice: CCP Games is a trademark of CCP ehf. 

This project is licensed under the [MIT License](LICENSE.md). Nothing in the [MIT License](LICENSE.md) grants any rights to CCP Games' trademarks or game content.
