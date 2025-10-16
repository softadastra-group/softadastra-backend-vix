cmake_minimum_required(VERSION 3.20)

# ────────────────────────────────────────────────────────────────
# 🛠️  Project Metadata
# ────────────────────────────────────────────────────────────────
project(softadastra-backend-vix
  VERSION 0.1.0
  DESCRIPTION "Softadastra backend in modern C++ (Vix.cpp + modular libraries)"
  LANGUAGES CXX
)

# ────────────────────────────────────────────────────────────────
# 🧱 C++ Standard and Global Flags
# ────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SA_ENABLE_OPTIMIZATION "Enable -O3 optimization" OFF)
option(SA_ENABLE_SANITIZERS  "Enable ASan/UBSan (dev only)" OFF)

if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -g)
  if(SA_ENABLE_OPTIMIZATION)
    add_compile_options(-O3)
  endif()
endif()

# Build all libs with PIC so we can switch to SHARED later if needed
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Standardize output dirs (bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ────────────────────────────────────────────────────────────────
# 📦 Options & Prefixes
# ────────────────────────────────────────────────────────────────
option(SA_WITH_OPENSSL "Link OpenSSL if found" ON)
option(SA_WITH_SQLITE  "Link SQLite3 if found" ON)
option(SA_WITH_MYSQL   "Link MySQL Connector/C++ if found" OFF)  # OFF by default

# If Vix is installed in /usr/local, help CMake find it.
list(APPEND CMAKE_PREFIX_PATH
  "/usr/local"
  "/usr/local/lib/cmake"
  "/usr/local/lib/cmake/Vix"
)

# ❗ Keep ORM off for now (core-only app)
set(CMAKE_DISABLE_FIND_PACKAGE_VixOrm ON)

# ────────────────────────────────────────────────────────────────
# 🔗 System/3rdparty Deps
# ────────────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

if(SA_WITH_OPENSSL)
  find_package(OpenSSL QUIET)
endif()

if(SA_WITH_SQLITE)
  find_package(SQLite3 QUIET)
endif()

# nlohmann/json (header-only) — handy across modules
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Optional: MySQL Connector/C++ (manual find; robust across distros)
if(SA_WITH_MYSQL)
  find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_connection.h
    PATHS /usr/include/cppconn /usr/include/mysql-cppconn-8 /usr/include/mysql /usr/local/include
  )
  find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn mysqlcppconn8
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
  )
  if(MYSQLCPPCONN_INCLUDE_DIR AND MYSQLCPPCONN_LIBRARY)
    add_library(MySQLCppConn::MySQLCppConn UNKNOWN IMPORTED)
    set_target_properties(MySQLCppConn::MySQLCppConn PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MYSQLCPPCONN_INCLUDE_DIR}"
      IMPORTED_LOCATION "${MYSQLCPPCONN_LIBRARY}"
    )
    message(STATUS "✅ MySQL C++ Connector found (manual import).")
    set(SA_MYSQL_AVAILABLE TRUE)
  else()
    message(STATUS "ℹ️ MySQL C++ Connector not found; DB features depending on it will be skipped.")
    set(SA_MYSQL_AVAILABLE FALSE)
  endif()
endif()


# Try to find Boost components Vix might require (filesystem, system, thread)
find_package(Boost QUIET COMPONENTS filesystem system thread)

# Create shims if the imported targets aren't provided by your Boost cmake config
# (Some distros ship Boost libraries without proper Boost:: targets)
if (NOT TARGET Boost::filesystem)
  if (Boost_FOUND AND Boost_FILESYSTEM_LIBRARY)
    add_library(Boost::filesystem UNKNOWN IMPORTED)
    set_target_properties(Boost::filesystem PROPERTIES
      IMPORTED_LOCATION "${Boost_FILESYSTEM_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
    )
    message(STATUS "Shim: Boost::filesystem -> ${Boost_FILESYSTEM_LIBRARY}")
  else()
    message(FATAL_ERROR "Vix needs Boost::filesystem, but it was not found.")
  endif()
endif()

if (NOT TARGET Boost::system)
  if (Boost_FOUND AND Boost_SYSTEM_LIBRARY)
    add_library(Boost::system UNKNOWN IMPORTED)
    set_target_properties(Boost::system PROPERTIES
      IMPORTED_LOCATION "${Boost_SYSTEM_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
    )
    message(STATUS "Shim: Boost::system -> ${Boost_SYSTEM_LIBRARY}")
  endif()
endif()

if (NOT TARGET Boost::thread)
  if (Boost_FOUND AND Boost_THREAD_LIBRARY)
    add_library(Boost::thread UNKNOWN IMPORTED)
    set_target_properties(Boost::thread PROPERTIES
      IMPORTED_LOCATION "${Boost_THREAD_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
    )
    message(STATUS "Shim: Boost::thread -> ${Boost_THREAD_LIBRARY}")
  endif()
endif()

# ---- Now this will succeed because the Boost::* targets exist ----
find_package(Vix CONFIG REQUIRED)  # provides Vix::vix (and Vix::core internally)


# Vix (core-only)
find_package(Vix CONFIG REQUIRED)  # provides Vix::vix

# ────────────────────────────────────────────────────────────────
# 📥 Public Headers (two namespaces)
# ────────────────────────────────────────────────────────────────
# We expose *both* include/softadastra and include/adastra to all targets via module libs.
set(SA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SA_INCLUDE_SOFT "${SA_INCLUDE_DIR}/softadastra")
set(SA_INCLUDE_ADA  "${SA_INCLUDE_DIR}/adastra")

# ────────────────────────────────────────────────────────────────
# 🧩 Modules (static libs) under lib/
# ────────────────────────────────────────────────────────────────
add_subdirectory(lib)

# ────────────────────────────────────────────────────────────────
# 🚀 App Executable
# ────────────────────────────────────────────────────────────────
add_executable(softadastra-backend-vix
  src/main.cpp
)

# The app should see public headers (through linked modules it already will),
# but we also add the root include/ to be explicit for main.cpp includes.
target_include_directories(softadastra-backend-vix
  PUBLIC
    "${SA_INCLUDE_DIR}"
)

# Link domain modules (from lib/...), plus Vix core and Threads
# NOTE: The concrete module targets are defined in lib/*.cmake; adjust if you add/remove modules.
target_link_libraries(softadastra-backend-vix
  PRIVATE
    sa_core
    sa_commerce
    adastra_core
    adastra_utils
    Vix::vix
    Threads::Threads
)

# Conditionally add crypto/db deps at the APP level if your main uses them.
if(SA_WITH_OPENSSL AND OpenSSL_FOUND)
  target_link_libraries(softadastra-backend-vix PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
if(SA_WITH_SQLITE AND SQLite3_FOUND)
  target_link_libraries(softadastra-backend-vix PRIVATE SQLite::SQLite3)
endif()
if(SA_WITH_MYSQL AND SA_MYSQL_AVAILABLE)
  target_link_libraries(softadastra-backend-vix PRIVATE MySQLCppConn::MySQLCppConn)
endif()

# Linux: sometimes useful for dlopen plugins
if(UNIX AND NOT APPLE)
  target_link_libraries(softadastra-backend-vix PRIVATE dl)
endif()

# Sanitizers (dev only, non-MSVC)
if(SA_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(softadastra-backend-vix PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=address,undefined)
  target_link_options(softadastra-backend-vix    PRIVATE -fsanitize=address,undefined)
endif()

# LTO/IPO for Release if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_ok OUTPUT ipo_msg)
if(ipo_ok)
  set_property(TARGET softadastra-backend-vix PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "IPO/LTO not supported: ${ipo_msg}")
endif()

# Convenience run target
add_custom_target(run
  COMMAND $<TARGET_FILE:softadastra-backend-vix>
  DEPENDS softadastra-backend-vix
  USES_TERMINAL
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install (binary + headers)
install(TARGETS softadastra-backend-vix RUNTIME DESTINATION bin)
install(DIRECTORY "${SA_INCLUDE_DIR}/" DESTINATION include)
