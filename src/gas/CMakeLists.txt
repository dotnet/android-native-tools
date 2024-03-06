set(GAS_DRIVER_SOURCES
  gas.cc
  llvm_mc_runner.cc
  llvm_mc_runner_arm32.cc
  llvm_mc_runner_arm64.cc
  llvm_mc_runner_x64.cc
  llvm_mc_runner_x86.cc
  main.cc
  ../shared/app.cc
  ../shared/process.cc
  )

set(ARCH_PREFIXES
  aarch64-linux-android-
  arm-linux-androideabi-
  i686-linux-android-
  x86_64-linux-android-
  )

if(WIN32)
  list(APPEND GAS_DRIVER_SOURCES
    ../shared/app.windows.cc
    ../shared/process.windows.cc
    )
else()
  list(APPEND GAS_DRIVER_SOURCES
    ../shared/app.posix.cc
    ../shared/process.posix.cc
    )
endif()

add_executable(
  as
  ${GAS_DRIVER_SOURCES}
  )

# SYSTEM disables warnings produced by cxxopts headers, which we can do nothing about
target_include_directories(as SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/../external/cxxopts/include)

if(WIN32)
  target_link_libraries(
    as
    shlwapi
  )
else()
  set(TARGET_DIR "${CMAKE_BINARY_DIR}/bin")
  set(AS_NAME as)
  set(PREFIXES ${ARCH_PREFIXES})
  string(REPLACE ";" " " PREFIXES "${PREFIXES}")
endif()
