################################################
# generate version.h from template version.h.in
################################################

# Create .hpp version file with VERSION_INFO   
set(VERSION_HPP_IN "${CMAKE_SOURCE_DIR}/test/version.hpp.in")
set(VERSION_HPP_OUT "${CMAKE_SOURCE_DIR}/test/version.hpp")
configure_file(${VERSION_HPP_IN} ${VERSION_HPP_OUT})

####################
# Compile test cases
####################

if(BUILD_CODE)

 file(GLOB MARKDOWN *.md)
 file(GLOB SRC *.cpp *.hpp)
 find_package( Boost REQUIRED COMPONENTS unit_test_framework )
 include_directories( ${Boost_INCLUDE_DIRS} ../src )
 add_executable(unitTest ${SRC} ${MARKDOWN})
 target_link_libraries(unitTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
 add_test(testMain unitTest)

 # install #
 install(TARGETS unitTest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT "test")

endif(BUILD_CODE)

