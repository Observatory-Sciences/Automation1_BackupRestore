cmake_minimum_required(VERSION 3.10)

project(BackupRestore VERSION 0.10
		DESCRIPTION "Tool to backup and restore files and configuration settings on an Aerotech Automation1 iSMC."
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True) #force c++11+

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})       #output the exe to source dir (one level up from build dir)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)   #output libs to lib folder

configure_file(BackupRestoreConfig.h.in ${PROJECT_SOURCE_DIR}/include/BackupRestoreConfig.h)

add_subdirectory(src)
list(APPEND EXTRA_LIBS src)

link_directories("${CMAKE_SOURCE_DIR}/lib")
add_executable(BackupRestore ${CMAKE_SOURCE_DIR}/src/backuprestore.cxx)

target_link_libraries(BackupRestore PUBLIC "${CMAKE_SOURCE_DIR}/lib/libautomation1c.so")
target_link_libraries(BackupRestore PUBLIC "${CMAKE_SOURCE_DIR}/lib/libautomation1compiler.so")

target_link_libraries(BackupRestore PUBLIC readline)            # readline library
target_link_libraries(BackupRestore PUBLIC ${EXTRA_LIBS})

target_include_directories(BackupRestore PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           "${PROJECT_SOURCE_DIR}/include"
                           "${PROJECT_SOURCE_DIR}/src"
                           )

install(TARGETS BackupRestore DESTINATION ${PROJECT_BINARY_DIR}/bin)
install(FILES "${PROJECT_BINARY_DIR}/BackupRestoreConfig.h"
  DESTINATION ${PROJECT_SOURCE_DIR}/include
  )