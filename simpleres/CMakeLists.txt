project(simpleres)


add_library(${PROJECT_NAME}
	src/simpleres.c
	src/memory.c
	src/buffreader.c
)
add_library(smpr::smpr ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} 
	PUBLIC ${PROJECT_SOURCE_DIR}/include
	PRIVATE ${PROJECT_SOURCE_DIR}/src/include
)
