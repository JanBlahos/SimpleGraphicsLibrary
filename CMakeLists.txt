project(sgl)
cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE SGL_CXX "src/*.cpp")
add_library(${PROJECT_NAME} STATIC ${SGL_CXX} "include/context.h" "src/context.cpp"   "src/contextManager.cpp" "include/contextManager.h" "include/matrixStack.h" "src/matrixStack.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SGL_CXX})

target_include_directories(${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS ${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  ARCHIVE DESTINATION lib ${CMAKE_INSTALL_LIBDIR}
)
