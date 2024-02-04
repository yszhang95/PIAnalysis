add_executable(test_closest_dis test_closest_dis.cpp)

target_include_directories(test_closest_dis PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/include
)
target_link_libraries(test_closest_dis PUBLIC PiAnaAcc PiRootDict ROOT::Core)


add_executable(test_connected_components test_connected_components.cpp)

target_include_directories(test_connected_components PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../shared/include
)
target_link_libraries(test_connected_components PUBLIC PiAnaAcc PiRootDict ROOT::Core)

add_executable(test_tcluster test_tcluster.C)

target_include_directories(test_tcluster PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)

target_link_libraries(test_tcluster PUBLIC PiAnaAcc PiRootDict ROOT::Core)
