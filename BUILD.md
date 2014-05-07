Building AdvSpec plugin
==============================

Notice
-
AdvSpec is not yet available for Linux & OS X.

Requirements
-
* **[Windows]** Visual Studio or Visual Express C++
* cmake 2.6+
* source-sdk-2013 - https://github.com/ValveSoftware/source-sdk-2013/

Building
-  
**Windows**  
`cmake -DHL2SDK=c:\path\to\source-sdk-2013\mp\src -G "Visual Studio 10" ..`  
Then open in Visual Studio/C++ Express and build. Note the `\mp\src` in the SDK path!  
  
Open advspec properties, then in `Linker -> Input -> Ignore Specific Default Libraries`, change `libcmt`, to `libcmtd`  
Do this for Debug and Release.
