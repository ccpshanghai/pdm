# CCP Module - PDM

Platform Detection Module (PDM) is an OS agnostic library for gathering metrics about the current machine.
More information about the module can be found on its [feature page](https://wiki.ccpgames.com/display/TE/Platform+Detection+Module).

## Building

### Windows
Select `Open in Visual Studio` under the `Clone or download` menu in the top right corner.
Visual studio 2019 should show up and give you the option to clone the repo.
Once the repo is cloned and cmake has finished initializing, select `pdmCLI.exe` under the `Select startup item` dropdown menu.
You should now be able to compile and run the CLI application.

Note that PDM depends on the [C++ 2017 redistibutable package](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads).
In other words `vcruntime140.dll`, `vcruntime140_1.dll`, and `msvcp140.dll`.

### macOS
Open a terminal, git clone the repo, and CD into the root of the cloned repo.
Make a new folder called build and CD into it.
Run following cmake command: `cmake -G Xcode ..`
Use XCode to open `pdm.xcodeproj` which was created in the build folder.
Select `pdmCLI` under the build target dropbown menu.
You should now be able to compile and run the CLI application.