project(userver-sqlite CXX)

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

file(GLOB_RECURSE SQLITE_FUNCTIONAL_TEST_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/functional_tests/*
)
list(REMOVE_ITEM SOURCES ${SQLITE_FUNCTIONAL_TEST_SOURCES})


add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC userver-core)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

_userver_directory_install(COMPONENT sqlite
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/..
)
_userver_install_targets(COMPONENT sqlite TARGETS ${PROJECT_NAME})

# _userver_directory_install(COMPONENT sqlite FILES
#     "${USERVER_ROOT_DIR}/cmake/install/userver-sqlite-config.cmake"
#     "${USERVER_ROOT_DIR}/cmake/modules/Findlibsqlite.cmake"
#     DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/userver"
# )
