set(VERSION_FILE "${radiance_BINARY_DIR}/src/daysim/Version.c")
create_version_file("${VERSION_FILE}")

#include_directories(${CMAKE_SOURCE_DIR}/src/common)

add_executable(gen_reindl gen_reindl.c file.c nrutil.c numerical.c skartveit.c sun.c "${VERSION_FILE}")
target_link_libraries(gen_reindl rtrad)

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)
