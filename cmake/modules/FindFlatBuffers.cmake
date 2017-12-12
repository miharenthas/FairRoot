################################################################################
#    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    #
#                                                                              #
#              This software is distributed under the terms of the             #
#              GNU Lesser General Public Licence (LGPL) version 3,             #
#                  copied verbatim in the file "LICENSE"                       #
################################################################################

MESSAGE(STATUS "Looking for FlatBuffers...")

if(DEFINED ENV{MSGPACK_ROOT})
    set(FLATBUFFERS_PATH $ENV{FLATBUFFERS_PATH})
endif()

find_path(FLATBUFFERS_INCLUDE_DIR NAMES flatbuffers/flatbuffers.h
   HINTS
  "${FLATBUFFERS_PATH}/include"
  "${SIMPATH}/include"
)

find_path(FLATBUFFERS_LIBRARY_DIR NAMES libflatbuffers.so
  HINTS "${FLATBUFFERS_PATH}/"
        "${SIMPATH}/lib"
        "/usr/lib64"
)

find_path(FLATBUFFERS_BINARY_DIR NAMES flatc
  HINTS "${FLATBUFFERS_PATH}/"
        "${SIMPATH}/bin"
)

if(FLATBUFFERS_INCLUDE_DIR AND FLATBUFFERS_LIBRARY_DIR AND FLATBUFFERS_BINARY_DIR)
  set(FLATBUFFERS_FOUND true)
  message(STATUS "Looking for FlatBuffers ... found at ${FLATBUFFERS_INCLUDE_DIR}")
else(FLATBUFFERS_INCLUDE_DIR AND FLATBUFFERS_LIBRARY_DIR AND FLATBUFFERS_BINARY_DIR)
  set(FLATBUFFERS_FOUND false)
  message(STATUS "Looking for FlatBuffers ... not found.")
endif(FLATBUFFERS_INCLUDE_DIR AND FLATBUFFERS_LIBRARY_DIR AND FLATBUFFERS_BINARY_DIR)

