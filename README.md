# nativeRAG

## 概述
这是一个C++编写的RAG程序（包含前后端），旨在依赖尽可能少的第三方库情况下，实现基本的RAG功能，并在任何电脑都能开箱即用。

*做这个的原因是用了[Dify](https://github.com/langgenius/dify)后，觉得Dify虽好，一应俱全，但太庞大了，特别是还要用Docker进行部署，个人很不喜欢*

## 基本组成
- Qt6.8 基础功能、后端服务、前端界面
- faiss 向量检索库
- Ollama 支持本地部署大模型的后端程序
	- LLM 任意的大语言模型，用于自然语言的问答，例如DeepSeek、Qwen等
	- Embedding 文本嵌入模型，用于将文本转换为向量数据，例如bga-m3

