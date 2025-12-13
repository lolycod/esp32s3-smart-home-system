/**
 * 用户界面管理器
 * 负责管理用户界面的更新和交互
 */

class UIManager {
    constructor() {
        console.log('🎨 UIManager 构造函数被调用');
        this.elements = {};
        try {
            this.initializeElements();
            console.log('✅ UIManager 元素初始化完成');
        } catch (error) {
            console.error('💥 UIManager 元素初始化失败:', error);
            throw error;
        }
    }

    /**
     * 初始化DOM元素引用
     */
    initializeElements() {
        console.log('🔍 开始初始化DOM元素引用');

        const requiredElements = [
            'serverUrl', 'connectBtn', 'disconnectBtn', 'connectionStatus',
            'messageInput', 'sendBtn', 'logContainer', 'clearLogBtn',
            'autoReconnect', 'saveConnection', 'sentMessagesContainer',
            'receivedMessagesContainer', 'clearSentBtn', 'clearReceivedBtn'
        ];

        this.elements = {};

        requiredElements.forEach(id => {
            const element = document.getElementById(id);
            if (element) {
                this.elements[id] = element;
                console.log(`✅ ${id} 元素找到`);
            } else {
                console.error(`❌ ${id} 元素未找到`);
            }
        });

        // 检查关键元素是否存在
        const criticalElements = ['serverUrl', 'connectBtn', 'disconnectBtn'];
        const missingCritical = criticalElements.filter(id => !this.elements[id]);

        if (missingCritical.length > 0) {
            throw new Error(`关键元素缺失: ${missingCritical.join(', ')}`);
        }

        console.log('✅ DOM元素初始化完成');
    }

    /**
     * 更新连接状态显示
     * @param {string} status - 连接状态
     */
    updateConnectionStatus(status) {
        const statusElement = this.elements.connectionStatus;
        const connectBtn = this.elements.connectBtn;
        const disconnectBtn = this.elements.disconnectBtn;
        const sendBtn = this.elements.sendBtn;

        // 移除所有状态类
        statusElement.className = 'status-indicator';
        
        switch (status) {
            case 'disconnected':
                statusElement.classList.add('disconnected');
                statusElement.textContent = '未连接';
                connectBtn.disabled = false;
                disconnectBtn.disabled = true;
                sendBtn.disabled = true;
                break;
            case 'connecting':
                statusElement.classList.add('connecting');
                statusElement.textContent = '连接中...';
                connectBtn.disabled = true;
                disconnectBtn.disabled = false;
                sendBtn.disabled = true;
                break;
            case 'connected':
                statusElement.classList.add('connected');
                statusElement.textContent = '已连接';
                connectBtn.disabled = true;
                disconnectBtn.disabled = false;
                sendBtn.disabled = false;
                break;
            case 'error':
                statusElement.classList.add('error');
                statusElement.textContent = '连接错误';
                connectBtn.disabled = false;
                disconnectBtn.disabled = true;
                sendBtn.disabled = true;
                break;
        }
    }

    /**
     * 添加日志条目
     * @param {Object} message - 消息对象
     */
    addLogEntry(message) {
        const logContainer = this.elements.logContainer;
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${message.type}`;

        const timestamp = new Date(message.timestamp).toLocaleTimeString();
        const timestampSpan = document.createElement('span');
        timestampSpan.className = 'log-timestamp';
        timestampSpan.textContent = `[${timestamp}]`;

        const messageSpan = document.createElement('span');
        messageSpan.textContent = this._formatForDisplay(message.data);

        logEntry.appendChild(timestampSpan);
        logEntry.appendChild(messageSpan);
        logContainer.appendChild(logEntry);

        // 自动滚动到底部
        logContainer.scrollTop = logContainer.scrollHeight;

        // 限制日志条目数量（保持最新的100条）
        const maxEntries = 100;
        while (logContainer.children.length > maxEntries) {
            logContainer.removeChild(logContainer.firstChild);
        }
    }

    /**
     * 清除所有日志
     */
    clearLogs() {
        this.elements.logContainer.innerHTML = '';
    }

    /**
     * 显示错误信息
     * @param {string} error - 错误信息
     */
    showError(error) {
        this.addLogEntry({
            type: 'error',
            timestamp: Date.now(),
            data: error
        });
    }

    /**
     * 获取服务器URL
     * @returns {string} 服务器URL
     */
    getServerUrl() {
        return this.elements.serverUrl.value.trim() || 'localhost';
    }

    /**
     * 获取服务器端口 - 从UI读取端口配置，默认80
     * @returns {number} 服务器端口
     */
    getServerPort() {
        // 从高级选项中读取端口配置
        const serverPortElement = document.getElementById('serverPort');
        if (serverPortElement && serverPortElement.value) {
            return parseInt(serverPortElement.value, 10);
        }
        // 默认返回80端口（通过FRP转发到本地8080）
        return 80;
    }

    /**
     * 获取消息输入内容
     * @returns {string} 消息内容
     */
    getMessageInput() {
        return this.elements.messageInput.value.trim();
    }

    /**
     * 清空消息输入框
     */
    clearMessageInput() {
        this.elements.messageInput.value = '';
    }

    /**
     * 验证输入 - 采用约定优于配置，只验证服务器地址
     * @returns {Object} 验证结果
     */
    validateInput() {
        const serverUrl = this.getServerUrl();

        const errors = [];

        // 验证服务器地址 - 端口已固定为8090，无需验证
        if (!serverUrl) {
            errors.push('服务器地址不能为空');
        }

        return {
            valid: errors.length === 0,
            errors: errors
        };
    }

    /**
     * 设置按钮事件监听器
     * @param {Object} handlers - 事件处理器对象
     */
    setEventHandlers(handlers) {
        console.log('🔧 UIManager.setEventHandlers 被调用');
        if (handlers.onConnect) {
            console.log('✅ 连接按钮事件处理器已设置');
            this.elements.connectBtn.addEventListener('click', handlers.onConnect);
        } else {
            console.log('⚠️ 未提供连接按钮事件处理器');
        }

        if (handlers.onDisconnect) {
            this.elements.disconnectBtn.addEventListener('click', handlers.onDisconnect);
        }

        if (handlers.onSendMessage) {
            this.elements.sendBtn.addEventListener('click', handlers.onSendMessage);
            
            // 支持回车键发送消息
            this.elements.messageInput.addEventListener('keypress', (event) => {
                if (event.key === 'Enter' && !event.shiftKey) {
                    event.preventDefault();
                    handlers.onSendMessage();
                }
            });
        }

        if (handlers.onClearLog) {
            this.elements.clearLogBtn.addEventListener('click', handlers.onClearLog);
        }
    }

    /**
     * 显示系统消息
     * @param {string} message - 系统消息
     */
    showSystemMessage(message) {
        this.addLogEntry({
            type: 'system',
            timestamp: Date.now(),
            data: message
        });
    }

    /**
     * 获取自动重连选项
     * @returns {boolean} 是否启用自动重连
     */
    getAutoReconnectOption() {
        return this.elements.autoReconnect ? this.elements.autoReconnect.checked : true;
    }

    /**
     * 获取保存连接选项
     * @returns {boolean} 是否保存连接信息
     */
    getSaveConnectionOption() {
        return this.elements.saveConnection ? this.elements.saveConnection.checked : true;
    }

    /**
     * 添加发送消息到发送窗口
     * @param {Object} message - 消息对象
     */
    addSentMessage(message) {
        const container = this.elements.sentMessagesContainer;
        const messageItem = this.createMessageItem(message, 'sent');
        container.appendChild(messageItem);
        this.scrollToBottom(container);
        this.limitMessageCount(container, 50); // 限制50条消息
    }

    /**
     * 添加接收消息到接收窗口
     * @param {Object} message - 消息对象
     */
    addReceivedMessage(message) {
        const container = this.elements.receivedMessagesContainer;
        const messageItem = this.createMessageItem(message, 'received');
        container.appendChild(messageItem);
        this.scrollToBottom(container);
        this.limitMessageCount(container, 50); // 限制50条消息
    }

    /**
     * 创建消息项元素
     * @param {Object} message - 消息对象
     * @param {string} type - 消息类型 (sent/received)
     * @returns {HTMLElement} 消息元素
     */
    createMessageItem(message, type) {
        const messageItem = document.createElement('div');
        messageItem.className = `message-item ${type}`;

        const timestamp = new Date(message.timestamp).toLocaleTimeString();

        // 时间戳
        const timeDiv = document.createElement('div');
        timeDiv.className = 'message-time';
        timeDiv.textContent = timestamp;

        // 消息内容
        const contentDiv = document.createElement('div');
        contentDiv.className = 'message-content';
        contentDiv.textContent = this._formatForDisplay(message.data);

        messageItem.appendChild(timeDiv);
        messageItem.appendChild(contentDiv);

        // 如果是接收的消息且包含发送者信息
        if (type === 'received' && message.sender) {
            const senderDiv = document.createElement('div');
            senderDiv.className = 'message-sender';
            senderDiv.textContent = `来自: ${message.sender.id}`;
            messageItem.appendChild(senderDiv);
        }

        return messageItem;
    }

    /**
     * 将底层数据安全转换为可读文本，避免在前端暴露原始JSON
     * @param {*} value
     * @returns {string}
     */
    _formatForDisplay(value) {
        if (value == null) return '';
        if (typeof value === 'object') {
            return '[控制消息]';
        }
        const text = String(value).trim();
        if ((text.startsWith('{') && text.endsWith('}')) || (text.startsWith('[') && text.endsWith(']'))) {
            return '[控制消息]';
        }
        return text;
    }

    /**
     * 滚动到容器底部
     * @param {HTMLElement} container - 容器元素
     */
    scrollToBottom(container) {
        container.scrollTop = container.scrollHeight;
    }

    /**
     * 限制消息数量
     * @param {HTMLElement} container - 容器元素
     * @param {number} maxCount - 最大消息数
     */
    limitMessageCount(container, maxCount) {
        while (container.children.length > maxCount) {
            container.removeChild(container.firstChild);
        }
    }

    /**
     * 清除发送消息
     */
    clearSentMessages() {
        this.elements.sentMessagesContainer.innerHTML = '';
        this.showSystemMessage('已清除发送消息记录');
    }

    /**
     * 清除接收消息
     */
    clearReceivedMessages() {
        this.elements.receivedMessagesContainer.innerHTML = '';
        this.showSystemMessage('已清除接收消息记录');
    }

    /**
     * 设置消息窗口事件监听器
     * @param {Object} handlers - 事件处理器对象
     */
    setMessageWindowHandlers(handlers) {
        if (handlers.onClearSent) {
            this.elements.clearSentBtn.addEventListener('click', handlers.onClearSent);
        }

        if (handlers.onClearReceived) {
            this.elements.clearReceivedBtn.addEventListener('click', handlers.onClearReceived);
        }
    }
}
