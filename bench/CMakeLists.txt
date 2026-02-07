# Benchmarks
############

# dependencies
find_package(benchmark REQUIRED)

# target "bench"
add_custom_target(bench
	COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_PROPERTY:bench,MANUALLY_ADDED_DEPENDENCIES>"
	COMMAND ${CMAKE_COMMAND} -E echo "(can be found in ${CMAKE_CURRENT_BINARY_DIR})"
	COMMENT "Available benchmarks:" VERBATIM)

# executables
file(GLOB BENCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(i ${BENCH_FILES})
	get_filename_component(name "${i}" NAME_WE)

	add_executable(bench_${name} EXCLUDE_FROM_ALL ${i})
	set_target_properties(bench_${name} PROPERTIES WIN32_EXECUTABLE FALSE)
	target_link_libraries(bench_${name} ${PROJECT_NAME} benchmark)
	add_dependencies(bench bench_${name})
endforeach()
