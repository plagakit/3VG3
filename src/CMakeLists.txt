# TODO: change this!

file(GLOB_RECURSE CPP_SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

target_sources(${PROJECT_NAME} PUBLIC ${CPP_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC .)