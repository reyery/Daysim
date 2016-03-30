set(VERSION_FILE "${daysim_BINARY_DIR}/src/daysim/Version.c")
create_version_file("${VERSION_FILE}")

include_directories(${CMAKE_SOURCE_DIR}/common)

add_library(daysim_common daylightfactor.c diverse_functions.c eputs.c fropen.c gethomedir.c getpath.c parse.c read_in_header.c words.c sun.c "${VERSION_FILE}")

add_executable(gen_reindl gen_reindl.c nrutil.c numerical.c skartveit.c)
target_link_libraries(gen_reindl rtrad daysim_common)

add_executable(ds_el_lighting ds_el_lighting.c allocate_memory.c analysis_data.c get_illuminances.c lightswitch.c simulation_assumptions.c BlindModel.c occ_func.c)
target_link_libraries(ds_el_lighting rtrad daysim_common)

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)

#BlindModel.c and occ_func.c shifted to executable line

#install(TARGETS gen_reindl RUNTIME DESTINATION bin)
