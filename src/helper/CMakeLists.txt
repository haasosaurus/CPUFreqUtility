
set(cpu_freq_helper_SRCS
    cpu_freq_helper.cpp)

qt5_wrap_cpp (HELPER_MOC_SOURCES cpu_freq_helper.h)

add_executable(cpu_freq_helper ${cpu_freq_helper_SRCS} ${HELPER_MOC_SOURCES})
target_link_libraries(cpu_freq_helper KF5::Auth)

#message("KAUTH_HELPER_INSTALL_DIR: ${CMAKE_INSTALL_PREFIX}/${KAUTH_HELPER_INSTALL_DIR}")
install(TARGETS cpu_freq_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

#message("KAUTH_POLICY_FILES_INSTALL_DIR: ${KAUTH_POLICY_FILES_INSTALL_DIR}")
kauth_install_helper_files(
    cpu_freq_helper
    org.freedesktop.auth.cpufrequtility
    root)

#message("DBUS_SYSTEM_SERVICES_INSTALL_DIR: ${CMAKE_INSTALL_PREFIX}/${DBUS_SYSTEM_SERVICES_INSTALL_DIR}")

kauth_install_actions(
    org.freedesktop.auth.cpufrequtility
    actions.actions)
