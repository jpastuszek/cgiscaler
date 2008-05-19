set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Magick REQUIRED)
find_package(Wand REQUIRED)

include_directories (${PROJECT_SOURCE_DIR}/cgreen)
link_directories (${PROJECT_SOURCE_DIR}/cgreen)

add_executable(cache
  test_cache.c
  asserts.c
  stdio_capture.c
  runtime_config.c
  cache.c
  debug.c
  file_utils.c
  serve.c
  format_info.c
  )
target_link_libraries(cache ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(commandline
  test_commandline.c
  asserts.c
  runtime_config.c
  commandline.c
  debug.c
  file_utils.c
  format_info.c
  scaler.c
  geometry_math.c
  )
target_link_libraries(commandline ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(file_utils
  test_file_utils.c
  asserts.c
  runtime_config.c
  debug.c
  file_utils.c
  format_info.c
  )
target_link_libraries(file_utils ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(format_info
  test_format_info.c
  asserts.c
  runtime_config.c
  debug.c
  format_info.c
  )
target_link_libraries(format_info ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(geometry_math
  test_geometry_math.c
  asserts.c
  runtime_config.c
  debug.c
  geometry_math.c
  format_info.c
  )
target_link_libraries(geometry_math ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(main
  test_main.c
  asserts.c
  stdio_capture.c
  cache.c
  commandline.c
  debug.c
  file_utils.c
  geometry_math.c
  cgiscaler.c
  query_string.c
  runtime_config.c
  scaler.c
  serve.c
  format_info.c
  )
target_link_libraries(main ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(query_string
  test_query_string.c
  asserts.c
  runtime_config.c
  debug.c
  file_utils.c
  query_string.c
  format_info.c
  )
target_link_libraries(query_string ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(runtime_config
  test_runtime_config.c
  debug.c
  runtime_config.c
  format_info.c
  )
target_link_libraries(runtime_config ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(scaler
  test_scaler.c
  asserts.c
  runtime_config.c
  debug.c
  file_utils.c
  geometry_math.c
  scaler.c
  format_info.c
  )
target_link_libraries(scaler ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(serve
  test_serve.c
  asserts.c
  debug.c
  runtime_config.c
  stdio_capture.c
  cache.c
  file_utils.c
  geometry_math.c
  scaler.c
  serve.c
  format_info.c
  )
target_link_libraries(serve ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

option(RUN_UNIT_TESTS "If set unit test will be performed" ON)
if(RUN_UNIT_TESTS)
  add_custom_command(TARGET cache commandline file_utils geometry_math main query_string runtime_config scaler serve 
    POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/test/unit_test)
endif(RUN_UNIT_TESTS)

find_package(Valgrind)
if(Valgrind_FOUND)
  option(RUN_MEMORY_TESTS "If set valgrind memory tests will be performed for each test suite - slow." OFF)
  if(RUN_MEMORY_TESTS)
    add_custom_command(TARGET cache commandline file_utils geometry_math main query_string runtime_config scaler serve 
      POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/test/memory_test)
  endif(RUN_MEMORY_TESTS)
endif(Valgrind_FOUND)


