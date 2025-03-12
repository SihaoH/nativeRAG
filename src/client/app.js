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
            selectedDataset: '',
            userInput: '',
            loading: false,
            tooltipVisible: false,
            tooltipContent: [],
            isDragging: false,
            startX: 0,
            startY: 0,
            startWidth: 0,
            startHeight: 0
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
            const inputArea = document.querySelector('.input-area')
            const chatWindow = this.$refs.chatWindow
            
            if (layout.inputHeight) {
                inputArea.style.height = `${layout.inputHeight}%`
                chatWindow.style.flex = `${100 - layout.inputHeight}`
            }
        }
    },
    
    async created() {
        // 加载数据集列表
        await this.loadDatasets()
        // 加载历史记录
        this.loadHistory()
        // 如果没有当前会话，创建新会话
        if (!this.currentChatId) {
            this.createNewChat()
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
            const chatWindow = this.$refs.chatWindow
            this.startHeight = chatWindow.offsetHeight
        },
        
        onDrag(e) {
            if (!this.isDragging) return
            
            if (this.isDragging === 'sidebar') {
                const width = this.startWidth + (e.clientX - this.startX)
                if (width >= 150 && width <= 500) {
                    this.$refs.sidebar.style.width = `${width}px`
                }
            } else if (this.isDragging === 'chat') {
                const container = this.$refs.chatWindow.parentElement
                const totalHeight = container.offsetHeight
                const mouseY = e.clientY
                const containerTop = container.getBoundingClientRect().top
                const relativeY = mouseY - containerTop
                
                // 计算输入区域的新高度比例
                let inputHeightPercent = ((totalHeight - relativeY) / totalHeight) * 100
                
                // 限制高度范围
                inputHeightPercent = Math.max(20, Math.min(50, inputHeightPercent))
                
                // 更新高度
                const inputArea = document.querySelector('.input-area')
                inputArea.style.height = `${inputHeightPercent}%`
                
                // 保存布局状态
                localStorage.setItem('chatLayout', JSON.stringify({
                    inputHeight: inputHeightPercent
                }))
            }
        },
        
        stopDrag() {
            this.isDragging = false
        },
        
        // 引用提示相关方法
        showTooltip(event, refs) {
            const tooltip = this.$refs.tooltip
            this.tooltipContent = refs.map(ref => ref.text)
            tooltip.style.display = 'block'
            
            const rect = event.target.getBoundingClientRect()
            const tooltipWidth = 400 // 最大宽度
            
            // 计算最佳显示位置
            let left = rect.left
            let top = rect.bottom + 5
            
            // 确保提示框不会超出屏幕右边界
            if (left + tooltipWidth > window.innerWidth) {
                left = window.innerWidth - tooltipWidth - 10
            }
            
            // 确保提示框不会超出屏幕底部
            const tooltipHeight = tooltip.offsetHeight
            if (top + tooltipHeight > window.innerHeight) {
                top = rect.top - tooltipHeight - 5
            }
            
            tooltip.style.left = `${left}px`
            tooltip.style.top = `${top}px`
            
            this.tooltipVisible = true
        },
        
        hideTooltip() {
            this.tooltipVisible = false
            this.$refs.tooltip.style.display = 'none'
        },
        
        // 引用分组方法
        groupedReferences(references) {
            if (!references) return {}
            const groups = {}
            references.forEach(ref => {
                if (!groups[ref.file]) {
                    groups[ref.file] = []
                }
                groups[ref.file].push(ref)
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
        }
    }
}).mount('#app') 