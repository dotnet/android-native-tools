set(LD_SOURCES
  main.cc
  ld.cc
  ../shared/app.cc
  ../shared/process.cc
)

if(WIN32)
  list(APPEND LD_SOURCES
    ../shared/app.windows.cc
    ../shared/process.windows.cc
  )
else()
  list(APPEND LD_SOURCES
    ../shared/app.posix.cc
    ../shared/process.posix.cc
  )
endif()

add_executable(
  ld
  ${LD_SOURCES}
)

# SYSTEM disables warnings produced by cxxopts headers, which we can do nothing about
target_include_directories(ld SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/../external/cxxopts/include)

if(WIN32)
  target_link_libraries(
    ld
    shlwapi
  )
endif()
