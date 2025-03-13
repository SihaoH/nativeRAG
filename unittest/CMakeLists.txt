set(TARGET unittest)

set(CMAKE_AUTOMOC ON)

# 定义所有测试源文件
set(TEST_SOURCES
    TestDatabase.cpp
    TestDataset.cpp
    TestModelAdapter.cpp
    TestEmbeddings.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Test)

# 为每个测试源文件创建单独的可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 获取文件名（不含扩展名）作为目标名
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # 链接依赖库
    target_link_libraries(${TEST_NAME}
        PRIVATE
        server_lib
        Qt6::Test
    )
    
    # 设置包含目录
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    
    # 设置工作目录和解决方案文件夹
    set_target_properties(${TEST_NAME} PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_OUTPUT_DIRECTORY_INTDIR}
        FOLDER "unittest"  # 将所有测试项目放在unittest文件夹下
    )
    
    # 添加测试
    add_test(
        NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
