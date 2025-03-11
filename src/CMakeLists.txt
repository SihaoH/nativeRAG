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

# øΩ±¥≈‰÷√Œƒº˛
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/dataset.json ${CMAKE_OUTPUT_DIRECTORY_INTDIR}/dataset.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dataset.json
)
