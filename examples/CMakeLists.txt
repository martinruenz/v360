cmake_minimum_required(VERSION 2.8)

project (examples)
set(CMAKE_BUILD_TYPE Release)


if(WIN32)
	## MODIFY HERE: ##
	set(PYTHON_PATH C:/Python34)
	set(PYTHON_EXECUTABLE ${PYTHON_PATH}/python.exe)
	set(PYTHON_INCLUDE_DIR ${PYTHON_PATH}/include)
	#set(PYTHON_LIBRARY ${PYTHON_PATH}/libs)
    set(PYTHON_LIBRARY ${PYTHON_PATH}/libs/python34.lib)
else()
	SET(PYTHON_INCLUDE_PATH /usr/include/python3.4)
	SET(PYTHON_LIBRARIES /usr/lib/x86_64-linux-gnu/libpython3.4m.so)
	SET(PYTHON_EXECUTABLE /usr/bin/python3.4)
	SET(PYTHON_INCLUDE_DIR  /usr/include/python3.4)
endif()

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib-debug)
#    set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib-debug)
#    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin-dbg)
#else()
#    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#    set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib)
#    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
#endif()

#find_package(PythonLibs 3.4 EXACT REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})
#LINK_DIRECTORIES(${PYTHON_LIBRARY})


add_executable (run_camera run_camera.cpp)

if(WIN32)
	target_link_libraries(run_camera ${PYTHON_LIBRARY}/python34.lib)
else()
	target_link_libraries(run_camera ${PYTHON_LIBRARIES})
endif()

#target_link_libraries(run_camera ${PYTHON_LIBRARY}/python34.lib)
#target_link_libraries(example C:/Python34/libs/python34.lib)

# Recurse into the "Hello" and "Demo" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
#add_subdirectory (examples)


