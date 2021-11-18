cmake_minimum_required(VERSION 3.1)
project(pyqram)

set(LIB_QRAMSimulator pyqram)
file(GLOB_RECURSE QRAMSimulator_SRC_CPP *.cpp)
add_library(${LIB_QRAMSimulator} SHARED ${QRAMSimulator_SRC_CPP})
if (UNIX)
    if (APPLE)
        set_target_properties(${LIB_QRAMSimulator} PROPERTIES SUFFIX ".so")
    endif(APPLE)
    set_target_properties(${LIB_QRAMSimulator} PROPERTIES PREFIX "")
elseif(WIN32)
    if (NOT MSVC)
        set_target_properties(${LIB_QRAMSimulator} PROPERTIES PREFIX "")
    endif()
    set_target_properties(${LIB_QRAMSimulator} PROPERTIES SUFFIX ".pyd")
endif(UNIX)

target_link_libraries(${LIB_QRAMSimulator} QRAMSimulator fmt ${PYTHON_LIBRARIES})

set(LIB_QRAMSimulator_OUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../pyqram")
if (UNIX)
    set(LIB_QRAMSimulator_OUT_LIB "${LIB_QRAMSimulator}.so")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBRARY_OUTPUT_PATH})
elseif (WIN32)
    set(LIB_QRAMSimulator_OUT_LIB "${LIB_QRAMSimulator}.pyd")
endif(UNIX)

add_custom_command(OUTPUT ${LIB_QRAMSimulator_OUT_LIB}
   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/${LIB_QRAMSimulator_OUT_LIB} ${LIB_QRAMSimulator_OUT_PATH}/${LIB_QRAMSimulator_OUT_LIB}
 )
add_custom_target(CopyTask ALL DEPENDS ${LIB_QRAMSimulator_OUT_LIB} ${LIB_QRAMSimulator})
