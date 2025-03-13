set(TARGET server)
set(CMAKE_AUTOMOC ON)

# 将源文件分为库文件和可执行文件
set(LIB_SOURCES
    TypeDefine.h
    Database.h
    Database.cpp
    Dataset.h
    Dataset.cpp
    Embeddings.h
    Embeddings.cpp
    ModelAdapter.h
    ModelAdapter.cpp
    Logger.h
    Logger.cpp
)

set(EXE_SOURCES
    Server.h
    Server.cpp
    main.cpp
)

# 创建静态库
add_library(${TARGET}_lib STATIC ${LIB_SOURCES})

# 创建可执行文件
add_executable(${TARGET} ${EXE_SOURCES})

# 设置库的依赖
find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(faiss CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${TARGET}_lib
    PUBLIC
    Qt6::Core
    Qt6::Sql
    httplib::httplib
    PRIVATE
    faiss
    spdlog::spdlog_header_only
)

# 可执行文件链接库
target_link_libraries(${TARGET}
    PRIVATE
    ${TARGET}_lib
)

# 修改从VS调试时的工作目录路径
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY_INTDIR})

# 为config.json创建符号链接
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/config.json
)

# 为client文件夹创建符号链接
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/client ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/client
)

install(TARGETS ${TARGET} RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/client DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION .)

# 安装所有dll文件到目标目录
install(CODE "
    file(GLOB DLL_FILES \"${CMAKE_OUTPUT_INSTALL_INTDIR}/*.dll\")
    file(INSTALL \${DLL_FILES} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/.\")
")

# 安装Qt的运行时依赖
install(CODE "
include(\"${QT_DEPLOY_SUPPORT}\")
set(QT_DEPLOY_BIN_DIR \".\")
qt_deploy_runtime_dependencies(
    EXECUTABLE \"${CMAKE_OUTPUT_INSTALL_INTDIR}/$<TARGET_FILE_NAME:${TARGET}>\" NO_TRANSLATIONS
)")
