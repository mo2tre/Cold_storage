cmake_minimum_required(VERSION 3.20)
project(cold_storage_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(cold_storage_backend
    main.cpp
    controllers/HealthController.cpp
    controllers/FarmerController.cpp
    controllers/AuthController.cpp
    services/FarmerService.cpp
    services/AuthService.cpp
    repositories/FarmerRepository.cpp
    repositories/AuthRepository.cpp
    dto/FarmerDto.cpp
    dto/AuthDto.cpp
    utils/Crypto.cpp
    utils/PasswordPrompt.cpp
)

# Allow includes like "utils/ApiResponse.h" and "controllers/..."
target_include_directories(cold_storage_backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cold_storage_backend PRIVATE Drogon::Drogon OpenSSL::Crypto)

# Copy the example config next to the executable for convenience
add_custom_command(TARGET cold_storage_backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:cold_storage_backend>/config
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/config.example.json
        $<TARGET_FILE_DIR:cold_storage_backend>/config/config.example.json
)
