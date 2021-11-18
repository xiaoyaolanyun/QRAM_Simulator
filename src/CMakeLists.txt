project(QRAMSimulator)

file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(${PROJECT_NAME} STATIC ${src})
# target_link_libraries(${PROJECT_NAME} fmt)
# target_link_libraries(${PROJECT_NAME})