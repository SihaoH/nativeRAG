set(TARGET server)

set(CMAKE_AUTOMOC ON)
set(SOURCES
    TypeDefine.h
    Database.h
    Database.cpp
    Dataset.h
    Dataset.cpp
    Embeddings.h
    Embeddings.cpp
    Logger.h
    Logger.cpp
    Server.h
    Server.cpp
    main.cpp
    ModelAdapter.h
    ModelAdapter.cpp
)
add_executable(${TARGET}
    ${SOURCES}
)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(faiss CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${TARGET}
    PRIVATE
    Qt6::Core
    Qt6::Sql
    faiss
    httplib::httplib
    spdlog::spdlog_header_only
)

# 修改从VS调试时的工作目录路径
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY_INTDIR})

# 拷贝配置文件
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/config.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config.json
)

# 拷贝client文件夹
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/client ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/client
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/client/index.html
        ${CMAKE_CURRENT_SOURCE_DIR}/client/style.css
        ${CMAKE_CURRENT_SOURCE_DIR}/client/app.js
)
