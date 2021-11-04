set(GAS_DRIVER_SOURCES
  gas.cc
  main.cc
  )

if(WIN32)
  list(APPEND GAS_DRIVER_SOURCES
    gas.windows.cc
    )
else()
  list(APPEND GAS_DRIVER_SOURCES
    gas.posix.cc
    )
endif()

add_executable(
  as
  ${GAS_DRIVER_SOURCES}
  )

if(WIN32)
  target_link_libraries(
    as
    shlwapi
    )
endif()
