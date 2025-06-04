#general script to search for dependency libraries and headers 
##############################################################################
# wobakj@web.de
# input and output variable names are not in caps nad case sensitive
##############################################################################
#input variables
# DEP - dependency id
# DEP_HEADER - header file to search for
# DEP_LIBRARY - library file to search for
# ARGN - additional library names to put int DEP_LIBRARIES
# [SILENT] - whether to prompt an input request for search dirs if not found
##############################################################################
#path hints
#EXTERNALS_DIR
#DEP_INCLUDE_SEARCH_DIR
#DEP_LIBRARY_SEARCH_DIR
##############################################################################
#output variables
#DEP_INCLUDE_DIRS
#DEP_LIBRARIES
#DEP_LIBRARY_DIRS
#DEP_FOUND
##############################################################################
function(find_external DEP DEP_HEADER DEP_LIB)
##############################################################################
# macros
##############################################################################
# feedback to provide user-defined paths to search for ${DEP}
MACRO(request_include_search_dirs)
  # if hints were provided, display
  if (DEFINED ${DEP}_INCLUDE_SEARCH_DIR)
    set(${DEP}_INCLUDE_SEARCH_DIR ${${DEP}_INCLUDE_SEARCH_DIR} CACHE PATH "path to ${DEP} headers.")
    # if not, request them
  else()
    set(${DEP}_INCLUDE_SEARCH_DIR "Please provide ${DEP} include path." CACHE PATH "path to ${DEP} headers.")
  endif()
  message(FATAL_ERROR "Unable to find ${DEP} headers, please specify the correct ${DEP}_INCLUDE_SEARCH_DIR.")
ENDMACRO()

MACRO(request_lib_search_dirs)
  # if hints were provided, display
  if (DEFINED ${DEP}_LIBRARY_SEARCH_DIR)
    set(${DEP}_LIBRARY_SEARCH_DIR ${${DEP}_LIBRARY_SEARCH_DIR} CACHE PATH "path to ${DEP} libraries.")
    # if not, request them
  else()
    set(${DEP}_LIBRARY_SEARCH_DIR "Please provide ${DEP} library path." CACHE PATH "path to ${DEP} libraries.")
  endif()
  message(FATAL_ERROR "Unable to find ${DEP} library '${_SEARCH_LIB}', please specify the correct ${DEP}_LIBRARY_SEARCH_DIR.")
ENDMACRO()

MACRO(create_import_lib)
  # create imported library if version is high enough, from version 3.0.1 on
  if(${CMAKE_MAJOR_VERSION} GREATER 3 OR (${CMAKE_MAJOR_VERSION} GREATER 2 AND ${CMAKE_MINOR_VERSION} GREATER 0 OR (${CMAKE_MINOR_VERSION} GREATER -1 AND ${CMAKE_PATCH_VERSION} GREATER 1)))
    add_library(${DEP} INTERFACE IMPORTED GLOBAL)
    set_property(TARGET ${DEP} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${${DEP}_INCLUDE_DIRS})
    set_property(TARGET ${DEP} PROPERTY INTERFACE_LINK_LIBRARIES "${${DEP}_LIBRARIES}")
  endif()
ENDMACRO()
##############################################################################
# start of actual function
# skip if dependency was already found
##############################################################################
if(${DEP}_FOUND)
  # import lib target needs ot be created during every invocation
  create_import_lib()
  return()
endif(${DEP}_FOUND)
##############################################################################
# search paths
##############################################################################
# create list of libs from lib and optional arguments
set(_EXTRA_ARGS ${ARGN})
# search for "SILENT" string
list(FIND _EXTRA_ARGS "SILENT" _INDEX)
if(NOT ${_INDEX} LESS 0)
  set(FIND_SILENT "TRUE")
  list(REMOVE_ITEM _EXTRA_ARGS "SILENT")
else()
  set(FIND_SILENT "FALSE")
endif()
##############################################################################
# search paths
##############################################################################
# search before global environment paths
list(APPEND ${DEP}_INCLUDE_SEARCH_HINTS
  ${${DEP}_INCLUDE_DIRS}
  ${${DEP}_INCLUDE_SEARCH_DIR}
  ${EXTERNALS_DIR}/inc
  ${EXTERNALS_DIR}/include
  ${EXTERNALS_DIR}/inc/${DEP}
  ${EXTERNALS_DIR}/include/${DEP}
  ${EXTERNALS_DIR}/${DEP}/include
)
# search after global environment paths
list(APPEND ${DEP}_INCLUDE_SEARCH_PATHS
  "/usr/include"
  "/usr/share/include"
  "/usr/include/x86_64-linux-gnu/"
)

list(APPEND ${DEP}_LIBRARY_SEARCH_HINTS
  ${${DEP}_LIBRARY_DIRS}
  ${${DEP}_LIBRARY_SEARCH_DIR}
  ${EXTERNALS_DIR}/lib
  ${EXTERNALS_DIR}/${DEP}/lib
)
list(APPEND ${DEP}_LIBRARY_SEARCH_PATHS
  "/usr/lib/"
  "/usr/lib/x86_64-linux-gnu/"
)
##############################################################################
# search
##############################################################################
message(STATUS "Looking for ${DEP}")
# search for include directory
if (NOT ${DEP}_INCLUDE_DIRS)

  find_path(${DEP}_INCLUDE_DIRS
    NAMES ${DEP_HEADER}
    HINTS ${${DEP}_INCLUDE_SEARCH_HINTS}
    PATHS ${${DEP}_INCLUDE_SEARCH_PATHS}
    PATH_SUFFIXES release debug
  )

  if(${DEP}_INCLUDE_DIRS)
    # add description in cache
    set(${DEP}_INCLUDE_DIRS ${${DEP}_INCLUDE_DIRS} CACHE PATH "${DEP} include directory.")
    # remove search dir var since headers were found
    unset(${DEP}_INCLUDE_SEARCH_DIR CACHE)
  else()
    unset(${DEP}_INCLUDE_DIRS CACHE)
    if(NOT FIND_SILENT)
      request_include_search_dirs()
    endif()
  endif()

endif (NOT ${DEP}_INCLUDE_DIRS)

# search for libraries
if (NOT ${DEP}_LIBS_FOUND)
  # create list of libs from lib and optional arguments
  set(${DEP}_LIB_LIST ${DEP_LIB})
  list(APPEND ${DEP}_LIB_LIST ${_EXTRA_ARGS})
  # list(APPEND ${DEP}_LIB_LIST ${ARGN})
  # accumulate library list
  foreach(_SEARCH_LIB ${${DEP}_LIB_LIST})
    # search only for lib if it hasnt already been found
    list(FIND ${DEP}_FOUND_LIBS ${_SEARCH_LIB} _INDEX)
    if(${_INDEX} LESS 0)
      find_library(_CUR_SEARCH
        NAMES ${_SEARCH_LIB}
        HINTS ${${DEP}_LIBRARY_SEARCH_HINTS}
        PATHS ${${DEP}_LIBRARY_SEARCH_PATHS}
        PATH_SUFFIXES release debug
      )
      # react to search result
      if(_CUR_SEARCH)
        # add library and path to output lists 
        set(${DEP}_LIBRARIES ${${DEP}_LIBRARIES} ${_CUR_SEARCH} CACHE FILEPATH "The ${DEP} library filenames." FORCE)
        get_filename_component(_${DEP}_LIB_PATH ${_CUR_SEARCH} DIRECTORY)
        set(${DEP}_LIBRARY_DIRS ${${DEP}_LIBRARY_DIRS} ${_${DEP}_LIB_PATH} CACHE PATH "The ${DEP} library directories." FORCE)
        # mark library as found
        set(${DEP}_FOUND_LIBS ${${DEP}_FOUND_LIBS} ${_SEARCH_LIB} CACHE INTERNAL "internal use")
        # reset search status for next search
        unset(_CUR_SEARCH CACHE)
      else()
        # reset search status for next search
        unset(_CUR_SEARCH CACHE)
        if(NOT FIND_SILENT)
          # stop processing and exits loop
          request_lib_search_dirs()
        else()
          # remember that search failed
          set(_LIB_SEARCH_ABORTED TRUE)
          break()
        endif()
      endif()
    endif()
  endforeach()
  # end of loop, check if aborted or succeeded
  if(NOT _LIB_SEARCH_ABORTED)
    unset(${DEP}_LIBRARY_SEARCH_DIR CACHE)
    # if statement is reached, all libraries were found
    set(${DEP}_LIBS_FOUND TRUE CACHE INTERNAL "internal use")
  endif()
endif()

##############################################################################
# verify
##############################################################################
if(${DEP}_INCLUDE_DIRS AND ${DEP}_LIBS_FOUND)
  message(STATUS "Looking for ${DEP} - found")
  create_import_lib()
  # force value to prevent the finding not registering at successive cmake calls
  set(${DEP}_FOUND YES CACHE STRING "Whether ${DEP} was found" FORCE)
else() 
  set(${DEP}_FOUND ${DEP}-NOTFOUND  CACHE STRING "Whether ${DEP} was found" FORCE)
  message(STATUS "Looking for ${DEP} - not found")
endif()

endfunction(find_external)
