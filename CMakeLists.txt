cmake_minimum_required(VERSION 2.6)

project(statusspec)

set(PACKAGE "statusspec")
set(PACKAGE_VERSION "v0.1")
set(PACKAGE_BUGREPORT "https://github.com/fwdcp/StatusSpec/issues")
set(PACKAGE_NAME "${PACKAGE}")
set(PACKAGE_STRING "${PACKAGE} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}_${PACKAGE_VERSION}")
set(PACKAGE_URL "https://github.com/fwdcp/StatusSpec")
set(VERSION "${PACKAGE_VERSION}")

include_directories(${PROJECT_BINARY_DIR})
# Put final product in 'bin' folder
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Include source and hl2sdk
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${HL2SDK}/common)
include_directories(${HL2SDK}/public)
include_directories(${HL2SDK}/public/tier0)
include_directories(${HL2SDK}/public/tier1)
include_directories(${HL2SDK}/game/client)
include_directories(${HL2SDK}/game/shared)

set(HDR_PUBLIC
	src/statusspec.h
	src/offsets.h
	src/stdafx.h
	src/vfuncs.h
	)

set(SOURCES
	src/statusspec.cpp
	src/offsets.cpp
	src/vfuncs.cpp
	)

source_group("Header Files" FILES ${HDR_PUBLIC})
source_group("Source FIles" FILES ${SOURCES})

# Setup defines, compiler options and linker options for each platform
if(UNIX) # Common Unix settings
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		# Malloc fix
		include_directories(/usr/include/malloc)
		set(SymbolVisibility "hidden")
		set(GCC_ExtraCompilerFlags "")
		set(GCC_ExtraLinkerFlags "")
		set(WarnFlags "-Wall -Wextra -Wshadow -Wno-invalid-offsetof  -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-value -Wno-missing-field-initializers -Wno-sign-compare -Wno-reorder -Wno-invalid-offsetof -Wno-float-equal -Werror=return-type -fdiagnostics-show-option -Wformat -Wformat-security")
		
		#Set Optimizer Level
		if(CMAKE_BUILD_TYPE MATCHES DEBUG)
			set(OptimizerLevel "-gdwarf-2 -g -O0")
			add_definitions(-DDEBUG -D_DEBUG -DGNUC -DPOSIX -D_OSX -DOSX -D_DARWIN_UNLIMITED_SELECT -DFD_SETSIZE=10240 -DQUICKTIME_VIDEO -DFORCE_QUICKTIME -DGL_GLEXT_PROTOTYPES -DDX_TO_GL_ABSTRACTION -D_MBCS -D_DLL_EXT=.dylib -D_POSIX=1)
		else()
			set(OptimizerLevel "-gdwarf-2 -g -O2 -fno-strict-aliasing -ffast-math -fno-omit-frame-pointer")
			add_definitions(-DNDEBUG -DGNUC -DPOSIX -D_OSX -DOSX -D_DARWIN_UNLIMITED_SELECT -DFD_SETSIZE=10240 -DQUICKTIME_VIDEO -DFORCE_QUICKTIME -DGL_GLEXT_PROTOTYPES -DDX_TO_GL_ABSTRACTION -D_MBCS -D_DLL_EXT=.dylib -D_POSIX=1)
		endif()
		
		set(CFLAGS "-arch i386 -m32 -march=prescott -momit-leaf-frame-pointer -mtune=core2 ${WarnFlags} -fvisibility=${SymbolVisibility} ${OptimizerLevel} -pipe ${GCC_ExtraCompilerFlags} -Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE -mmacosx-version-min=10.5 -fasm-blocks")
		set(COMPILER_FLAGS ${CFLAGS})
		add_definitions(-DVPROF_LEVEL=1 -DGNUC -DNO_HOOK_MALLOC -DNO_MALLOC_OVERRIDE)
		set(LINKER_FLAGS "${CFLAGS} ${GCC_ExtraLinkerFlags} ${OptimizerLevel} -dynamiclib -current_version 1.0 -compatibility_version 1.0 -Wl,-dead_strip -Wl,-no_dead_strip_inits_and_terms")
	else() # Linux
		set(SymbolVisibility "hidden")
		set(GCC_ExtraCompilerFlags "-U_FORTIFY_SOURCE")
		set(GCC_ExtraLinkerFlags "")
		set(WarnFlags "-Wall -Wextra -Wshadow -Wno-invalid-offsetof -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-value -Wno-missing-field-initializers -Wno-sign-compare -Wno-reorder -Wno-invalid-offsetof -Wno-float-equal -Werror=return-type -fdiagnostics-show-option -Wformat -Wformat-security")
		
		# Set Optimizer Level
		if(CMAKE_BUILD_TYPE MATCHES DEBUG)
			set(OptimizerLevel "-gdwarf-2 -g -O0")
			add_definitions(-DDEBUG -D_DEBUG -DPOSIX -DGNUC -DLINUX -D_LINUX -DRAD_TELEMETRY_DISABLED -DBINK_VIDEO -DGL_GLEXT_PROTOTYPES -DDX_TO_GL_ABSTRACTION -DUSE_SDL -D_MBCS -D_EXTERNAL_DLL_EXT=.so -D_DLL_EXT=.so -D_LINUX=1 -D_POSIX=1 -DLINUX=1 -DPOSIX=1)
		else()
			set(OptimizerLevel "-gdwarf-2 -g -O2 -fno-strict-aliasing -ffast-math -fno-omit-frame-pointer -ftree-vectorize -fpredictive-commoning -funswitch-loops")
			add_definitions(-DNDEBUG -DPOSIX -DGNUC -DLINUX -D_LINUX -DRAD_TELEMETRY_DISABLED -DBINK_VIDEO -DGL_GLEXT_PROTOTYPES -DDX_TO_GL_ABSTRACTION -DUSE_SDL -D_MBCS -D_EXTERNAL_DLL_EXT=.so -D_DLL_EXT=.so -D_LINUX=1 -D_POSIX=1 -DLINUX=1 -DPOSIX=1)
		endif()
		
		set(CFLAGS "-m32 -march=pentium4 -mtune=core2 -msse2 -mfpmath=sse ${WarnFlags} -fvisibility=${SymbolVisibility} ${OptimizerLevel} -pipe ${GCC_ExtraCompilerFlags} -Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE")
		set(COMPILER_FLAGS ${CFLAGS})
		add_definitions(-DVPROF_LEVEL=1 -DGNUC -DNO_HOOK_MALLOC -DNO_MALLOC_OVERRIDE)
		set(LINKER_FLAGS "${CFLAGS} ${GCC_ExtraLinkerFlags} ${OptimizerLevel} -Wl,--build-id -shared -Wl,--no-undefined -Wl,-Map,$(@:.so=).map -static-libgcc -Wl,--start-group -Wl,--end-group -lm -ldl -lpthread -l:ld-linux.so.2")
	endif()
else() # Windows
	# Force use of static libs
	# Credit SteveL & brofield on StackOverflow
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINOFO)
		string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
	endforeach()
	
	add_definitions(/D_CRT_SECURE_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE)
	set(COMPILER_FLAGS "/EHsc /DR- /W3 /nologo /Zi /TP")
	set(LINKER_FLAGS "/MACHINE:X86 /subsystem:windows /NODEFAULTLIB:libc /NODEFAULTLIB:libcd")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libcmt")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")

# Create the plugin
add_library(statusspec SHARED
	${HDR_PUBLIC}
	${SOURCES})

# Remove 'lib' prefix
set_target_properties(statusspec PROPERTIES PREFIX "")

# Link sdk libraries
if(WIN32)
	target_link_libraries(statusspec ${HL2SDK}/lib/public/tier0.lib)
	target_link_libraries(statusspec ${HL2SDK}/lib/public/tier1.lib)
else()
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		target_link_libraries(statusspec ${HL2SDK}/lib/public/osx32/libtier0.dylib)
		target_link_libraries(statusspec ${HL2SDK}/lib/public/osx32/tier1.a)
	else()
		target_link_libraries(statusspec ${HL2SDK}/lib/public/linux32/libtier0.so)
		target_link_libraries(statusspec ${HL2SDK}/lib/public/linux32/tier1.a)
	endif()
	
	target_link_libraries(statusspec dl)
endif()
