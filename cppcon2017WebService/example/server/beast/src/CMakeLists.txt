add_executable (${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/lib/beast/include
  "${CMAKE_SOURCE_DIR}/../strawpoll"
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  ${CMAKE_THREAD_LIBS_INIT}
  Boost::boost Boost::system
)

target_compile_features(${PROJECT_NAME} PUBLIC
  cxx_std_17
)
