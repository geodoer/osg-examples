cmake_minimum_required(VERSION 2.8.12)
project(simple)

set(OSG_DIR "D:/install/OpenSceneGraph-3.6.5-VC2022-64-Debug")
find_package(OpenSceneGraph REQUIRED osgDB osgUtil osgViewer)
    # libOpenThreads & libosg automatically searched

#
# 创建子工程
# 
set(Project_Src_Dir ${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE files "${Project_Src_Dir}/*.cpp")
foreach(file ${files})
  #工程名
  get_filename_component(name ${file} NAME_WE)
  
  #工程文件夹
  string(REGEX MATCH "${Project_Src_Dir}(.+)/${name}.+" DIR ${file})

  # message(STATUS "${file} ==== ${name} ==== ${DIR}")

  #创建工程 
	add_executable(${name} ${file})
  target_include_directories(${name} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
	target_link_libraries(${name} PRIVATE ${OPENSCENEGRAPH_LIBRARIES})

  # 子工程的文件夹
  # set_property(TARGET ${name} PROPERTY FOLDER ${DIR})
  # 代码分组
  # source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${Project_Headers_Files})

  #数据路径
  target_compile_definitions(${name} 
  PRIVATE 
    "-DDATA_PATH=\"${CMAKE_SOURCE_DIR}/../data/\""
  )
endforeach()

#
# 其他配置
#

# 开启工程分组
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_options(-fno-elide-constructors) #关闭编译器优化