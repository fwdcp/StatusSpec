Building StatusSpec plugin
==========================

Notice
------
StatusSpec is not yet compatible with Linux & OS X. Due to the advanced features of StatusSpec, no support for platforms other than Windows is planned in the near future.

Requirements
------------
* **[Windows]** Visual Studio or Visual Express C++
* Source SDK 2013 - https://github.com/ValveSoftware/source-sdk-2013
* Metamod:Source - https://github.com/alliedmodders/metamod-source
* MinHook - https://github.com/alliedmodders/metamod-source

Building
--------
1. Clone this repository in `/mp/src/utils` in source-sdk-2013.
2. Clone Metamod:Source in your directory of choice.
3. Clone MinHook in your directory of choice.
4. Adjust the `MMSOURCE_DEV` macro in `statusspec.vpc` to the location of Metamod:Source.
5. Adjust the `MINHOOK` macro in `statusspec.vpc` to the location of MinHook.
6. Copy `build_tools/createstatusspec.bat` or `build_tools/createstatusspec` (depending on platform) to `mp/src` in source-sdk-2013.
7. Add the contents of `build_tools/project.vgc` to `mp/src/vpc_scripts/projects.vgc` in source-sdk-2013.
8. Run the script file you copied in step #2 to generate project files.
9. Build the project with the project files that were generated.
