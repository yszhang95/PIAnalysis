cmake_minimum_required(VERSION 3.1...3.27)

project(
  PIAnaAcceptance
  VERSION 0.1
  LANGUAGES CXX)

# required for installation
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

# include("${ROOT_DIR}/RootMacros.cmake")

find_package(ROOT 6.20 CONFIG REQUIRED)

# file(GLOB sources ${PIAnaAcceptance_SOURCE_DIR}/src/*.cpp)
# file(GLOB headers ${PIAnaAcceptance_SOURCE_DIR}/include/*.hpp)

add_library(PiAnaAcc SHARED
                     ${PIAnaAcceptance_SOURCE_DIR}/src/PIAnalyzer.cpp
                     ${PIAnaAcceptance_SOURCE_DIR}/src/PIAnaHit.cpp
                     ${PIAnaAcceptance_SOURCE_DIR}/src/PIAnaPat.cpp
                     ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnalyzer.hpp
                     ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnaHit.hpp
                     ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnaPat.hpp
                     ${PIONEER_LIB_SOURCE_DIR}/include/PIMCAtar.hh)

target_include_directories(PiAnaAcc PUBLIC include
                           ${PIONEER_LIB_SOURCE_DIR}/include)

target_link_libraries(PiAnaAcc PUBLIC PiRootDict ROOT::Core)

root_generate_dictionary(G__PiAnaAcc  ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnalyzer.hpp ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnaHit.hpp ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnaPat.hpp
                         MODULE PiAnaAcc LINKDEF ${PIAnaAcceptance_SOURCE_DIR}/include/PIAnaAcc_linkdef.h)


install(TARGETS PiAnaAcc DESTINATION lib)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libPiAnaAcc.rootmap
        ${CMAKE_CURRENT_BINARY_DIR}/libPiAnaAcc_rdict.pcm
        DESTINATION lib)
