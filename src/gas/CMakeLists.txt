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
else()
  set(TARGET_DIR "${CMAKE_BINARY_DIR}/bin")
  set(AS_NAME as)
  set(PREFIXES ${ARCH_PREFIXES})
  string(REPLACE ";" " " PREFIXES "${PREFIXES}")
endif()
