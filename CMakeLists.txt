cmake_minimum_required ( VERSION 2.8 )

project ( muxer CXX )

find_package ( LibFTDI1 NO_MODULE REQUIRED )
FIND_PACKAGE(PkgConfig)

include ( ${LIBFTDI_USE_FILE} )

PKG_CHECK_MODULES(MUXER_DEP
  REQUIRED
  popt
)

add_executable ( muxer main.cpp )
target_link_libraries( muxer ${LIBFTDI_LIBRARIES} ${MUXER_DEP_LIBRARIES})

install ( TARGETS muxer
          DESTINATION bin )
