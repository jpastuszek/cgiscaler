project(CGIScaler)
cmake_minimum_required(VERSION 2.6)
set(CGISCALER_VERSION "2.1" CACHE STRING "Version of this program")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(cgreen)
add_subdirectory(test)

find_package(AsciiDoc)
if(AsciiDoc_FOUND)
    add_custom_command(OUTPUT INSTALL.html 
        COMMAND asciidoc INSTALL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_custom_command(OUTPUT PERFORMANCE.html
        COMMAND asciidoc PERFORMANCE
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    add_custom_command(OUTPUT README.html
        COMMAND asciidoc README
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    add_custom_target(doc ALL 
        DEPENDS INSTALL.html PERFORMANCE.html README.html)

    install(FILES INSTALL.html PERFORMANCE.html README.html DESTINATION share/doc/CGIScaler-${CGISCALER_VERSION})
    install(FILES doc/strict.jpg doc/fit.jpg DESTINATION share/doc/CGIScaler-${CGISCALER_VERSION}/doc)
endif(AsciiDoc_FOUND)

install(FILES RELEASE_NOTES LICENSE README TODO DESTINATION share/doc/CGIScaler-${CGISCALER_VERSION})
