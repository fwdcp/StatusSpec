Building AdvSpec plugin
==============================

Notice
------
AdvSpec is not yet available for Linux & OS X.

Requirements
------------
* **[Windows]** Visual Studio or Visual Express C++
* source-sdk-2013 - https://github.com/ValveSoftware/source-sdk-2013/

Building
--------
1. Clone this repository in `/mp/src/utils` in source-sdk-2013.
2. Copy `build_tools/createstatusspec.bat` or `build_tools/createstatusspec` (depending on platform) to `mp/src` in source-sdk-2013.
3. Add the contents of `build_tools/project.vgc` to `mp/src/vpc_scripts/projects.vgc` in source-sdk-2013.
4. Run the script file you copied in step #2 to generate project files.
5. Build the project with the project files that were generated.
