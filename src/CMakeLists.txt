# Source files
file(GLOB SOURCES_NETWORKBOT "*.cpp" "*.h")

# Include directories
include_directories(SYSTEM
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/s2client-api/include
    ${PROJECT_SOURCE_DIR}/s2client-api/contrib/protobuf/src
    ${PROJECT_BINARY_DIR}/s2client-api/generated
)

# Link directories
link_directories(${PROJECT_BINARY_DIR}/s2client-api/bin)

# Create the executable.
add_executable(NetworkBot ${SOURCES_NETWORKBOT})

target_link_libraries(NetworkBot
    sc2api sc2lib sc2utils sc2protocol civetweb libprotobuf
)

# Set working directory as the project root
set_target_properties(NetworkBot PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")