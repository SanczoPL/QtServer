cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(QtServer VERSION 0.0.1 DESCRIPTION "Simple QtServer" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON}")

set(TARGET_NAME "QtServer")
set(DEFAULT_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_AUTOMOC ON)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

macro(copy_json jsonname)
	configure_file(${jsonname} ${CMAKE_BINARY_DIR}/${jsonname})
	configure_file(${jsonname} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin/${jsonname})
	configure_file(${jsonname} ${CMAKE_BINARY_DIR}/bin/${jsonname})
	configure_file(${jsonname} ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${jsonname})
endmacro()
copy_json("server.json")


if (WIN32)
	set(QT5_PATH  "E:\\Qt\\5.15.2\\msvc2019_64")
	list(APPEND CMAKE_PREFIX_PATH "E:\\repo\\opencvBuild")
	list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
	list(APPEND CMAKE_PREFIX_PATH "E:\\repo\\spdlog\\build\\install")
endif (WIN32)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Network REQUIRED)


enable_testing()

macro(clone_build libname)
	configure_file(clone_build/${libname}.in ${libname}-download/CMakeLists.txt)
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${libname}-download")
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${libname}-download")
	add_subdirectory("${CMAKE_BINARY_DIR}/${libname}-src"
                 "${CMAKE_BINARY_DIR}/${libname}-build")
endmacro()

clone_build(googletest)
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#add_subdirectory(Utils)
find_package(Utils)

set(SRC_MAIN src/main.cc)
set(SRC
  src/sockio.cc
  src/connection.cc
  src/message.cc
  src/ioserver.cc)

set(HEADERS
  include/sockio.h
  include/connection.h
  include/message.h
  include/ioserver.h)

add_library(${PROJECT_NAME} STATIC
server.json ${SRC} ${HEADERS} )

add_executable(g_test 
server.json 
gtest/gtest.cc 
gtest/gtest_ioserver.cc 
gtest/gtest_ioserver.h)

add_executable(q_test server.json qtest/qtest.cc qtest/qtest.h)

add_executable( 
 ${PROJECT_NAME}_app server.json ${SRC} ${SRC_MAIN} ${HEADERS} )

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

target_link_libraries(${PROJECT_NAME} Utils )
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network Threads::Threads)

target_link_libraries(${PROJECT_NAME}_app ${PROJECT_NAME} Utils)
target_link_libraries(${PROJECT_NAME}_app Qt5::Core Qt5::Network Threads::Threads)

target_link_libraries(g_test ${PROJECT_NAME} Utils gtest gmock)
target_link_libraries(g_test Qt5::Core Qt5::Network Threads::Threads)

target_link_libraries(q_test ${PROJECT_NAME} Utils)
target_link_libraries(q_test Qt5::Test Qt5::Core Qt5::Network Threads::Threads)

  
if (WIN32)
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    add_custom_command(TARGET ${PROJECT_NAME}_app POST_BUILD
       COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --qmldir ${CMAKE_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}_app>)
endif(WIN32)

if (WIN32)
	macro(qt5_copy_dll APP DLL)
		# find the release *.dll file
		get_target_property(Qt5_${DLL}Location Qt5::${DLL} LOCATION)
		# find the debug *d.dll file
		get_target_property(Qt5_${DLL}LocationDebug Qt5::${DLL} IMPORTED_LOCATION_DEBUG)
		add_custom_command(TARGET ${APP} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${Qt5_${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Qt5_${DLL}Location}> $<TARGET_FILE_DIR:${APP}>)
	endmacro()
	qt5_copy_dll( ${PROJECT_NAME}_app Core)

endif(WIN32)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "default install path" FORCE )
endif()

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)
