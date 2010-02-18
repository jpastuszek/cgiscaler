# - Find the AsciiDoc executables
#

FIND_PROGRAM(AsciiDoc_EXECUTABLE
    NAMES asciidoc
    PATHS /usr/bin /usr/local/bin
)

IF (AsciiDoc_EXECUTABLE)
   SET(AsciiDoc_FOUND TRUE)
ENDIF (AsciiDoc_EXECUTABLE)

IF (AsciiDoc_FOUND)
   IF (NOT AsciiDoc_FIND_QUIETLY)
      MESSAGE(STATUS "Found AsciiDoc: ${AsciiDoc_EXECUTABLE}")
   ENDIF (NOT AsciiDoc_FIND_QUIETLY)
ELSE (AsciiDoc_FOUND)
   IF (AsciiDoc_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find AsciiDoc")
   ENDIF (AsciiDoc_FIND_REQUIRED)
  MESSAGE(STATUS "AsciiDoc not found. Source documentation will NOT be generated.")
ENDIF (AsciiDoc_FOUND)
