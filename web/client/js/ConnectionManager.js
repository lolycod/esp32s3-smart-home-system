/**
 * WebSocket连接管理器
 * 负责管理WebSocket连接状态和通信
 */

class ConnectionManager {
    constructor() {
        console.log('🔗 ConnectionManager 构造函数被调用');
        this.ws = null;
        this.url = '';
        this.status = 'disconnected';
        this.messageCallbacks = [];
        this.statusCallbacks = [];
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectInterval = 3000;
        this.autoReconnect = true;
        this.reconnectTimer = null;
        this.lastServerUrl = '';
        this.lastServerPort = '';

        // 加载连接历史
        try {
            this.loadConnectionHistory();
            console.log('✅ ConnectionManager 初始化完成');
        } catch (error) {
            console.error('💥 ConnectionManager 初始化失败:', error);
            throw error;
        }
    }

    /**
     * 连接到WebSocket服务器 - 采用约定优于配置设计哲学，默认使用80端口
     * @param {string} serverUrl - 服务器地址
     * @param {number} port - 端口号（可选，默认为80）
     * @param {boolean} saveConnection - 是否保存连接信息
     */
    connect(serverUrl, port = 80, saveConnection = true, autoReconnect = true) {
        // 智能URL构建 - 支持域名和IP地址
        let wsUrl;

        // 检查是否已经包含协议
        if (serverUrl.startsWith('ws://') || serverUrl.startsWith('wss://')) {
            // 如果已经包含协议，直接使用
            wsUrl = `${serverUrl}:${port}`;
        } else {
            // 构建WebSocket URL
            wsUrl = `ws://${serverUrl}:${port}`;
        }

        this.url = wsUrl;
        this.autoReconnect = autoReconnect;
        this.lastServerUrl = serverUrl;
        this.lastServerPort = port;

        console.log(`🔗 正在连接到WebSocket服务器: ${this.url}`);
        console.log(`📡 连接参数: serverUrl=${serverUrl}, port=${port}`);
        console.log(`🔧 URL构建结果: ${wsUrl}`);

        this.setStatus('connecting');

        // 保存连接历史
        if (saveConnection) {
            this.saveConnectionHistory(serverUrl, port);
        }

        // 清除之前的重连定时器
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }

        try {
            console.log(`🔄 正在创建WebSocket对象...`);
            this.ws = new WebSocket(this.url);
            console.log(`✅ WebSocket对象创建成功`);
            this.setupEventHandlers();
        } catch (error) {
            console.error(`❌ WebSocket创建失败:`, error);
            console.error(`💥 错误详情:`, error.message);
            this.setStatus('error');
            this.notifyMessage({
                type: 'error',
                timestamp: Date.now(),
                data: `连接失败: ${error.message}`
            });

            // 尝试自动重连
            if (this.autoReconnect) {
                this.scheduleReconnect();
            }
        }
    }

    /**
     * 断开WebSocket连接
     * @param {boolean} disableAutoReconnect - 是否禁用自动重连
     */
    disconnect(disableAutoReconnect = true) {
        if (disableAutoReconnect) {
            this.autoReconnect = false;
        }

        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }

        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
        this.setStatus('disconnected');
    }

    /**
     * 发送消息
     * @param {string} message - 要发送的消息
     */
    send(message) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            const messageObj = {
                type: 'message',
                timestamp: Date.now(),
                data: message
            };
            this.ws.send(JSON.stringify(messageObj));
            return true;
        }
        return false;
    }

    /**
     * 设置消息回调
     * @param {Function} callback - 消息回调函数
     */
    onMessage(callback) {
        this.messageCallbacks.push(callback);
    }

    /**
     * 设置状态变化回调
     * @param {Function} callback - 状态变化回调函数
     */
    onStatusChange(callback) {
        this.statusCallbacks.push(callback);
    }

    /**
     * 设置WebSocket事件处理器
     */
    setupEventHandlers() {
        this.ws.onopen = () => {
            console.log(`🎉 WebSocket连接成功: ${this.url}`);
            this.setStatus('connected');
            this.reconnectAttempts = 0;
            this.notifyMessage({
                type: 'system',
                timestamp: Date.now(),
                data: `已连接到服务器: ${this.url}`
            });
        };

        this.ws.onmessage = (event) => {
            try {
                const message = JSON.parse(event.data);
                this.notifyMessage(message);
            } catch (error) {
                this.notifyMessage({
                    type: 'error',
                    timestamp: Date.now(),
                    data: `消息解析失败: ${error.message}`
                });
            }
        };

        this.ws.onclose = (event) => {
            console.log(`🔌 WebSocket连接关闭: 代码=${event.code}, 原因=${event.reason}`);
            this.setStatus('disconnected');
            this.notifyMessage({
                type: 'system',
                timestamp: Date.now(),
                data: `连接已断开 (代码: ${event.code})`
            });

            // 如果不是用户主动断开，尝试自动重连
            if (this.autoReconnect && event.code !== 1000) {
                this.scheduleReconnect();
            }
        };

        this.ws.onerror = (error) => {
            console.error(`❌ WebSocket错误:`, error);
            this.setStatus('error');
            this.notifyMessage({
                type: 'error',
                timestamp: Date.now(),
                data: `连接错误: ${error.message || '未知错误'}`
            });
        };
    }

    /**
     * 设置连接状态
     * @param {string} status - 新状态
     */
    setStatus(status) {
        this.status = status;
        this.statusCallbacks.forEach(callback => callback(status));
    }

    /**
     * 通知消息回调
     * @param {Object} message - 消息对象
     */
    notifyMessage(message) {
        this.messageCallbacks.forEach(callback => callback(message));
    }

    /**
     * 获取当前连接状态
     * @returns {string} 当前状态
     */
    getStatus() {
        return this.status;
    }

    /**
     * 检查是否已连接
     * @returns {boolean} 是否已连接
     */
    isConnected() {
        return this.status === 'connected' && this.ws && this.ws.readyState === WebSocket.OPEN;
    }

    /**
     * 计划重连
     */
    scheduleReconnect() {
        if (!this.autoReconnect || this.reconnectAttempts >= this.maxReconnectAttempts) {
            return;
        }

        this.reconnectAttempts++;
        const delay = this.reconnectInterval * this.reconnectAttempts; // 递增延迟

        this.notifyMessage({
            type: 'system',
            timestamp: Date.now(),
            data: `连接断开，${delay/1000}秒后尝试重连 (${this.reconnectAttempts}/${this.maxReconnectAttempts})`
        });

        this.reconnectTimer = setTimeout(() => {
            if (this.autoReconnect) {
                this.connect(this.lastServerUrl, this.lastServerPort, false, true);
            }
        }, delay);
    }

    /**
     * 保存连接历史 - 采用约定优于配置，只保存服务器地址
     * @param {string} serverUrl - 服务器地址
     * @param {number} port - 端口号（默认8090）
     */
    saveConnectionHistory(serverUrl, port = 8090) {
        try {
            // 获取现有历史
            let urlHistory = JSON.parse(localStorage.getItem('serverUrlHistory') || '[]');

            // 添加新记录（避免重复）
            if (!urlHistory.includes(serverUrl)) {
                urlHistory.unshift(serverUrl);
                urlHistory = urlHistory.slice(0, 10); // 保留最近10个
            }

            // 保存到localStorage - 不再保存端口历史
            localStorage.setItem('serverUrlHistory', JSON.stringify(urlHistory));
            localStorage.setItem('lastConnection', JSON.stringify({ serverUrl, port }));

        } catch (error) {
            console.warn('保存连接历史失败:', error);
        }
    }

    /**
     * 加载连接历史 - 采用约定优于配置，只加载服务器地址
     */
    loadConnectionHistory() {
        try {
            // 加载历史记录到datalist - 只加载URL历史
            const urlHistory = JSON.parse(localStorage.getItem('serverUrlHistory') || '[]');

            const urlDataList = document.getElementById('serverUrlHistory');

            if (urlDataList) {
                urlDataList.innerHTML = urlHistory.map(url => `<option value="${url}">`).join('');
            }

            // 加载上次连接信息 - 默认端口为8090
            const lastConnection = JSON.parse(localStorage.getItem('lastConnection') || 'null');
            if (lastConnection) {
                const urlInput = document.getElementById('serverUrl');
                if (urlInput) urlInput.value = lastConnection.serverUrl;
                // 不再设置端口输入框，采用默认8090端口
            }

        } catch (error) {
            console.warn('加载连接历史失败:', error);
        }
    }

    /**
     * 获取连接历史 - 采用约定优于配置，只返回服务器地址历史
     */
    getConnectionHistory() {
        try {
            return {
                urlHistory: JSON.parse(localStorage.getItem('serverUrlHistory') || '[]'),
                portHistory: [], // 不再维护端口历史
                lastConnection: JSON.parse(localStorage.getItem('lastConnection') || 'null')
            };
        } catch (error) {
            return { urlHistory: [], portHistory: [], lastConnection: null };
        }
    }
}