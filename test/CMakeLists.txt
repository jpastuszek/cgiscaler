set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Magick REQUIRED)
find_package(Wand REQUIRED)

include_directories (${PROJECT_SOURCE_DIR}/cgreen)
link_directories (${PROJECT_SOURCE_DIR}/cgreen)

add_executable(test_cache
  test_cache.c
  asserts.c
  stdio_capture.c
  cache.c
  debug.c
  file_utils.c
  serve.c
  )
target_link_libraries(test_cache ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(test_commandline
  test_commandline.c
  asserts.c
  commandline.c
  debug.c
  file_utils.c
  runtime_config.c
  )
target_link_libraries(test_commandline ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(test_file_utils
  test_file_utils.c
  asserts.c
  debug.c
  file_utils.c
  )
target_link_libraries(test_file_utils ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(test_geometry_math
  test_geometry_math.c
  asserts.c
  debug.c
  geometry_math.c
  )
target_link_libraries(test_geometry_math ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(test_main
  test_main.c
  asserts.c
  stdio_capture.c
  cache.c
  commandline.c
  debug.c
  file_utils.c
  geometry_math.c
  main.c
  query_string.c
  runtime_config.c
  scaler.c
  serve.c
  )
target_link_libraries(test_main ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(test_query_string
  test_query_string.c
  asserts.c
  debug.c
  file_utils.c
  query_string.c
  runtime_config.c
  )
target_link_libraries(test_query_string ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(test_runtime_config
  test_runtime_config.c
  debug.c
  runtime_config.c
  )
target_link_libraries(test_runtime_config ${Magick_LIBRARY} ${Wand_LIBRARY}
  cgreen)

add_executable(test_scaler
  test_scaler.c
  asserts.c
  debug.c
  file_utils.c
  geometry_math.c
  scaler.c
  )
target_link_libraries(test_scaler ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

add_executable(test_serve
  test_serve.c
  asserts.c
  debug.c
  stdio_capture.c
  cache.c
  file_utils.c
  geometry_math.c
  scaler.c
  serve.c
  )
target_link_libraries(test_serve ${Magick_LIBRARY} ${Wand_LIBRARY} cgreen)

option(RUN_UNIT_TESTS "If set unit test will be performed" ON)
if(RUN_UNIT_TESTS)
  add_custom_command(TARGET test_cache test_commandline test_file_utils test_geometry_math test_main test_query_string test_runtime_config test_scaler test_serve 
    POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/test/unit_test)
endif(RUN_UNIT_TESTS)

find_package(Valgrind)
if(Valgrind_FOUND)
  option(RUN_MEMORY_TESTS "If set valgrind memory tests will be performed for each test suite - slow." OFF)
  if(RUN_MEMORY_TESTS)
    add_custom_command(TARGET test_cache test_commandline test_file_utils test_geometry_math test_main test_query_string test_runtime_config test_scaler test_serve 
      POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/test/memory_test)
  endif(RUN_MEMORY_TESTS)
endif(Valgrind_FOUND)


