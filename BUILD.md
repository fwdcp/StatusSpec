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
* MinHook - https://github.com/TsudaKageyu/minhook
* JsonCpp - https://github.com/open-source-parsers/jsoncpp

Building
--------
1. Clone this repository in `/mp/src/utils` in source-sdk-2013.
2. Clone Metamod:Source in your directory of choice.
3. Clone MinHook in your directory of choice.
4. Clone JsonCpp in your directory of choice.
5. Build the MinHook project in both Debug and Release configurations with one of the included Visual Studio project directories within its `build` directory.
6. Run `amalgamate.py` within JsonCpp to generate include and source files.
7. Adjust the `MMSOURCE_DEV` macro in `statusspec.vpc` to the location of Metamod:Source.
8. Adjust the `MINHOOK` macro in `statusspec.vpc` to the location of MinHook.
9. Adjust the `MINHOOK_BUILD` macro in `statusspec.vpc` to the location of the MinHook Visual Studio project directory you selected.
10. Adjust the `JSONCPP` macro in `statusspec.vpc` to the location of the generated JsonCpp files (usually within the `dist` directory of the source directory).
11. Copy `build_tools/createstatusspec.bat` or `build_tools/createstatusspec` (depending on platform) to `mp/src` in source-sdk-2013.
12. Add the contents of `build_tools/project.vgc` to `mp/src/vpc_scripts/projects.vgc` in source-sdk-2013.
13. Run the script file you copied in step #2 to generate project files.
14. Build the project with the project files that were generated.