set(VERSION_FILE "${radiance_BINARY_DIR}/src/daysim/Version.c")
create_version_file("${VERSION_FILE}")

#include_directories(${CMAKE_SOURCE_DIR}/src/common)

add_executable(gen_reindl gen_reindl.c file.c nrutil.c numerical.c skartveit.c sun.c "${VERSION_FILE}")
target_link_libraries(gen_reindl rtrad)

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)

include_directories(${CMAKE_SOURCE_DIR}/common)

add_library(daysim_common daylightfactor.c diverse_functions.c eputs.c fropen.c gethomedir.c getpath.c parse.c read_in_header.c words.c file.c)

add_executable(gen_reindl gen_reindl.c sun.c nrutil.c numerical.c skartveit.c)

add_executable(ds_el_lighting ds_el_lighting.c allocate_memory.c analysis_data.c get_illuminances.c lightswitch.c simulation_assumptions.c BlindModel.c occ_func.c)
target_link_libraries(ds_el_lighting rtrad daysim_common)

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)

#gendaylit.c included in rtrad now!
#BlindModel.c and occ_func.c shifted to executable line

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)
