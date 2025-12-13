/**
 * WebSocket服务器类
 * 负责管理独立的WebSocket服务器，不依赖HTTP服务器
 */

const WebSocket = require('ws');
const config = require('../config/server.config');
const ConnectionManager = require('./ConnectionManager');
const MessageHandler = require('./MessageHandler');

class WebSocketServer {
    constructor() {
        this.wss = null;
        this.port = config.defaultPort;
        this.isRunning = false;
        this.connectionManager = new ConnectionManager();
        this.messageHandler = new MessageHandler(this.connectionManager);
    }

    /**
     * 启动WebSocket服务器
     * @param {number} port - 端口号（可选）
     * @returns {Promise<number>} 返回实际使用的端口号
     */
    async start(port = null) {
        if (this.isRunning) {
            throw new Error('WebSocket服务器已经在运行中');
        }

        const targetPort = port || this.port;

        try {
            // 启动独立的WebSocket服务器
            const actualPort = await this.startOnPort(targetPort);
            this.port = actualPort;
            this.isRunning = true;

            console.log(`✅ WebSocket服务器已启动`);
            console.log(`📡 监听端口: ${actualPort}`);
            console.log(`🌐 WebSocket地址: ws://localhost:${actualPort}`);

            return actualPort;
        } catch (error) {
            console.error('❌ WebSocket服务器启动失败:', error.message);
            throw error;
        }
    }

    /**
     * 在指定端口启动独立的WebSocket服务器
     * @param {number} port - 目标端口
     * @returns {Promise<number>} 实际使用的端口
     */
    startOnPort(port) {
        return new Promise((resolve, reject) => {
            // 创建独立的WebSocket服务器，不依赖HTTP服务器
            this.wss = new WebSocket.Server({
                port: port,
                host: '0.0.0.0',
                perMessageDeflate: false,
                // 添加CORS支持
                verifyClient: (info, cb) => {
                    // 允许所有来源（生产环境应该限制具体域名）
                    console.log(`🔍 WebSocket连接验证 - Origin: ${info.origin}, Secure: ${info.secure}`);
                    cb(true);
                }
            });

            // 设置WebSocket事件处理
            this.setupWebSocketHandlers();

            // WebSocket服务器启动成功
            console.log(`✅ WebSocket服务器监听所有网络接口: 0.0.0.0:${port}`);
            resolve(port);

            // 处理WebSocket服务器错误
            this.wss.on('error', (error) => {
                console.error('❌ WebSocket服务器错误:', error);
                if (error.code === 'EADDRINUSE') {
                    reject(new Error(`端口 ${port} 被占用，无法启动WebSocket服务器`));
                } else {
                    reject(error);
                }
            });
        });
    }

    /**
     * 设置WebSocket事件处理器
     */
    setupWebSocketHandlers() {
        this.wss.on('connection', (ws, request) => {
            this.handleConnection(ws, request);
        });

        this.wss.on('error', (error) => {
            console.error('WebSocket服务器错误:', error);
        });
    }

    /**
     * 处理新的WebSocket连接
     * @param {WebSocket} ws - WebSocket连接对象
     * @param {http.IncomingMessage} request - HTTP请求对象
     */
    handleConnection(ws, request) {
        try {
            // 获取客户端信息
            const clientIP = request.headers['x-forwarded-for'] ||
                           request.connection.remoteAddress ||
                           request.socket.remoteAddress;

            const origin = request.headers.origin || 'unknown';
            const userAgent = request.headers['user-agent'] || 'unknown';

            console.log(`🔌 新的WebSocket连接请求:`);
            console.log(`   - 客户端IP: ${clientIP}`);
            console.log(`   - Origin: ${origin}`);
            console.log(`   - User-Agent: ${userAgent}`);
            console.log(`   - 请求URL: ${request.url}`);

            // 使用ConnectionManager添加连接
            const connectionId = this.connectionManager.addConnection(ws, request);

            // 发送欢迎消息
            this.sendMessage(ws, {
                type: 'system',
                timestamp: Date.now(),
                data: '欢迎连接到智能终端管理系统'
            });

            // 设置消息处理
            ws.on('message', (data) => {
                this.handleMessage(ws, data);
                // 更新连接活动时间
                this.connectionManager.updateActivityByWs(ws);
            });

            // 设置连接关闭处理
            ws.on('close', (code, reason) => {
                this.connectionManager.removeConnectionByWs(ws);
            });

            // 设置错误处理
            ws.on('error', (error) => {
                console.error(`❌ WebSocket连接错误:`, error);
                this.connectionManager.removeConnectionByWs(ws);
            });

            // 设置心跳检测
            this.setupHeartbeat(ws);

        } catch (error) {
            console.error('❌ 处理WebSocket连接时出错:', error);
            ws.close(1011, '服务器内部错误');
        }
    }

    /**
     * 处理消息
     * @param {WebSocket} ws - WebSocket连接
     * @param {Buffer} data - 接收到的数据
     */
    handleMessage(ws, data) {
        try {
            const message = JSON.parse(data.toString());
            const result = this.messageHandler.processMessage(ws, data);
            if (!result.success) {
                console.error("❌ 消息处理失败:", result.error);
                this.sendMessage(ws, {
                    type: "error",
                    timestamp: Date.now(),
                    data: result.error
                });
            } else {
                console.log("✅ 消息处理成功，转发完成");
            }
        } catch (error) {
            console.error('❌ 消息解析失败:', error);
            this.sendMessage(ws, {
                type: 'error',
                timestamp: Date.now(),
                data: '消息格式错误'
            });
        }
    }

    /**
     * 发送消息给指定连接
     * @param {WebSocket} ws - WebSocket连接
     * @param {Object} message - 消息对象
     */
    sendMessage(ws, message) {
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify(message));
        }
    }

    /**
     * 广播消息给所有连接
     * @param {Object} message - 消息对象
     * @param {string} senderId - 发送者ID（可选，用于排除发送者）
     */
    broadcastMessage(message, senderId = null) {
        const connections = this.connectionManager.getAllConnections();
        let successCount = 0;

        console.log(`📡 开始广播消息给 ${connections.length} 个连接...`);

        connections.forEach(connection => {
            if (connection.ws.readyState === WebSocket.OPEN) {
                if (senderId && connection.id === senderId) {
                    return; // 跳过发送者
                }
                try {
                    connection.ws.send(JSON.stringify(message));
                    successCount++;
                } catch (error) {
                    console.error(`❌ 广播消息失败: ${connection.id}`, error);
                }
            }
        });

        console.log(`📡 广播完成: ${successCount}/${connections.length} 连接成功接收消息`);
    }

    /**
     * 设置心跳检测
     * @param {WebSocket} ws - WebSocket连接
     */
    setupHeartbeat(ws) {
        const heartbeatInterval = config.websocket.heartbeatInterval;

        const heartbeat = () => {
            if (ws.readyState === WebSocket.OPEN) {
                ws.ping();
            }
        };

        const heartbeatTimer = setInterval(heartbeat, heartbeatInterval);

        ws.on('pong', () => {
            // 客户端响应心跳，更新活动时间
            this.connectionManager.updateActivityByWs(ws);
        });

        ws.on('close', () => {
            clearInterval(heartbeatTimer);
        });
    }

    /**
     * 获取当前连接数
     * @returns {number} 当前连接数
     */
    getConnectionCount() {
        return this.connectionManager.getConnectionCount();
    }

    /**
     * 停止服务器
     */
    stop() {
        if (this.wss) {
            console.log('🔴 正在停止WebSocket服务器...');
            this.wss.close();
            this.isRunning = false;
            console.log('✅ WebSocket服务器已停止');
        }
    }

    /**
     * 获取服务器状态信息
     * @returns {Object} 状态信息
     */
    getStatus() {
        return {
            isRunning: this.isRunning,
            port: this.port,
            connectionCount: this.getConnectionCount(),
            maxConnections: config.websocket.maxConnections,
            uptime: process.uptime()
        };
    }
}

module.exports = WebSocketServer;