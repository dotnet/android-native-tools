set(STRIP_DRIVER_SOURCES
  main.cc
  llvm-strip.cc
  ../shared/app.cc
  ../shared/process.cc
)

if(WIN32)
  list(APPEND STRIP_DRIVER_SOURCES
    ../shared/app.windows.cc
    ../shared/process.windows.cc
  )
else()
  list(APPEND STRIP_DRIVER_SOURCES
    ../shared/app.posix.cc
    ../shared/process.posix.cc
  )
endif()

add_executable(
  llvm-strip
  ${STRIP_DRIVER_SOURCES}
)

# SYSTEM disables warnings produced by cxxopts headers, which we can do nothing about
target_include_directories(llvm-strip SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/../external/cxxopts/include)

if(WIN32)
  target_link_libraries(
    llvm-strip
    shlwapi
  )
endif()
