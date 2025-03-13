# nativeRAG

## 概述
这是一个由C++编写的简单RAG系统（包含前后端），旨在依赖尽可能少的第三方库情况下，实现基本的RAG功能，并在任何电脑（Windows系统）都能开箱即用，支持完全封闭的内网，不需要从互联网拉取任何东西（除了大模型）。

*做这个的原因是用了[Dify](https://github.com/langgenius/dify)后，觉得Dify虽好，一应俱全，但太庞大了，特别是还要用Docker进行部署，拉取一堆东西，例如Nginx之类的，不是很喜欢*

## 基本组成
- **Qt6.8**：字符串、Json处理，多线程等
- **cpp-httplib**：http服务端，并路由前端页面
- **faiss**：向量检索库
- **Ollama**：本地部署大模型的后端程序
	- LLM 任意的大语言模型，用于自然语言的问答，例如DeepSeek、Qwen等
	- Embedding 文本嵌入模型，用于将文本转换为向量数据，例如bge-m3

## 编译依赖
### 编译工具链
- CMake >= 3.30
- Visual Studio 2022（勾选`使用C++进行桌面开发`）

### vcpkg
用于导入第三方库cpp-httplib、faiss
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat
```
然后将vcpkg.exe的路径添加到系统环境变量PATH里

### Qt6.8.2
可以直接使用在线安装包，不过还是建议自己下载源码来构建。目前只需编译qt-base模块即可。

#### 参考
https://doc.qt.io/qt-6/getting-sources-from-git.html

https://doc.qt.io/qt-6/windows-building.html

https://doc.qt.io/qt-6/configure-options.html

#### 下载源码
`git clone --branch v6.8.2 git://code.qt.io/qt/qt5.git .\src`

#### 生成
```
mkdir build
cd build
..\src\configure.bat -init-submodules -submodules qtbase
..\src\configure.bat -debug-and-release -prefix <path-to-qt>
cmake --build . --parallel
cmake --install .
cmake --install . --config debug
```
*上述步骤有两次install，第一次默认是只安装release*
最后将`<path-to-qt>\bin`添加到系统环境变量PATH中里

### Ollama
Ollama需要自行去[官网下载](https://ollama.com/)并安装运行，根据需要自行拉取所需的大模型

## 构建
clone源码并进入到源码目录下
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
```

最后打开build/nativeRAG.sln进行编译和调试

## 配置
修改[config.json](./src/config.json)，可简单配置RAG服务端的功能
- server
	- port：服务端监听的端口号
- model
	- host：ollama的地址
	- chat：生成对话的模型，推荐deepseek和qwen
	- embed：本文转嵌入数据的模型，推荐bge-m3，对中文支持友好
- dataset
	- 知识库/数据集的数组，需要给出name和path字段，name代表知识库的名称，前端可以选择，path代表目录所在的路径，建议使用内网共享的文件夹；服务端启动后，会遍历处理这些文件夹里的文件（目前仅支持纯文本文件），将其生成的嵌入数据保存在SQLite数据库中