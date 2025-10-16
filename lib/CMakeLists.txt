# lib/CMakeLists.txt

function(sa_add_module MODULE_NAME SRC_DIR INC_ROOT)
  # Cherche des .cpp dans le sous-dossier du module (relatif au CMake courant)
  file(GLOB_RECURSE MOD_SOURCES
       "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cpp")

  if (MOD_SOURCES STREQUAL "")
    message(WARNING "Module '${MODULE_NAME}' has no sources in ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} -> creating INTERFACE target.")
    add_library(${MODULE_NAME} INTERFACE)
    target_include_directories(${MODULE_NAME} INTERFACE
      "${INC_ROOT}"
      "${SA_INCLUDE_DIR}"
    )
    target_link_libraries(${MODULE_NAME} INTERFACE
      Vix::vix
      Threads::Threads
      nlohmann_json::nlohmann_json
    )
  else()
    add_library(${MODULE_NAME} STATIC ${MOD_SOURCES})
    target_include_directories(${MODULE_NAME} PUBLIC
      "${INC_ROOT}"
      "${SA_INCLUDE_DIR}"
    )
    target_link_libraries(${MODULE_NAME} PUBLIC
      Vix::vix
      Threads::Threads
      nlohmann_json::nlohmann_json
    )
    if (MSVC)
      target_compile_options(${MODULE_NAME} PRIVATE /W4 /permissive-)
    else()
      target_compile_options(${MODULE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
  endif()
endfunction()

# Namespaces
add_subdirectory(softadastra)
add_subdirectory(adastra)
