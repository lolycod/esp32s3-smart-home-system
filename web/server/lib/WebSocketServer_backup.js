/**
 * WebSocket服务器类
 * 负责管理WebSocket服务器的启动、关闭和连接处理
 */

const WebSocket = require('ws');
const http = require('http');
const config = require('../config/server.config');
const ConnectionManager = require('./ConnectionManager');
const MessageHandler = require('./MessageHandler');

class WebSocketServer {
    constructor() {
        this.server = null;
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
            throw new Error('服务器已经在运行中');
        }

        const targetPort = port || this.port;
        
        try {
            // 尝试启动服务器
            const actualPort = await this.startOnPort(targetPort);
            this.port = actualPort;
            this.isRunning = true;
            
            console.log(`✅ WebSocket服务器已启动`);
            console.log(`📡 监听端口: ${actualPort}`);
            console.log(`🌐 WebSocket地址: ws://localhost:${actualPort}`);
            
            return actualPort;
        } catch (error) {
            console.error('❌ 服务器启动失败:', error.message);
            throw error;
        }
    }

    /**
     * 在指定端口启动服务器，如果端口被占用则尝试其他端口
     * @param {number} port - 目标端口
     * @returns {Promise<number>} 实际使用的端口
     */
    startOnPort(port) {
        return new Promise((resolve, reject) => {
            // 创建HTTP服务器
            this.server = http.createServer();
            
            // 创建WebSocket服务器
            this.wss = new WebSocket.Server({
                server: this.server,
                perMessageDeflate: false,
                // 添加CORS支持
                verifyClient: (info, cb) => {
                    // 允许所有来源（生产环境应该限制具体域名）
                    cb(true);
                }
            });

            // 设置WebSocket事件处理
            this.setupWebSocketHandlers();

            // 尝试监听端口，绑定到所有网络接口
            this.server.listen(port, '0.0.0.0', (error) => {
                if (error) {
                    if (error.code === 'EADDRINUSE') {
                        // 端口被占用，尝试下一个端口
                        if (port < config.portRange.max) {
                            console.log(`⚠️  端口 ${port} 被占用，尝试端口 ${port + 1}`);
                            this.server.close();
                            this.startOnPort(port + 1)
                                .then(resolve)
                                .catch(reject);
                        } else {
                            reject(new Error(`端口范围 ${config.portRange.min}-${config.portRange.max} 内没有可用端口`));
                        }
                    } else {
                        reject(error);
                    }
                } else {
                    console.log(`✅ 服务器监听所有网络接口: 0.0.0.0:${port}`);
                    resolve(port);
                }
            });

            // 处理服务器错误
            this.server.on('error', (error) => {
                console.error('HTTP服务器错误:', error);
                reject(error);
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
            console.error('❌ 处理新连接失败:', error.message);
            // 如果是连接数限制，发送错误消息后关闭连接
            this.sendError(ws, error.message);
            ws.close(1008, error.message);
        }
    }

    /**
     * 处理接收到的消息
     * @param {WebSocket} ws - 发送消息的WebSocket连接
     * @param {Buffer} data - 消息数据
     */
    handleMessage(ws, data) {
        // 使用MessageHandler处理消息
        const result = this.messageHandler.processMessage(ws, data);
        
        if (!result.success) {
            // 发送错误消息给发送者
            this.sendError(ws, result.error);
            console.error('❌ 消息处理失败:', result.error);
        }
        // 成功的情况下，MessageHandler已经处理了转发逻辑
    }

    /**
     * 验证消息格式
     * @param {Object} message - 消息对象
     * @returns {boolean} 是否有效
     */
    validateMessage(message) {
        return message && 
               typeof message === 'object' &&
               message.type &&
               message.timestamp &&
               message.data !== undefined;
    }

    /**
     * 向指定连接发送消息
     * @param {WebSocket} ws - 目标连接
     * @param {Object} message - 消息对象
     */
    sendMessage(ws, message) {
        if (ws.readyState === WebSocket.OPEN) {
            try {
                ws.send(JSON.stringify(message));
            } catch (error) {
                console.error('发送消息失败:', error);
            }
        }
    }

    /**
     * 发送错误消息
     * @param {WebSocket} ws - 目标连接
     * @param {string} errorMessage - 错误信息
     */
    sendError(ws, errorMessage) {
        this.sendMessage(ws, {
            type: 'error',
            timestamp: Date.now(),
            data: errorMessage
        });
    }

    /**
     * 广播消息给所有连接的客户端
     * @param {Object} message - 要广播的消息
     * @param {WebSocket} excludeWs - 要排除的连接（可选）
     */
    broadcast(message, excludeWs = null) {
        const broadcastMessage = {
            ...message,
            timestamp: Date.now() // 更新时间戳
        };

        // 找到要排除的连接ID
        let excludeConnectionId = null;
        if (excludeWs) {
            const connections = this.connectionManager.getConnections();
            const excludeConnection = connections.find(conn => conn.ws === excludeWs);
            if (excludeConnection) {
                excludeConnectionId = excludeConnection.id;
            }
        }

        // 使用ConnectionManager广播消息
        this.connectionManager.broadcastToAll(broadcastMessage, excludeConnectionId);
    }

    /**
     * 设置心跳检测
     * @param {WebSocket} ws - WebSocket连接
     */
    setupHeartbeat(ws) {
        // 设置初始状态
        this.connectionManager.setConnectionAlive(ws);
        
        ws.on('pong', () => {
            this.connectionManager.setConnectionAlive(ws);
        });

        // 定期发送ping
        const heartbeatInterval = setInterval(() => {
            // 检查连接是否还在管理器中
            const connections = this.connectionManager.getConnections();
            const connectionInfo = connections.find(conn => conn.ws === ws);
            
            if (!connectionInfo) {
                clearInterval(heartbeatInterval);
                return;
            }

            if (!connectionInfo.isAlive) {
                console.log('💔 客户端心跳超时，断开连接');
                ws.terminate();
                this.connectionManager.removeConnectionByWs(ws);
                clearInterval(heartbeatInterval);
                return;
            }

            this.connectionManager.setConnectionDead(ws);
            ws.ping();
        }, config.websocket.heartbeatInterval);

        // 连接关闭时清理定时器
        ws.on('close', () => {
            clearInterval(heartbeatInterval);
        });
    }

    /**
     * 停止WebSocket服务器
     * @returns {Promise<void>}
     */
    stop() {
        return new Promise((resolve) => {
            if (!this.isRunning) {
                resolve();
                return;
            }

            console.log('🛑 正在停止WebSocket服务器...');

            // 使用ConnectionManager关闭所有连接
            this.connectionManager.closeAllConnections(1000, '服务器关闭');

            // 关闭WebSocket服务器
            if (this.wss) {
                this.wss.close(() => {
                    console.log('✅ WebSocket服务器已关闭');
                });
            }

            // 关闭HTTP服务器
            if (this.server) {
                this.server.close(() => {
                    console.log('✅ HTTP服务器已关闭');
                    this.isRunning = false;
                    resolve();
                });
            } else {
                this.isRunning = false;
                resolve();
            }
        });
    }

    /**
     * 获取服务器状态
     * @returns {Object} 服务器状态信息
     */
    getStatus() {
        return {
            isRunning: this.isRunning,
            port: this.port,
            connectionCount: this.connectionManager.getConnectionCount(),
            uptime: this.isRunning ? process.uptime() : 0
        };
    }

    /**
     * 获取连接数量
     * @returns {number} 当前连接数
     */
    getConnectionCount() {
        return this.connectionManager.getConnectionCount();
    }

    /**
     * 获取连接统计信息
     * @returns {Object} 详细的连接统计信息
     */
    getConnectionStatistics() {
        return this.connectionManager.getStatistics();
    }

    /**
     * 获取消息统计信息
     * @returns {Object} 消息统计信息
     */
    getMessageStatistics() {
        return this.messageHandler.getMessageStatistics();
    }

    /**
     * 获取消息历史
     * @param {number} limit - 限制数量
     * @param {string} type - 消息类型过滤
     * @returns {Array} 消息历史
     */
    getMessageHistory(limit = 50, type = null) {
        return this.messageHandler.getMessageHistory(limit, type);
    }

    /**
     * 发送系统广播消息
     * @param {string} message - 消息内容
     * @returns {Object} 发送结果
     */
    broadcastSystemMessage(message) {
        return this.messageHandler.broadcastSystemMessage(message);
    }

    /**
     * 清除消息历史
     * @returns {number} 清除的消息数量
     */
    clearMessageHistory() {
        return this.messageHandler.clearMessageHistory();
    }
}

module.exports = WebSocketServer;