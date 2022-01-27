set(GAS_DRIVER_SOURCES
  gas.cc
  llvm_mc_runner.cc
  llvm_mc_runner_arm32.cc
  llvm_mc_runner_arm64.cc
  llvm_mc_runner_x64.cc
  llvm_mc_runner_x86.cc
  main.cc
  process.cc
  )

set(ARCH_PREFIXES
  aarch64-linux-android-
  arm-linux-androideabi-
  i686-linux-android-
  x86_64-linux-android-
  )

if(WIN32)
  list(APPEND GAS_DRIVER_SOURCES
    gas.windows.cc
    process.windows.cc
    )
else()
  list(APPEND GAS_DRIVER_SOURCES
    gas.posix.cc
    process.posix.cc
    )
endif()

add_executable(
  as
  ${GAS_DRIVER_SOURCES}
  )

if(WIN32)
  target_link_options(
    as
    PRIVATE
    -static-libstdc++
    -static-libgcc
    )

  target_link_libraries(
    as
    shlwapi
    -static
    winpthread
    -dynamic
    )

  foreach(PREFIX ${ARCH_PREFIXES})
    set(AS_NAME ${PREFIX}as)
    set(TARGET_NAME_ARG "@name=${AS_NAME}")

    configure_file(gas.cmd.in ${CMAKE_BINARY_DIR}/bin/${AS_NAME}.cmd)
  endforeach()
else()
  set(TARGET_DIR "${CMAKE_BINARY_DIR}/bin")
  set(AS_NAME as)
  set(PREFIXES ${ARCH_PREFIXES})
  string(REPLACE ";" " " PREFIXES "${PREFIXES}")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/xatu-make-symlinks.sh.in
    ${CMAKE_BINARY_DIR}/xatu-make-symlinks.sh
    USE_SOURCE_PERMISSIONS
    @ONLY
    NEWLINE_STYLE UNIX
    )

  add_custom_command(
    TARGET as
    POST_BUILD
    COMMAND ${CMAKE_BINARY_DIR}/xatu-make-symlinks.sh
    )
endif()
