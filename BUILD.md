Building StatusSpec plugin
==========================

Notice
------
StatusSpec is not yet available for Linux & OS X.

Requirements
------------
* **[Windows]** Visual Studio or Visual Express C++
* Source SDK 2013 - https://github.com/ValveSoftware/source-sdk-2013
* Metamod:Source - https://github.com/alliedmodders/metamod-source

Building
--------
1. Clone this repository in `/mp/src/utils` in source-sdk-2013.
2. Clone Metamod:Source in your directory of choice.
3. Adjust the `MMSOURCE_DEV` macro in `statusspec.vpc` to the location of Metamod:Source.
4. Copy `build_tools/createstatusspec.bat` or `build_tools/createstatusspec` (depending on platform) to `mp/src` in source-sdk-2013.
5. Add the contents of `build_tools/project.vgc` to `mp/src/vpc_scripts/projects.vgc` in source-sdk-2013.
6. Run the script file you copied in step #2 to generate project files.
7. Build the project with the project files that were generated.
