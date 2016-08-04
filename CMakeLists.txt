cmake_minimum_required (VERSION 3.5)

# if we want just generate documentation (doxygen & latex required) 
# then there is not need to have a proper C++ compiler & related utilities (boost, ...)
if(JUST_DOCUMENTATION)
  project (SuperSimpleStocks NONE)
else()
  project (SuperSimpleStocks CXX)
endif()

# Just to check it out compilers/libraries
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/" ${CMAKE_MODULE_PATH})
find_package(Rules) 

# Get ready for unit tests 
enable_testing()

# recursive call to code folders
add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(doc)

# from "make install" task generate basic package
set(CPACK_GENERATOR "TGZ")
set(CPACK_SET_DESTDIR ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
include(CPack)
