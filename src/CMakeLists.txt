set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Magick REQUIRED)
find_package(Wand REQUIRED)

option(DEBUG_SYNC "If set every log line will be flushed to disk (very slow)" FALSE)
if(DEBUG_SYNC STREQUAL "ON")
  set(DEBUG_SYNC 1)
else(DEBUG_SYNC STREQUAL "ON")
   set(DEBUG_SYNC 0)
endif(DEBUG_SYNC STREQUAL "ON")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_config.h.tpl ${CMAKE_CURRENT_SOURCE_DIR}/build_config.h)

add_executable(cgiscaler cache.c  commandline.c  debug.c  file_utils.c
  geometry_math.c  main.c cgiscaler.c format_info.c  query_string.c  runtime_config.c  scaler.c
  serve.c)
target_link_libraries(cgiscaler ${Magick_LIBRARY})
target_link_libraries(cgiscaler ${Wand_LIBRARY})

  
find_package(Doxygen)
if(Doxygen_FOUND)
  option(MAKE_DOXYGEN_DOCUMENTATION "If set doxygen documentation will be generated to doc directory" OFF)
  if(MAKE_DOXYGEN_DOCUMENTATION)
    add_custom_command(TARGET cgiscaler POST_BUILD COMMAND doxygen ${PROJECT_SOURCE_DIR}/doxygen.conf WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  endif(MAKE_DOXYGEN_DOCUMENTATION)
endif(Doxygen_FOUND)


install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cgiscaler DESTINATION libexec)
