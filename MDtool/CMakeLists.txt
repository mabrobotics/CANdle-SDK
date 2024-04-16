cmake_minimum_required(VERSION 3.8)

if(MAKE_TESTS)
    add_subdirectory(test)
    return()
endif()

project(MDtool)
set(EXECUTABLE MDtool)
set(MDtool_VERSION 0.0.1)

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)
set(CLI11_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CLI11/inc/)
set(SPDLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mdtool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/MD80Downloader/MD80Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/CANdleDownloader/CANdleDownloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/Checksum/Sha256.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/Checksum/Checksum.cpp)

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

target_include_directories(${EXECUTABLE} PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/CANdleDownloader/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/MD80Downloader/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/BinaryParser/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/Checksum/
${CMAKE_CURRENT_SOURCE_DIR}/src/Flashers/Commons/
${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigParser/)

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${CLI11_INCLUDE_DIR}
    ${mINI_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})


add_custom_command(TARGET ${EXECUTABLE} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/MDtool/configs $<TARGET_FILE_DIR:${EXECUTABLE}>)

if(WIN32)
    target_link_libraries(${EXECUTABLE} -static CANdle_lib -static-libstdc++ -static-libgcc)
    install(TARGETS ${EXECUTABLE} RUNTIME DESTINATION ${INSTALL_PATH} COMPONENT MDtool)
    install(DIRECTORY "configs/" DESTINATION bin COMPONENT Configuration)
else()
    target_link_libraries(${EXECUTABLE} pthread CANdle_lib -static-libstdc++ -static-libgcc)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "wget, libusb-1.0-0, libglfw3, rpm")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})

    install(TARGETS ${PROJECT_NAME}
        DESTINATION /usr/local/bin)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/configs
        DESTINATION /usr/local/share/mdtool)
endif()

set(CPACK_PROJECT_NAME "MDtool")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION "Console tool for configuring and setting up MAB's MD10 and MD80 via CANdle")
set(CPACK_NSIS_CREATE_UNINSTALLER ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)

include(CPack)
