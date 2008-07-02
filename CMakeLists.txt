project(CGIScaler)
set(CGISCALER_VERSION "2.0.1" CACHE STRING "Version of this program")

add_subdirectory(src)
add_subdirectory(cgreen)
add_subdirectory(test)

INSTALL(FILES INSTALL.html PERFORMANCE.html RELEASE_NOTES COPYING README TODO DESTINATION share/doc/CGIScaler-${CGISCALER_VERSION})
