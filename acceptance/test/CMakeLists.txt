add_executable(test_closest_dis test_closest_dis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/PIAnaPointCloud.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/PIAnaHit.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/PIPointCloud.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/PIAnaPointCloud.hpp)

target_include_directories(test_closest_dis PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/include
)
target_link_libraries(test_closest_dis PUBLIC PiAnaAcc PiRootDict ROOT::Core)
