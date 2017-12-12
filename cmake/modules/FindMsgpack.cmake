################################################################################
#    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    #
#                                                                              #
#              This software is distributed under the terms of the             #
#              GNU Lesser General Public Licence (LGPL) version 3,             #
#                  copied verbatim in the file "LICENSE"                       #
################################################################################

if(DEFINED ENV{MSGPACK_ROOT})
    set(MSGPACK_ROOT $ENV{MSGPACK_ROOT})
endif()

MESSAGE(STATUS "Looking for MessagePack...")

find_path(MSGPACK_INCLUDE_DIR NAMES msgpack.hpp
  HINTS "${SIMPATH}/include"
        "${MSGPACK_ROOT}/include"
)

find_library(MSGPACK_LIBRARY_SHARED NAMES libmsgpackc.dylib libmsgpackc.so
  HINTS "${SIMPATH}/lib"
        "${MSGPACK_ROOT}/lib"
)

if(MSGPACK_INCLUDE_DIR AND MSGPACK_LIBRARY_SHARED)
  set(MSGPACK_FOUND true)
  add_library(Msgpack SHARED IMPORTED)
  set_target_properties(Msgpack PROPERTIES
    IMPORTED_LOCATION ${MSGPACK_LIBRARY_SHARED}
    INTERFACE_INCLUDE_DIRECTORIES ${MSGPACK_INCLUDE_DIR}
  )
  message(STATUS "Looking for MessagePack ... found at ${MSGPACK_LIBRARY_SHARED}")
else(MSGPACK_INCLUDE_DIR AND MSGPACK_LIBRARY_DIR)
  set(MSGPACK_FOUND false)
  message(STATUS "Looking for MessagePack ... not found.")
endif(MSGPACK_INCLUDE_DIR AND MSGPACK_LIBRARY_SHARED)
