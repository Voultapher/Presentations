add_executable (${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  uWS
)

target_compile_features(${PROJECT_NAME} PUBLIC
  cxx_std_17
)
