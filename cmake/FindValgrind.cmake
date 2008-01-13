# - Find the Valgrind executables
#

FIND_PROGRAM(Valgrind_EXECUTABLE
    NAMES valgrind
    PATHS /usr/bin /usr/local/bin
)

IF (Valgrind_EXECUTABLE)
   SET(Valgrind_FOUND TRUE)
ENDIF (Valgrind_EXECUTABLE)

IF (Valgrind_FOUND)
   IF (NOT Valgrind_FIND_QUIETLY)
      MESSAGE(STATUS "Found Valgrind: ${Valgrind_EXECUTABLE}")
   ENDIF (NOT Valgrind_FIND_QUIETLY)
ELSE (Valgrind_FOUND)
   IF (Valgrind_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Valgrind")
   ENDIF (Valgrind_FIND_REQUIRED)
  MESSAGE(STATUS "Valgrind not found. Memory leak test will NOT run.")
ENDIF (Valgrind_FOUND)
