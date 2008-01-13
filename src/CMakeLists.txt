set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Magick REQUIRED)
find_package(Wand REQUIRED)

add_executable(cgiscaler cache.c  commandline.c  debug.c  file_utils.c
  geometry_math.c  main.c  query_string.c  runtime_config.c  scaler.c
  serve.c)
target_link_libraries(cgiscaler ${Magick_LIBRARY})
target_link_libraries(cgiscaler ${Wand_LIBRARY})

  
find_package(Doxygen)
if(Doxygen_FOUND)
  option(MAKE_DOXYGEN_DOCUMENTATION "If set doxygen documentation will be generated to doc directory" ON)
  if(MAKE_DOXYGEN_DOCUMENTATION)
    add_custom_command(TARGET cgiscaler POST_BUILD COMMAND doxygen ${PROJECT_SOURCE_DIR}/doxygen.conf WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  endif(MAKE_DOXYGEN_DOCUMENTATION)
endif(Doxygen_FOUND)
