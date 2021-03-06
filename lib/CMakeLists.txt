set(PROJECT_NAME typeart_llvm)
set(TARGETS_EXPORT_NAME ${PROJECT_NAME}-targets)

add_subdirectory(analysis)

set(PASS_SOURCES
  TypeARTPass.cpp
  support/TypeUtil.cpp
  support/InstrumentationHelper.cpp
  TypeManager.cpp
)

make_llvm_module(typeartpass
  "${PASS_SOURCES}"
  LINK_LIBS
    typelib
  DEPENDS
    meminstfinderpass
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/typelib
    ${PROJECT_SOURCE_DIR}/runtime
)

target_project_compile_options(typeartpass)
target_project_compile_definitions(typeartpass
  PRIVATE_DEFS
    LOG_LEVEL=${LOG_LEVEL}
)

if(SHOW_STATS)
  target_project_compile_definitions(typeartpass
    PRIVATE_DEFS 
      LLVM_ENABLE_STATS
  )
endif()

install(
  TARGETS typeartpass
  EXPORT ${TARGETS_EXPORT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# also handles subdir meminstfinder
install(
  EXPORT ${TARGETS_EXPORT_NAME}
  NAMESPACE typeart::
  DESTINATION lib/cmake
)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION lib/cmake
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/cmake
)
