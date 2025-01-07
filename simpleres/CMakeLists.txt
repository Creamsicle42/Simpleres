project(simpleres)


add_library(&{PROJECT_NAME})
add_library(smpr::smpr ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
