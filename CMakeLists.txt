cmake_minimum_required(VERSION 3.20)
project(softadastra-backend-vix LANGUAGES CXX)

# ===== C++ standard =====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(VIX_ENABLE_SANITIZERS "Enable ASan/UBSan (dev only)" OFF)

# ===== Prefixes (adjust if your install lives elsewhere) =====
list(APPEND CMAKE_PREFIX_PATH 
  "/usr/local"
  "/usr/local/lib/cmake"
  "/usr/local/lib/cmake/Vix"
)

# ❗ Disable ORM completely in generated apps (core-only)
set(CMAKE_DISABLE_FIND_PACKAGE_VixOrm ON)

# ===== System deps first =====
find_package(Threads REQUIRED)
find_package(OpenSSL QUIET)
find_package(Boost REQUIRED COMPONENTS system thread filesystem)

# Optional: create a small shim if Boost::filesystem isn't exported
if (NOT TARGET Boost::filesystem)
  if (Boost_FOUND AND Boost_FILESYSTEM_LIBRARY)
    add_library(Boost::filesystem UNKNOWN IMPORTED)
    set_target_properties(Boost::filesystem PROPERTIES
      IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
    message(STATUS "Shim: Using Boost::filesystem from ${Boost_FILESYSTEM_LIBRARY}")
  else()
    message(FATAL_ERROR "Boost::filesystem not found and no library path provided.")
  endif()
endif()

# ===== Vix (core-only) =====
find_package(Vix CONFIG REQUIRED)

# ===== App target =====
add_executable(softadastra-backend-vix src/main.cpp)

target_link_libraries(softadastra-backend-vix PRIVATE
  Vix::vix
  Boost::system Boost::thread Boost::filesystem
  Threads::Threads
)

if (OpenSSL_FOUND)
  target_link_libraries(softadastra-backend-vix PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if (UNIX AND NOT APPLE)
  target_link_libraries(softadastra-backend-vix PRIVATE dl)
endif()

# Warnings
if (MSVC)
  target_compile_options(softadastra-backend-vix PRIVATE /W4 /permissive-)
else()
  target_compile_options(softadastra-backend-vix PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Sanitizers (optional)
if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(softadastra-backend-vix PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
  target_link_options(softadastra-backend-vix PRIVATE -fsanitize=address,undefined)
endif()

# Run helper
add_custom_target(run
  COMMAND $<TARGET_FILE:softadastra-backend-vix>
  DEPENDS softadastra-backend-vix
  USES_TERMINAL
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(softadastra-backend-vix PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  INSTALL_RPATH_USE_LINK_PATH ON
)
