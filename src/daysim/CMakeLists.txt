set(VERSION_FILE "${daysim_BINARY_DIR}/src/daysim/Version.c")
create_version_file("${VERSION_FILE}")

include_directories(${CMAKE_SOURCE_DIR}/common)

add_library(daysim_common fropen.c parse.c read_in_header.c nrutil.c numerical.c sun.c spa.c "${VERSION_FILE}")

add_executable(gen_reindl gen_reindl.c)
target_link_libraries(gen_reindl rtrad daysim_common)

add_executable(gencumulativesky gendiscretesky.cpp climateFile.cpp cPerezSkyModel.cpp cSkyVault.cpp cSun.cpp)
target_link_libraries(gencumulativesky rtrad daysim_common)

add_executable(ds_el_lighting ds_el_lighting.c allocate_memory.c analysis_data.c daylightfactor.c get_illuminances.c lightswitch.c simulation_assumptions.c BlindModel.c occ_func.c)
target_link_libraries(ds_el_lighting rtrad daysim_common)

add_executable(ds_illum ds_illum.c calculate_Perez.c shadow_testing.c calculate_sky_patches.c check_direct_sunlight.c)
target_link_libraries(ds_illum rtrad daysim_common)

add_executable(gen_dc gen_dc.c write_dds_files.c calculate_sky_patches_gen_dc.c)
target_link_libraries(gen_dc rtrad daysim_common)

add_executable(ds_shortterm ds_shortterm.c clearsky_models.c 60min_file.c read_in.c skartveit.c)
target_link_libraries(ds_shortterm rtrad daysim_common)

add_executable(gen_dgp_profile gen_dgp_profile.c)
target_link_libraries(gen_dgp_profile rtrad daysim_common)

add_executable(gen_directsunlight gen_directsunlight.c)
target_link_libraries(gen_directsunlight rtrad daysim_common)

add_executable(gen_single_office gen_single_office.c)
target_link_libraries(gen_single_office rtrad daysim_common)

add_executable(radfiles2daysim radfiles2daysim.c)
target_link_libraries(radfiles2daysim rtrad daysim_common)

add_executable(rotate_scene rotate_scene.c)
target_link_libraries(rotate_scene rtrad daysim_common)

add_executable(scale_dc scale_dc.c)
target_link_libraries(scale_dc rtrad daysim_common)

add_executable(spa_tester spa_tester.c spa.c)

#install(TARGETS gen_reindl gencumulativesky ds_el_lighting ds_illum ds_shortterm gen_dc gen_dgp_profile gen_directsunlight gen_single_office radfiles2daysim rotate_scene scale_dc RUNTIME DESTINATION bin)

