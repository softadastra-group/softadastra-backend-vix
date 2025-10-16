# lib/softadastra/CMakeLists.txt

# Les sources sont sous lib/softadastra/core/*.cpp et lib/softadastra/commerce/*.cpp
sa_add_module(sa_core     "core"     "${SA_INCLUDE_SOFT}")
sa_add_module(sa_commerce "commerce" "${SA_INCLUDE_SOFT}")

# Liens optionnels (si besoin)
if (SA_WITH_OPENSSL AND OpenSSL_FOUND)
  target_link_libraries(sa_core     PUBLIC OpenSSL::SSL OpenSSL::Crypto)
  target_link_libraries(sa_commerce PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()
if (SA_WITH_SQLITE AND SQLite3_FOUND)
  target_link_libraries(sa_core     PUBLIC SQLite::SQLite3)
  target_link_libraries(sa_commerce PUBLIC SQLite::SQLite3)
endif()
if (SA_WITH_MYSQL AND SA_MYSQL_AVAILABLE)
  target_link_libraries(sa_core     PUBLIC MySQLCppConn::MySQLCppConn)
  target_link_libraries(sa_commerce PUBLIC MySQLCppConn::MySQLCppConn)
endif()
