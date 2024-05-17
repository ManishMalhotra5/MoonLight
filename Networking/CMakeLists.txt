cmake_minimum_required(VERSION 3.6.0)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Sun_Net" Sun_Net)

project(Sun_Client)
project(Sun_Server)

set (CMAKE_CXX_FLAGS "-std=c++2a ${CMAKE_CXX_FLAGS}")

add_library(Sun_Client INTERFACE)
add_library(Sun_Server INTERFACE)
if(WIN32)
find_library(WSOCK32_LIBRARY wsock32)
find_library(WS2_32_LIBRARY ws2_32 )
endif()

file(GLOB_RECURSE CLIENT_SRCS CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/Sun_Client/src/*.cpp"
)

file(GLOB_RECURSE SERVER_SRCS CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/Sun_Server/src/*.cpp"
)

add_executable(client ${CLIENT_SRCS})
if(WIN32)
target_link_libraries(client PUBLIC ws2_32 wsock32)
endif()
target_include_directories(Sun_Client INTERFACE
	${PROJECT_SOURCE_DIR}/Sun_Client/include
	${PROJECT_SOURCE_DIR}/Sun_Client/src
)
target_sources(client PRIVATE ${CLIENT_SRCS})
target_link_libraries(client PUBLIC Sun_Net Sun_Client)



add_executable(server ${SERVER_SRCS})
if(WIN32)
target_link_libraries(server PUBLIC wsock32 ws2_32)
endif()
target_include_directories(Sun_Server INTERFACE
	${PROJECT_SOURCE_DIR}/Sun_Server/include
	${PROJECT_SOURCE_DIR}/Sun_Server/src
)
target_sources(server PRIVATE ${SERVER_SRCS})
target_link_libraries(server PUBLIC Sun_Net Sun_Server)
