# lib/adastra/CMakeLists.txt

# Each module lives under lib/adastra/<module>/*.cpp
# Public headers are under include/adastra/...

# Core building blocks
sa_add_module(adastra_core    "core"     "${SA_INCLUDE_ADA}")
sa_add_module(adastra_utils   "utils"    "${SA_INCLUDE_ADA}")
sa_add_module(adastra_tools   "tools"    "${SA_INCLUDE_ADA}")
sa_add_module(adastra_config  "config"   "${SA_INCLUDE_ADA}")

# Feature modules (enable/skip per project)
sa_add_module(adastra_crypto  "crypto"   "${SA_INCLUDE_ADA}")
sa_add_module(adastra_network "network"  "${SA_INCLUDE_ADA}")
sa_add_module(adastra_storage "storage"  "${SA_INCLUDE_ADA}")
sa_add_module(adastra_ai      "ai"       "${SA_INCLUDE_ADA}")
sa_add_module(adastra_db      "database" "${SA_INCLUDE_ADA}")
sa_add_module(adastra_tests   "test_utils" "${SA_INCLUDE_ADA}")

# Optional deps at module level (if those modules actually use them)
if (SA_WITH_OPENSSL AND OpenSSL_FOUND)
  target_link_libraries(adastra_crypto  PUBLIC OpenSSL::SSL OpenSSL::Crypto)
  target_link_libraries(adastra_network PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

if (SA_WITH_SQLITE AND SQLite3_FOUND)
  target_link_libraries(adastra_db PUBLIC SQLite::SQLite3)
endif()

if (SA_WITH_MYSQL AND SA_MYSQL_AVAILABLE)
  target_link_libraries(adastra_db PUBLIC MySQLCppConn::MySQLCppConn)
endif()

# (Optional) Aggregate interface target to consume all Adastra modules at once
add_library(adastra INTERFACE)
target_link_libraries(adastra INTERFACE
  adastra_core
  adastra_utils
  adastra_tools
  adastra_config
  adastra_crypto
  adastra_network
  adastra_storage
  adastra_ai
  adastra_db
  adastra_tests
  Threads::Threads
  nlohmann_json::nlohmann_json
)
target_include_directories(adastra INTERFACE "${SA_INCLUDE_DIR}")
