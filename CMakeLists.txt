# Convery (tcn@ayria.se)
cmake_minimum_required(VERSION 3.1)
project(Desktop_cpp)

# Use the latest standard at this time.
set(CMAKE_CXX_STANDARD 17)
enable_language(ASM_MASM)

# Export to the a gitignored directory.
set(EXECUTABLE_OUTPUT_PATH ${Desktop_cpp_SOURCE_DIR}/Bin)
set(LIBRARY_OUTPUT_PATH ${Desktop_cpp_SOURCE_DIR}/Bin)

# Separate release and debugbuilds output and dependencies.
mark_as_advanced(CMAKE_IMPORT_LIBRARY_SUFFIX CMAKE_DEBUG_POSTFIX)
set(CMAKE_DEBUG_POSTFIX "d")

# Platform libraries to be linked.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CORE_LIBS dl pthread)
else()
	set(CORE_LIBS gdiplus)
endif()

# Extra flags for compiling and linking.
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		set(EXTRA_CMPFLAGS ${EXTRA_CMPFLAGS} "-m32")
		set(EXTRA_LNKFLAGS ${EXTRA_LNKFLAGS} "-m32 -static-libgcc -static-libstdc++")
	else()
		set(EXTRA_CMPFLAGS ${EXTRA_CMPFLAGS} "-m64")
		set(EXTRA_LNKFLAGS ${EXTRA_LNKFLAGS} "-m64 -static-libgcc -static-libstdc++")
    endif()
else()
    set(EXTRA_CMPFLAGS ${EXTRA_CMPFLAGS} "/permissive- /Gy /Gv /Gw /GS- /GL")
    set(EXTRA_LNKFLAGS ${EXTRA_LNKFLAGS} "/subsystem:windows /entry:mainCRTStartup /LTCG /safeseh:no /filealign:512 /opt:ref /incremental:no")
endif()

# Set warningflags per compiler.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# Enforce static VC runtimes when building for Windows.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    foreach(flag_var
			CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

# Third-party libraries, prefer find_library() and find_path().
find_path(HASHMAP_INC parallel_hashmap)
find_path(JSON_INC nlohmann)
find_path(ABSL_INC absl)
set(MODULE_LIBS )
include_directories(${JSON_INC} ${HASHMAP_INC} ${ABSL_INC})

# Just pull all the files from /Source
file(GLOB_RECURSE SOURCES "Source/*.cpp")
file(GLOB_RECURSE ASSEMBLY "Source/*.asm")
add_definitions(-DMODULENAME="${PROJECT_NAME}")
add_executable(${PROJECT_NAME} ${SOURCES} ${ASSEMBLY})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} ${CORE_LIBS} ${MODULE_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${EXTRA_CMPFLAGS}" LINK_FLAGS "${EXTRA_LNKFLAGS}")
