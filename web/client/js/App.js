/**
 * 应用主控制器
 * 协调ConnectionManager和UIManager，处理用户操作
 */

class App {
    constructor() {
        this.connectionManager = new ConnectionManager();
        this.uiManager = new UIManager();
        this.init();
    }

    /**
     * 初始化应用
     */
    init() {
        console.log('🚀 应用初始化开始');
        this.setupEventHandlers();
        console.log('✅ 事件处理器设置完成');
        this.setupConnectionCallbacks();
        console.log('✅ 连接回调设置完成');

        // 显示欢迎消息
        this.uiManager.showSystemMessage('智能终端管理系统已启动，请配置服务器连接');
        console.log('✅ 初始化完成');
    }

    /**
     * 设置事件处理器
     */
    setupEventHandlers() {
        this.uiManager.setEventHandlers({
            onConnect: () => this.handleConnect(),
            onDisconnect: () => this.handleDisconnect(),
            onSendMessage: () => this.handleSendMessage(),
            onClearLog: () => this.handleClearLog()
        });

        // 设置消息窗口事件处理器
        this.uiManager.setMessageWindowHandlers({
            onClearSent: () => this.handleClearSentMessages(),
            onClearReceived: () => this.handleClearReceivedMessages()
        });
    }

    /**
     * 设置连接回调
     */
    setupConnectionCallbacks() {
        // 监听连接状态变化
        this.connectionManager.onStatusChange((status) => {
            this.uiManager.updateConnectionStatus(status);
        });

        // 监听消息接收
        this.connectionManager.onMessage((message) => {
            if (message.type === 'message') {
                // 接收到的用户消息显示在接收窗口
                this.uiManager.addReceivedMessage(message);
                // 同时在系统日志中记录
                this.uiManager.addLogEntry(message);
            } else if (message.type === 'system' || message.type === 'error') {
                // 系统消息和错误消息只显示在日志中
                this.uiManager.addLogEntry(message);
            }
        });
    }

    /**
     * 处理连接操作 - 采用约定优于配置，默认使用8090端口
     */
    handleConnect() {
        console.log('🎯 连接按钮被点击');
        // 验证输入 - 只验证服务器地址
        const serverUrl = this.uiManager.getServerUrl();
        console.log(`📋 获取到的服务器地址: ${serverUrl}`);

        if (!serverUrl) {
            this.uiManager.showError('服务器地址不能为空');
            return;
        }

        // 获取连接选项
        const saveConnection = this.uiManager.getSaveConnectionOption();
        const autoReconnect = this.uiManager.getAutoReconnectOption();
        console.log(`⚙️ 连接选项: saveConnection=${saveConnection}, autoReconnect=${autoReconnect}`);

        // 获取端口配置
        const serverPort = this.uiManager.getServerPort();

        // 显示连接尝试消息 - 使用配置端口
        this.uiManager.showSystemMessage(`正在连接到 ${serverUrl}:${serverPort}...`);

        // 尝试连接 - 使用配置端口
        console.log(`🚀 开始连接: ${serverUrl}:${serverPort}`);
        this.connectionManager.connect(serverUrl, serverPort, saveConnection, autoReconnect);
    }

    /**
     * 处理断开连接操作
     */
    handleDisconnect() {
        // 禁用自动重连，因为用户主动断开
        this.connectionManager.disconnect(true);
        this.uiManager.showSystemMessage('已主动断开连接');
    }

    /**
     * 处理发送消息操作
     */
    handleSendMessage() {
        const message = this.uiManager.getMessageInput();
        
        if (!message) {
            this.uiManager.showError('消息内容不能为空');
            return;
        }

        if (!this.connectionManager.isConnected()) {
            this.uiManager.showError('未连接到服务器，无法发送消息');
            return;
        }

        // 发送消息
        const success = this.connectionManager.send(message);
        if (success) {
            // 创建发送消息对象
            const sentMessage = {
                type: 'message',
                timestamp: Date.now(),
                data: message
            };

            // 显示在发送消息窗口
            this.uiManager.addSentMessage(sentMessage);

            // 同时在系统日志中记录
            this.uiManager.addLogEntry({
                type: 'message',
                timestamp: Date.now(),
                data: `发送: ${message}`
            });

            // 清空输入框
            this.uiManager.clearMessageInput();
        } else {
            this.uiManager.showError('消息发送失败');
        }
    }

    /**
     * 处理清除日志操作
     */
    handleClearLog() {
        this.uiManager.clearLogs();
        this.uiManager.showSystemMessage('日志已清除');
    }

    /**
     * 处理清除发送消息
     */
    handleClearSentMessages() {
        this.uiManager.clearSentMessages();
    }

    /**
     * 处理清除接收消息
     */
    handleClearReceivedMessages() {
        this.uiManager.clearReceivedMessages();
    }
}

// 当页面加载完成后启动应用
document.addEventListener('DOMContentLoaded', () => {
    console.log('📄 DOMContentLoaded 事件触发');
    try {
        const app = new App();
        console.log('✅ App 实例创建成功');
    } catch (error) {
        console.error('💥 App 创建失败:', error);
        console.error('错误栈:', error.stack);
    }
});