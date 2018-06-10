# Source files
file(GLOB SOURCES_NETWORKLADDERBOT "*.cpp" "*.h")

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
add_executable(NetworkLadderBot ${SOURCES_NETWORKLADDERBOT})

target_link_libraries(NetworkLadderBot
    sc2api sc2lib sc2utils sc2protocol civetweb libprotobuf
)

# Set working directory as the project root
set_target_properties(NetworkLadderBot PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")