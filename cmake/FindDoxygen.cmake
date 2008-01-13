# - Find the Doxygen executables
#

FIND_PROGRAM(Doxygen_EXECUTABLE
    NAMES doxygen
    PATHS /usr/bin /usr/local/bin
)

IF (Doxygen_EXECUTABLE)
   SET(Doxygen_FOUND TRUE)
ENDIF (Doxygen_EXECUTABLE)

IF (Doxygen_FOUND)
   IF (NOT Doxygen_FIND_QUIETLY)
      MESSAGE(STATUS "Found Doxygen: ${Doxygen_EXECUTABLE}")
   ENDIF (NOT Doxygen_FIND_QUIETLY)
ELSE (Doxygen_FOUND)
   IF (Doxygen_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Doxygen")
   ENDIF (Doxygen_FIND_REQUIRED)
  MESSAGE(STATUS "Doxygen not found. Source documentation will NOT be generated.")
ENDIF (Doxygen_FOUND)
