project (simple-weston-signal)

find_package(PkgConfig)
pkg_check_modules(WAYLAND_CLIENT wayland-client REQUIRED)
pkg_check_modules(LIBWESTON_PROTOCOLS libweston-11-protocols QUIET)

if(${LIBWESTON_PROTOCOLS_FOUND})
    #import the pkgdatadir from libweston-protocols pkgconfig file
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=pkgdatadir libweston-11-protocols
                    OUTPUT_VARIABLE WestonProtocols_PKGDATADIR)
    string(REGEX REPLACE "[\r\n]" "" WestonProtocols_PKGDATADIR "${WestonProtocols_PKGDATADIR}")
    SET(LIBWESTON_PROTOCOLS_PKGDATADIR ${WestonProtocols_PKGDATADIR})
endif(${LIBWESTON_PROTOCOLS_FOUND})

find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

add_custom_command(
    OUTPUT  weston-simple-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
            < ${CMAKE_SOURCE_DIR}/protocol/weston-simple.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/weston-simple-client-protocol.h
    DEPENDS ${CMAKE_SOURCE_DIR}/protocol/weston-simple.xml
)

add_custom_command(
    OUTPUT  weston-simple-protocol.c
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code
            < ${CMAKE_SOURCE_DIR}/protocol/weston-simple.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/weston-simple-protocol.c
    DEPENDS ${CMAKE_SOURCE_DIR}/protocol/weston-simple.xml
)

include_directories(
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(
    ${WAYLAND_CLIENT_LIBRARY_DIRS}
)

SET(LIBS
    ${WAYLAND_CLIENT_LIBRARIES}
)

SET(SRC_FILES
    src/simple-weston-signal.c
    weston-simple-protocol.c
    weston-simple-client-protocol.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

add_dependencies(${PROJECT_NAME} ${LIBS})

target_link_libraries(${PROJECT_NAME} ${LIBS})

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
