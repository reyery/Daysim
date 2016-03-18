#include_directories(${CMAKE_SOURCE_DIR}/src/common)

add_executable(gen_reindl gen_reindl.c file.c nrutil.c numerical.c skartveit.c sun.c)
#target_link_libraries(gen_reindl rtrad)

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)
