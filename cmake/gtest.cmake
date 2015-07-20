
include(ExternalProject)
include (CMakeParseArguments)

set(GTEST_FRAMEWORK gtest_framework)

ExternalProject_Add(
    ${GTEST_FRAMEWORK}
    URL https://googletest.googlecode.com/files/gtest-1.6.0.zip
    #SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/gtest-1.7.0
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    INSTALL_COMMAND "" # do not install this library
    CMAKE_ARGS -Dgtest_disable_pthreads=ON
)

ExternalProject_Get_Property(${GTEST_FRAMEWORK} source_dir binary_dir)

set(gtest_INCLUDE_DIR ${source_dir}/include)
set(gtest_LIBRARY_PATH
        ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(gtest_main_LIBRARY_PATH
        ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
set(gtest_LIBRARY gtest)
set(gtest_main_LIBRARY gtest_main)

add_library(${gtest_LIBRARY} STATIC IMPORTED GLOBAL)
set_target_properties(${gtest_LIBRARY} PROPERTIES
    IMPORTED_LOCATION ${gtest_LIBRARY_PATH}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
add_dependencies(${gtest_LIBRARY} ${GTEST_FRAMEWORK})

add_library(${gtest_main_LIBRARY} STATIC IMPORTED GLOBAL)
set_target_properties(${gtest_main_LIBRARY} PROPERTIES
    IMPORTED_LOCATION ${gtest_main_LIBRARY_PATH}
    IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
add_dependencies(${gtest_main_LIBRARY} ${gtest_LIBRARY})

include_directories(${gtest_INCLUDE_DIR}/include)
set(fl_TEST_LIBS ${gtest_LIBRARY} ${gtest_main_LIBRARY})

function(fl_add_test)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES LIBS)
    cmake_parse_arguments(
        fl "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TEST_NAME "${fl_NAME}_test")

    if(NOT catkin_FOUND)
        add_executable(${TEST_NAME} ${fl_SOURCES})
        target_link_libraries(${TEST_NAME} ${fl_TEST_LIBS} ${fl_LIBS})
        add_test(${TEST_NAME} ${TEST_NAME})
    else(NOT catkin_FOUND)
        catkin_add_test(${TEST_NAME} ${fl_SOURCES})
        target_link_libraries(${TEST_NAME} ${fl_TEST_LIBS} ${fl_LIBS})
    endif(NOT catkin_FOUND)
endfunction(fl_add_test)