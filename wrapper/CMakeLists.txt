
if(BUILD_C_WRAPPER )

	add_library( avproto SHARED cstub.cpp)
	target_link_libraries(avproto avproto++)

	if(MSVC)
	if(CMAKE_CL_64)
	set_target_properties(avproto
			PROPERTIES
			OUTPUT_NAME avproto64
			PREFIX ""
			CLEAN_DIRECT_OUTPUT 1)
	else()
	set_target_properties(avproto
			PROPERTIES
			OUTPUT_NAME avproto
			PREFIX ""
			CLEAN_DIRECT_OUTPUT 1)
	endif()
	endif()

endif()
