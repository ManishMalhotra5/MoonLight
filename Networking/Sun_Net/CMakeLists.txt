cmake_minimum_required(VERSION 3.6.0)
project(Sun_Net)

set (CMAKE_CXX_FLAGS "-std=c++2a ${CMAKE_CXX_FLAGS}")

add_library(Sun_Net INTERFACE)
if(WIN32)
find_library(WSOCK32_LIBRARY wsock32)
find_library(WS2_32_LIBRARY ws2_32)

endif()

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(Sun_Net_test ${SRCS})
if(WIN32)
target_link_libraries(Sun_Net_test PUBLIC wsock32 ws2_32)
endif()
target_include_directories(Sun_Net INTERFACE
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/vendor/asio/include
)

target_sources(Sun_Net_test PRIVATE ${SRCS})
target_link_libraries(Sun_Net_test PUBLIC Sun_Net)

