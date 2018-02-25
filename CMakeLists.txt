if (NOT CMAKE_INSTALL_PREFIX)
 set (CMAKE_INSTALL_PREFIX /usr)
endif ()

#message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0037 OLD)

project (CPUFreqUtility)

find_package(KF5Auth          REQUIRED)
find_package(KF5Notifications REQUIRED)
add_subdirectory(src)

#message("installing ${PROJECT_NAME}.desktop to: ${CMAKE_INSTALL_PREFIX}/share/applications")
install ( FILES       ${PROJECT_NAME}.desktop
          DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications )
