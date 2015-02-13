cmake_minimum_required(VERSION 2.8)

project (examples)
set(CMAKE_BUILD_TYPE Release)

###### ###### MODIFY HERE: ###### ######
if(WIN32)
	set(PYTHON_PATH C:/Python34)
	set(PYTHON_EXECUTABLE ${PYTHON_PATH}/python.exe)
	set(PYTHON_INCLUDE_DIR ${PYTHON_PATH}/include)
    set(PYTHON_LIBRARY ${PYTHON_PATH}/libs/python34.lib)
else()
	SET(PYTHON_INCLUDE_PATH /usr/include/python3.4)
	SET(PYTHON_LIBRARY /usr/lib/x86_64-linux-gnu/libpython3.4m.so)
	SET(PYTHON_EXECUTABLE /usr/bin/python3.4)
	SET(PYTHON_INCLUDE_DIR  /usr/include/python3.4)
endif()
###### ###### ###### ###### ###### ######

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${PYTHON_INCLUDE_DIR})

add_executable (run_camera run_camera.cpp)

target_link_libraries(run_camera ${PYTHON_LIBRARY})

