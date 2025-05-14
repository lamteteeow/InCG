cmake_minimum_required (VERSION 3.0.2)
# allows access to environment variables with the name 
project(INCG)

# include glm
include_directories(BEFORE SYSTEM ${LOCAL_EXTERNAL_DIR}/glm-0.9.9.1)

include(FindExternal)

# find_external(objloader "libobjloader/objloader.h" "objloader")

# create cgl library 
file(GLOB FRAMEWORK_SOURCES "${PROJECT_SOURCE_DIR}/source/*.cpp" "${PROJECT_SOURCE_DIR}/source/*.c")
file(GLOB FRAMEWORK_HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp" "${PROJECT_SOURCE_DIR}/include/*.h")
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(incg STATIC ${FRAMEWORK_SOURCES} ${LODEPNG_SOURCES})
target_include_directories(incg PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_compile_options(incg PRIVATE -Wno-unused-parameter)
target_link_libraries(incg glbinding glfw ${GLFW_LIBRARIES} )
# look for libraries in own folder
set_target_properties(incg PROPERTIES INSTALL_RPATH "$ORIGIN/")
# header list needs to be in quotes
set_target_properties(incg PROPERTIES PUBLIC_HEADER "${FRAMEWORK_HEADERS}")
install(
      TARGETS incg
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      PUBLIC_HEADER DESTINATION include/incg
  )

# mark_as_advanced(OPTION_BUILD_TESTS OPTION_BUILD_TOOLS OPTION_BUILD_GPU_TESTS
#  OPTION_BUILD_WITH_BOOST_THREAD OPTION_SELF_CONTAINED OPTION_BUILD_DOCS
#  OPTION_BUILD_EXAMPLES PYTHON)
