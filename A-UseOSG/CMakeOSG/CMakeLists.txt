cmake_minimum_required(VERSION 2.8.12)
project(CMakeOSG)

set(OSG_DIR "D:/install/OpenSceneGraph-3.6.5-VC2022-64-Debug")
find_package(OpenSceneGraph REQUIRED osgDB osgUtil osgViewer)
    # libOpenThreads & libosg automatically searched

set(Project_Src_Dir ${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE files "${Project_Src_Dir}/*.cpp")
add_executable(${PROJECT_NAME} ${files})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSCENEGRAPH_LIBRARIES})