cmake_minimum_required ( VERSION 2.8 )

project ( badgerd-muxer CXX )

find_package ( LibFTDI1 NO_MODULE REQUIRED )
FIND_PACKAGE(PkgConfig)

include ( ${LIBFTDI_USE_FILE} )

SET(BIN_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/bin"
    CACHE PATH
    "Binary installation directory")

PKG_CHECK_MODULES(MUXER_DEP
  REQUIRED
  popt
)

add_executable ( badgerd-muxer main.cpp )
target_link_libraries( badgerd-muxer ${LIBFTDI_LIBRARIES} ${MUXER_DEP_LIBRARIES})

install ( TARGETS badgerd-muxer
          DESTINATION ${BIN_INSTALL_DIR} )
