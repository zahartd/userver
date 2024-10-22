_userver_module_begin(
    NAME SQLite3
    VERSION 3.46.1
)

_userver_module_find_include(
    NAMES sqlite3/sqlite3.h
)

_userver_module_find_library(
    NAMES SQLite3
)

_userver_module_end()
