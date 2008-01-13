# - Find the ImageMagick MagickWand libs
#

FIND_PATH(Wand_INCLUDE_DIR MagickWand.h /usr/include/wand)

FIND_LIBRARY(Wand_LIBRARY
    NAMES Wand
    PATHS /usr/lib /usr/local/lib
)

IF (Wand_INCLUDE_DIR AND Wand_LIBRARY)
   SET(Wand_FOUND TRUE)
ENDIF (Wand_INCLUDE_DIR AND Wand_LIBRARY)

IF (Wand_FOUND)
   IF (NOT Wand_FIND_QUIETLY)
      MESSAGE(STATUS "Found Wand: ${Wand_LIBRARY}")
   ENDIF (NOT Wand_FIND_QUIETLY)
ELSE (Wand_FOUND)
   IF (Wand_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Wand")
   ENDIF (Wand_FIND_REQUIRED)
ENDIF (Wand_FOUND)
