#   创建共享库的宏
macro(QT_ADD_SHARED_LIB TARGET_NAME)
    add_library(${TARGET_NAME} SHARED ${ARGN})
endmacro(QT_ADD_SHARED_LIB)

#   创建静态库的宏
macro(QT_ADD_STATIC_LIB TARGET_NAME)
    add_library(${TARGET_NAME} STATIC ${ARGN})
endmacro(QT_ADD_STATIC_LIB)