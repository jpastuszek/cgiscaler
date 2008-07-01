# - Find the ImageMagick Core libs
#

FIND_PATH(Magick_INCLUDE_DIR ImageMagick.h /usr/include /usr/include/ImageMagick /usr/include /usr/local/include)

IF (NOT Magick_INCLUDE_DIR)
  FIND_PATH(Magick_INCLUDE_DIR magick/ImageMagick.h /usr/include /usr/include/ImageMagick /usr/include /usr/local/include)
ENDIF (NOT Magick_INCLUDE_DIR)

FIND_LIBRARY(Magick_LIBRARY
    NAMES Magick
    PATHS /usr/lib /usr/local/lib
)

IF (NOT Magick_FOUND)
  FIND_LIBRARY(Magick_LIBRARY
        NAMES MagickCore
        PATHS /usr/lib /usr/local/lib
  )
ENDIF (NOT Magick_FOUND)

IF (Magick_INCLUDE_DIR AND Magick_LIBRARY)
   SET(Magick_FOUND TRUE)
ENDIF (Magick_INCLUDE_DIR AND Magick_LIBRARY)

IF (Magick_FOUND)
   INCLUDE_DIRECTORIES(BEFORE ${Magick_INCLUDE_DIR})
   IF (NOT Magick_FIND_QUIETLY)
      MESSAGE(STATUS "Found Magick: ${Magick_LIBRARY}")
   ENDIF (NOT Magick_FIND_QUIETLY)
ELSE (Magick_FOUND)
   IF (Magick_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Magick")
   ENDIF (Magick_FIND_REQUIRED)
ENDIF (Magick_FOUND)
