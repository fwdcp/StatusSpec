Developing StatusSpec
=====================

Notice
------
StatusSpec is not yet compatible with Linux & OS X. Due to the advanced features of StatusSpec, no support for platforms other than Windows is planned in the near future.

Requirements
------------
### Installed Software ###
* **[Windows]** Visual Studio 2013
* Python 2.6+

### Code Resources ###
* Source SDK 2013 - https://github.com/ValveSoftware/source-sdk-2013
* Metamod:Source - https://github.com/alliedmodders/metamod-source
* MinHook - https://github.com/TsudaKageyu/minhook
* RapidJSON - https://github.com/miloyip/rapidjson

Setup
-----
1. Move, copy, or link this repository as necessary such that the files of this repository are located under `mp/src/utils/StatusSpec` within the Source SDK 2013 repository.
2. Build the MinHook project in both Debug and Release configurations with one of the included Visual Studio project directories within its `build` directory.
3. Run `amalgamate.py` within JsonCpp to generate include and source files.
4. Adjust the `MMSOURCE_DEV` macro in `statusspec.vpc` to the path of the Metamod:Source repository on disk.
5. Adjust the `MINHOOK` macro in `statusspec.vpc` to the path of MinHook repository on disk.
6. Adjust the `MINHOOK_BUILD` macro in `statusspec.vpc` to the path of the Visual Studio project directory you used to build MinHook in step #2.
7. Adjust the `RAPIDJSON` macro in `statusspec.vpc` to the path of the RapidJSON include directory (usually the `include` directory in the JsonCpp repository).
8. Add the contents of this repository's `build_tools/project.vgc` to `mp/src/vpc_scripts/projects.vgc` within the Source SDK 2013 repository.
9. Copy this repository's `build_tools/createstatusspec.bat` or `build_tools/createstatusspec` (depending on platform) to `mp/src` within the Source SDK 2013 repository.
10. Run the script file you copied in the previous step to generate platform-appropriate project files.
