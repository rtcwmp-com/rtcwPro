#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CURL::libcurl" for configuration "RelWithDebInfo"
set_property(TARGET CURL::libcurl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(CURL::libcurl PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libcurl_imp.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/libcurl.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS CURL::libcurl )
list(APPEND _IMPORT_CHECK_FILES_FOR_CURL::libcurl "${_IMPORT_PREFIX}/lib/libcurl_imp.lib" "${_IMPORT_PREFIX}/bin/libcurl.dll" )

# Import target "CURL::curl" for configuration "RelWithDebInfo"
set_property(TARGET CURL::curl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(CURL::curl PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/curl.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS CURL::curl )
list(APPEND _IMPORT_CHECK_FILES_FOR_CURL::curl "${_IMPORT_PREFIX}/bin/curl.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
