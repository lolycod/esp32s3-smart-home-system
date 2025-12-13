/**
 * 连接管理器
 * 负责管理所有活跃的WebSocket连接
 */

const WebSocket = require('ws');
const config = require('../config/server.config');

class ConnectionManager {
    constructor() {
        this.connections = new Map(); // 使用Map存储连接信息
        this.connectionCounter = 0;
        this.maxConnections = config.websocket.maxConnections;
    }

    /**
     * 添加新连接
     * @param {WebSocket} ws - WebSocket连接对象
     * @param {http.IncomingMessage} request - HTTP请求对象
     * @returns {string} 连接ID
     */
    addConnection(ws, request) {
        // 检查连接数限制
        if (this.connections.size >= this.maxConnections) {
            throw new Error(`连接数已达到最大限制: ${this.maxConnections}`);
        }

        // 生成唯一连接ID
        const connectionId = this.generateConnectionId();
        
        // 获取客户端信息
        const clientInfo = this.extractClientInfo(request);
        
        // 创建连接信息对象
        const connectionInfo = {
            id: connectionId,
            ws: ws,
            clientInfo: clientInfo,
            connectedAt: new Date(),
            lastActivity: new Date(),
            isAlive: true,
            messageCount: 0
        };

        // 存储连接
        this.connections.set(connectionId, connectionInfo);

        console.log(`🔗 新连接已添加: ${connectionId} (${clientInfo.ip})`);
        console.log(`📊 当前连接数: ${this.connections.size}/${this.maxConnections}`);

        return connectionId;
    }

    /**
     * 移除连接
     * @param {string} connectionId - 连接ID
     * @returns {boolean} 是否成功移除
     */
    removeConnection(connectionId) {
        const connectionInfo = this.connections.get(connectionId);
        
        if (connectionInfo) {
            this.connections.delete(connectionId);
            console.log(`🔌 连接已移除: ${connectionId} (${connectionInfo.clientInfo.ip})`);
            console.log(`📊 当前连接数: ${this.connections.size}/${this.maxConnections}`);
            return true;
        }
        
        return false;
    }

    /**
     * 通过WebSocket对象移除连接
     * @param {WebSocket} ws - WebSocket连接对象
     * @returns {boolean} 是否成功移除
     */
    removeConnectionByWs(ws) {
        for (const [connectionId, connectionInfo] of this.connections) {
            if (connectionInfo.ws === ws) {
                return this.removeConnection(connectionId);
            }
        }
        return false;
    }

    /**
     * 获取连接信息
     * @param {string} connectionId - 连接ID
     * @returns {Object|null} 连接信息
     */
    getConnection(connectionId) {
        return this.connections.get(connectionId) || null;
    }

    /**
     * 获取所有连接
     * @returns {Array} 连接信息数组
     */
    getConnections() {
        return Array.from(this.connections.values());
    }

    /**
     * 获取活跃连接数
     * @returns {number} 活跃连接数
     */
    getConnectionCount() {
        return this.connections.size;
    }

    /**
     * 向所有连接广播消息
     * @param {Object} message - 要广播的消息
     * @param {string} excludeConnectionId - 要排除的连接ID（可选）
     * @returns {number} 成功发送的连接数
     */
    broadcastToAll(message, excludeConnectionId = null) {
        let successCount = 0;
        const totalConnections = this.connections.size;

        console.log(`📡 开始广播消息给 ${totalConnections} 个连接...`);

        for (const [connectionId, connectionInfo] of this.connections) {
            // 跳过排除的连接
            if (connectionId === excludeConnectionId) {
                continue;
            }

            // 检查连接状态
            if (connectionInfo.ws.readyState === WebSocket.OPEN) {
                try {
                    connectionInfo.ws.send(JSON.stringify(message));
                    connectionInfo.lastActivity = new Date();
                    successCount++;
                } catch (error) {
                    console.error(`❌ 向连接 ${connectionId} 发送消息失败:`, error.message);
                    // 标记连接为不活跃，稍后清理
                    connectionInfo.isAlive = false;
                }
            } else {
                console.log(`⚠️  连接 ${connectionId} 状态异常: ${connectionInfo.ws.readyState}`);
                connectionInfo.isAlive = false;
            }
        }

        console.log(`📡 广播完成: ${successCount}/${totalConnections} 连接成功接收消息`);
        
        // 清理不活跃的连接
        this.cleanupInactiveConnections();

        return successCount;
    }

    /**
     * 向指定连接发送消息
     * @param {string} connectionId - 目标连接ID
     * @param {Object} message - 要发送的消息
     * @returns {boolean} 是否发送成功
     */
    sendToConnection(connectionId, message) {
        const connectionInfo = this.connections.get(connectionId);
        
        if (!connectionInfo) {
            console.error(`❌ 连接不存在: ${connectionId}`);
            return false;
        }

        if (connectionInfo.ws.readyState === WebSocket.OPEN) {
            try {
                connectionInfo.ws.send(JSON.stringify(message));
                connectionInfo.lastActivity = new Date();
                connectionInfo.messageCount++;
                return true;
            } catch (error) {
                console.error(`❌ 向连接 ${connectionId} 发送消息失败:`, error.message);
                connectionInfo.isAlive = false;
                return false;
            }
        } else {
            console.log(`⚠️  连接 ${connectionId} 状态异常: ${connectionInfo.ws.readyState}`);
            connectionInfo.isAlive = false;
            return false;
        }
    }

    /**
     * 更新连接活动时间
     * @param {string} connectionId - 连接ID
     */
    updateActivity(connectionId) {
        const connectionInfo = this.connections.get(connectionId);
        if (connectionInfo) {
            connectionInfo.lastActivity = new Date();
            connectionInfo.messageCount++;
        }
    }

    /**
     * 通过WebSocket对象更新连接活动时间
     * @param {WebSocket} ws - WebSocket连接对象
     */
    updateActivityByWs(ws) {
        for (const [connectionId, connectionInfo] of this.connections) {
            if (connectionInfo.ws === ws) {
                this.updateActivity(connectionId);
                break;
            }
        }
    }

    /**
     * 清理不活跃的连接
     */
    cleanupInactiveConnections() {
        const connectionsToRemove = [];

        for (const [connectionId, connectionInfo] of this.connections) {
            if (!connectionInfo.isAlive || connectionInfo.ws.readyState !== WebSocket.OPEN) {
                connectionsToRemove.push(connectionId);
            }
        }

        connectionsToRemove.forEach(connectionId => {
            this.removeConnection(connectionId);
        });

        if (connectionsToRemove.length > 0) {
            console.log(`🧹 清理了 ${connectionsToRemove.length} 个不活跃连接`);
        }
    }

    /**
     * 关闭所有连接
     * @param {number} code - 关闭代码
     * @param {string} reason - 关闭原因
     */
    closeAllConnections(code = 1000, reason = '服务器关闭') {
        console.log(`🛑 正在关闭所有连接 (${this.connections.size} 个)...`);

        for (const [connectionId, connectionInfo] of this.connections) {
            try {
                if (connectionInfo.ws.readyState === WebSocket.OPEN) {
                    connectionInfo.ws.close(code, reason);
                }
            } catch (error) {
                console.error(`❌ 关闭连接 ${connectionId} 时出错:`, error.message);
            }
        }

        this.connections.clear();
        console.log('✅ 所有连接已关闭');
    }

    /**
     * 获取连接统计信息
     * @returns {Object} 统计信息
     */
    getStatistics() {
        const connections = Array.from(this.connections.values());
        const now = new Date();

        const stats = {
            totalConnections: connections.length,
            maxConnections: this.maxConnections,
            connectionUtilization: (connections.length / this.maxConnections * 100).toFixed(1) + '%',
            averageConnectionTime: 0,
            totalMessages: 0,
            connectionsInfo: []
        };

        if (connections.length > 0) {
            let totalConnectionTime = 0;
            let totalMessages = 0;

            connections.forEach(conn => {
                const connectionTime = now - conn.connectedAt;
                totalConnectionTime += connectionTime;
                totalMessages += conn.messageCount;

                stats.connectionsInfo.push({
                    id: conn.id,
                    ip: conn.clientInfo.ip,
                    userAgent: conn.clientInfo.userAgent,
                    connectedAt: conn.connectedAt,
                    connectionTime: Math.floor(connectionTime / 1000) + 's',
                    messageCount: conn.messageCount,
                    lastActivity: conn.lastActivity
                });
            });

            stats.averageConnectionTime = Math.floor(totalConnectionTime / connections.length / 1000) + 's';
            stats.totalMessages = totalMessages;
        }

        return stats;
    }

    /**
     * 生成唯一连接ID
     * @returns {string} 连接ID
     */
    generateConnectionId() {
        this.connectionCounter++;
        const timestamp = Date.now().toString(36);
        const counter = this.connectionCounter.toString(36);
        return `conn_${timestamp}_${counter}`;
    }

    /**
     * 提取客户端信息
     * @param {http.IncomingMessage} request - HTTP请求对象
     * @returns {Object} 客户端信息
     */
    extractClientInfo(request) {
        return {
            ip: request.socket.remoteAddress || 'unknown',
            port: request.socket.remotePort || 0,
            userAgent: request.headers['user-agent'] || 'unknown',
            origin: request.headers.origin || 'unknown',
            host: request.headers.host || 'unknown'
        };
    }

    /**
     * 设置连接为活跃状态
     * @param {WebSocket} ws - WebSocket连接对象
     */
    setConnectionAlive(ws) {
        for (const [connectionId, connectionInfo] of this.connections) {
            if (connectionInfo.ws === ws) {
                connectionInfo.isAlive = true;
                break;
            }
        }
    }

    /**
     * 设置连接为不活跃状态
     * @param {WebSocket} ws - WebSocket连接对象
     */
    setConnectionDead(ws) {
        for (const [connectionId, connectionInfo] of this.connections) {
            if (connectionInfo.ws === ws) {
                connectionInfo.isAlive = false;
                break;
            }
        }
    }
}

module.exports = ConnectionManager;