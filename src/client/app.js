const { createApp } = Vue

createApp({
    data() {
        return {
            history: [],
            currentChatId: null,
            currentChat: {
                id: null,
                title: '',
                messages: [],
                context: null
            },
            datasets: [],
            selectedDataset: localStorage.getItem('selectedDataset') || '',
            userInput: '',
            loading: false,
            tooltipVisible: false,
            tooltipContent: [],
            isDragging: false,
            startX: 0,
            startY: 0,
            startWidth: 0,
            startHeight: 0,
            startChatHeight: 0,
            startInputHeight: 0
        }
    },
    
    mounted() {
        // 侧边栏拖动
        const sidebarHandle = document.querySelector('.sidebar-handle')
        sidebarHandle.addEventListener('mousedown', this.startDragSidebar)
        
        // 聊天窗口拖动
        const chatHandle = document.querySelector('.chat-handle')
        chatHandle.addEventListener('mousedown', this.startDragChat)
        
        document.addEventListener('mousemove', this.onDrag)
        document.addEventListener('mouseup', this.stopDrag)
        
        // 恢复上次的布局
        const savedLayout = localStorage.getItem('chatLayout')
        if (savedLayout) {
            const layout = JSON.parse(savedLayout)
            const chatWindow = this.$refs.chatWindow
            const inputArea = document.querySelector('.input-area')
            const handle = document.querySelector('.chat-handle')
            
            if (layout.chatHeight && layout.inputHeight) {
                chatWindow.style.height = `${layout.chatHeight}%`
                inputArea.style.height = `${layout.inputHeight}%`
                handle.style.bottom = `${layout.inputHeight}%`
            }
        }
    },
    
    async created() {
        // 先加载数据集列表
        await this.loadDatasets()
        // 再加载历史记录
        this.loadHistory()
        // 如果没有当前会话，创建新会话
        if (!this.currentChatId) {
            this.createNewChat()
        }
    },
    
    watch: {
        // 监听数据集选择变化
        selectedDataset(newValue) {
            localStorage.setItem('selectedDataset', newValue)
        }
    },
    
    methods: {
        // 拖动相关方法
        startDragSidebar(e) {
            this.isDragging = 'sidebar'
            this.startX = e.clientX
            this.startWidth = this.$refs.sidebar.offsetWidth
        },
        
        startDragChat(e) {
            this.isDragging = 'chat'
            this.startY = e.clientY
            
            // 添加拖动时的视觉反馈
            e.target.classList.add('dragging')
            
            // 记录初始高度
            const chatWindow = this.$refs.chatWindow
            const inputArea = document.querySelector('.input-area')
            this.startChatHeight = chatWindow.offsetHeight
            this.startInputHeight = inputArea.offsetHeight
        },
        
        onDrag(e) {
            if (!this.isDragging) return
            
            if (this.isDragging === 'sidebar') {
                const width = this.startWidth + (e.clientX - this.startX)
                if (width >= 150 && width <= 500) {
                    this.$refs.sidebar.style.width = `${width}px`
                }
            } else if (this.isDragging === 'chat') {
                const deltaY = this.startY - e.clientY
                const container = this.$refs.chatWindow.parentElement
                const totalHeight = container.offsetHeight
                
                // 计算新的高度百分比 - 反转加减关系
                let chatHeightPercent = ((this.startChatHeight - deltaY) / totalHeight) * 100
                let inputHeightPercent = ((this.startInputHeight + deltaY) / totalHeight) * 100
                
                // 应用限制
                if (chatHeightPercent >= 50 && chatHeightPercent <= 80 &&
                    inputHeightPercent >= 20 && inputHeightPercent <= 50) {
                    
                    const chatWindow = this.$refs.chatWindow
                    const inputArea = document.querySelector('.input-area')
                    const handle = document.querySelector('.chat-handle')
                    
                    // 更新元素高度
                    chatWindow.style.height = `${chatHeightPercent}%`
                    inputArea.style.height = `${inputHeightPercent}%`
                    handle.style.bottom = `${inputHeightPercent}%`
                    
                    // 保存布局状态
                    localStorage.setItem('chatLayout', JSON.stringify({
                        chatHeight: chatHeightPercent,
                        inputHeight: inputHeightPercent
                    }))
                }
            }
        },
        
        stopDrag() {
            if (this.isDragging === 'chat') {
                // 移除拖动时的视觉反馈
                document.querySelector('.chat-handle').classList.remove('dragging')
            }
            this.isDragging = false
        },
        
        // 修改引用提示相关方法
        showTooltip(event, refs) {
            const tooltip = this.$refs.tooltip
            this.tooltipContent = refs.map(ref => ref.text)
            
            // 计算位置
            const rect = event.target.getBoundingClientRect()
            const tooltipWidth = 400
            
            // 初始位置在链接的右侧
            let left = rect.right + 10
            let top = rect.top
            
            // 如果右侧空间不足，显示在左侧
            if (left + tooltipWidth > window.innerWidth) {
                left = rect.left - tooltipWidth - 10
            }
            
            // 如果tooltip会超出底部，向上调整
            tooltip.style.display = 'block' // 临时显示以获取高度
            const tooltipHeight = tooltip.offsetHeight
            if (top + tooltipHeight > window.innerHeight) {
                top = window.innerHeight - tooltipHeight - 10
            }
            
            // 设置位置并显示
            tooltip.style.left = `${Math.max(10, left)}px`
            tooltip.style.top = `${Math.max(10, top)}px`
            this.tooltipVisible = true
        },
        
        hideTooltip() {
            this.tooltipVisible = false
        },
        
        // 修改引用分组方法
        groupedReferences(references) {
            if (!references) return {}
            const groups = {}
            references.forEach(ref => {
                // 提取文件名
                const fileName = ref.file.split('/').pop()
                if (!groups[ref.file]) {
                    groups[ref.file] = {
                        displayName: fileName,  // 添加显示名称
                        refs: []
                    }
                }
                groups[ref.file].refs.push(ref)
            })
            return groups
        },
        
        async loadDatasets() {
            try {
                const response = await fetch('/api/dataset')
                this.datasets = await response.json()
            } catch (error) {
                console.error('加载数据集失败:', error)
            }
        },
        
        loadHistory() {
            const savedHistory = localStorage.getItem('chatHistory')
            if (savedHistory) {
                this.history = JSON.parse(savedHistory)
                // 加载最近的会话
                if (this.history.length > 0) {
                    this.loadChat(this.history[0])
                }
            }
        },
        
        createNewChat() {
            const newChat = {
                id: Date.now(),
                title: '新会话',
                messages: [],
                context: null
            }
            this.history.unshift(newChat)
            this.loadChat(newChat)
            this.saveHistory()
        },
        
        loadChat(chat) {
            this.currentChatId = chat.id
            this.currentChat = chat
        },
        
        saveHistory() {
            localStorage.setItem('chatHistory', JSON.stringify(this.history))
        },
        
        updateChatTitle(content) {
            const title = content.length > 10 ? content.substring(0, 10) + '...' : content
            this.currentChat.title = title
            this.saveHistory()
        },
        
        async sendMessage(event) {
            if (event.key === 'Enter' && event.shiftKey) {
                return // 允许Shift+Enter换行
            }
            
            if (!this.userInput.trim() || this.loading) return
            
            const userMessage = this.userInput.trim()
            this.userInput = ''
            
            // 添加用户消息
            this.currentChat.messages.push({
                role: 'user',
                content: userMessage
            })
            
            this.loading = true
            
            try {
                const response = await fetch('/api/chat', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        dataset: this.selectedDataset,
                        content: userMessage,
                        context: this.currentChat.context
                    })
                })
                
                const data = await response.json()
                
                // 添加助手回复
                this.currentChat.messages.push({
                    role: 'assistant',
                    content: data.response,
                    references: data.reference
                })
                
                // 更新context
                this.currentChat.context = data.context
                
                // 更新会话标题
                this.updateChatTitle(userMessage)
                
                // 保存到localStorage
                this.saveHistory()
                
                // 滚动到底部
                this.$nextTick(() => {
                    const chatWindow = this.$refs.chatWindow
                    chatWindow.scrollTop = chatWindow.scrollHeight
                })
            } catch (error) {
                console.error('发送消息失败:', error)
                this.currentChat.messages.push({
                    role: 'assistant',
                    content: '发送消息失败，请稍后重试。'
                })
            } finally {
                this.loading = false
            }
        },
        
        // 删除会话
        async deleteChat(chatId, event) {
            // 阻止事件冒泡，避免触发loadChat
            event.stopPropagation()
            
            if (!confirm('确定要删除这个会话吗？')) {
                return
            }
            
            // 从历史记录中删除
            this.history = this.history.filter(chat => chat.id !== chatId)
            
            // 如果删除的是当前会话，切换到第一个会话或创建新会话
            if (chatId === this.currentChatId) {
                if (this.history.length > 0) {
                    this.loadChat(this.history[0])
                } else {
                    this.createNewChat()
                }
            }
            
            // 保存更新后的历史记录
            this.saveHistory()
        }
    }
}).mount('#app') 