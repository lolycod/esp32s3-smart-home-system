/**
 * 消息处理器
 * 负责处理和转发WebSocket消息
 */

const config = require('../config/server.config');

class MessageHandler {
    constructor(connectionManager) {
        this.connectionManager = connectionManager;
        this.messageTypes = config.message.types;
        this.maxMessageSize = config.message.maxSize;
        this.messageHistory = []; // 消息历史记录
        this.maxHistorySize = 1000; // 最大历史记录数
    }

    /**
     * 处理接收到的消息
     * @param {WebSocket} senderWs - 发送消息的WebSocket连接
     * @param {Buffer|string} rawData - 原始消息数据
     * @returns {Object} 处理结果
     */
    processMessage(senderWs, rawData) {
        try {
            // 检查消息大小
            if (rawData.length > this.maxMessageSize) {
                return {
                    success: false,
                    error: `消息大小超过限制 (${rawData.length}/${this.maxMessageSize} 字节)`
                };
            }

            // 解析消息
            const messageStr = rawData.toString();
            let message;
            
            try {
                message = JSON.parse(messageStr);
            } catch (parseError) {
                return {
                    success: false,
                    error: '消息格式无效，必须是有效的JSON'
                };
            }

            // 验证消息结构
            const validation = this.validateMessage(message);
            if (!validation.valid) {
                return {
                    success: false,
                    error: validation.error
                };
            }

            // 获取发送者信息
            const senderInfo = this.getSenderInfo(senderWs);
            
            // 处理不同类型的消息
            const processResult = this.handleMessageByType(message, senderInfo);
            if (!processResult.success) {
                return processResult;
            }

            // 准备转发的消息
            const forwardMessage = this.prepareForwardMessage(message, senderInfo);
            
            // 转发消息
            const forwardResult = this.forwardMessage(forwardMessage, senderWs);

            // 记录消息历史
            this.recordMessage(forwardMessage, senderInfo, forwardResult);

            console.log(`📨 消息处理完成: 类型=${message.type}, 发送者=${senderInfo.id}, 转发=${forwardResult.successCount}个客户端`);

            return {
                success: true,
                message: forwardMessage,
                forwardResult: forwardResult
            };

        } catch (error) {
            console.error('❌❌❌ 消息处理异常 ❌❌❌');
            console.error('错误类型:', error.name);
            console.error('错误消息:', error.message);
            console.error('错误堆栈:', error.stack);
            console.error('原始消息:', rawData.toString());
            return {
                success: false,
                error: '消息处理时发生内部错误: ' + error.message
            };
        }
    }

    /**
     * 验证消息格式
     * @param {Object} message - 消息对象
     * @returns {Object} 验证结果
     */
    validateMessage(message) {
        // 检查基本结构
        if (!message || typeof message !== 'object') {
            return {
                valid: false,
                error: '消息必须是一个对象'
            };
        }

        // 检查必需字段
        if (!message.type) {
            return {
                valid: false,
                error: '消息必须包含type字段'
            };
        }

        if (!message.timestamp) {
            return {
                valid: false,
                error: '消息必须包含timestamp字段'
            };
        }

        if (message.data === undefined) {
            return {
                valid: false,
                error: '消息必须包含data字段'
            };
        }

        // 检查消息类型
        const validTypes = Object.values(this.messageTypes);
        if (!validTypes.includes(message.type)) {
            return {
                valid: false,
                error: `无效的消息类型: ${message.type}，支持的类型: ${validTypes.join(', ')}`
            };
        }

        // 检查时间戳
        if (typeof message.timestamp !== 'number' || message.timestamp <= 0) {
            return {
                valid: false,
                error: '时间戳必须是正数'
            };
        }

        // 检查数据字段
        if (typeof message.data === 'object' && message.data !== null) {
            try {
                JSON.stringify(message.data);
            } catch (error) {
                return {
                    valid: false,
                    error: '消息数据包含不可序列化的内容'
                };
            }
        }

        return { valid: true };
    }

    /**
     * 根据消息类型处理消息
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 处理结果
     */
    handleMessageByType(message, senderInfo) {
        switch (message.type) {
            case this.messageTypes.MESSAGE:
                return this.handleUserMessage(message, senderInfo);

            case this.messageTypes.SYSTEM:
                return this.handleSystemMessage(message, senderInfo);

            case this.messageTypes.ERROR:
                return this.handleErrorMessage(message, senderInfo);

            case this.messageTypes.AI_DETECTION:
                return this.handleAIDetectionMessage(message, senderInfo);

            default:
                return {
                    success: false,
                    error: `不支持的消息类型: ${message.type}`
                };
        }
    }

    /**
     * 处理AI检测消息（来自MaixCAM）
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 处理结果
     */
    handleAIDetectionMessage(message, senderInfo) {
        // 验证AI检测数据结构
        if (typeof message.data !== 'object' || message.data === null) {
            return {
                success: false,
                error: 'AI检测消息data字段必须是对象'
            };
        }

        if (!Array.isArray(message.data.detections)) {
            return {
                success: false,
                error: 'AI检测消息必须包含detections数组'
            };
        }

        // 修复：处理senderInfo为null的情况（连接可能已断开但消息还在处理）
        const senderId = senderInfo ? senderInfo.id : 'unknown';
        console.log(`🎯 接收到AI检测数据: ${senderId}, 检测数量: ${message.data.detections.length}`);

        // AI检测消息直接广播，不需要额外处理
        return { success: true };
    }

    /**
     * 处理用户消息
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 处理结果
     */
    handleUserMessage(message, senderInfo) {
        // 允许data是字符串或对象（用于传感器数据等结构化数据）
        if (typeof message.data === 'string') {
            // 检查字符串消息内容
            if (message.data.trim().length === 0) {
                return {
                    success: false,
                    error: '用户消息内容不能为空'
                };
            }

            // 检查消息长度
            if (message.data.length > 10000) {
                return {
                    success: false,
                    error: '消息内容过长，最大支持10000字符'
                };
            }

            // 简单的内容过滤（可以根据需要扩展）
            const filteredData = this.filterMessageContent(message.data);
            if (filteredData !== message.data) {
                message.data = filteredData;
                console.log(`🔍 消息内容已过滤: ${senderInfo.id}`);
            }
        } else if (typeof message.data === 'object' && message.data !== null) {
            // 对象类型的data（如传感器数据），直接通过验证
            console.log(`📊 接收到结构化数据: ${senderInfo.id}`);
        } else {
            // data既不是字符串也不是对象
            return {
                success: false,
                error: '消息数据类型无效'
            };
        }

        return { success: true };
    }

    /**
     * 处理系统消息
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 处理结果
     */
    handleSystemMessage(message, senderInfo) {
        // 系统消息通常由服务器生成，客户端发送的系统消息需要特殊处理
        console.log(`⚠️  客户端 ${senderInfo.id} 尝试发送系统消息`);
        
        // 可以选择拒绝或者转换为普通消息
        message.type = this.messageTypes.MESSAGE;
        message.data = `[系统消息] ${message.data}`;
        
        return { success: true };
    }

    /**
     * 处理错误消息
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 处理结果
     */
    handleErrorMessage(message, senderInfo) {
        // 错误消息通常由服务器生成，记录客户端发送的错误消息
        console.log(`⚠️  客户端 ${senderInfo.id} 发送错误消息:`, message.data);
        
        // 转换为普通消息
        message.type = this.messageTypes.MESSAGE;
        message.data = `[错误报告] ${message.data}`;
        
        return { success: true };
    }

    /**
     * 准备转发的消息
     * @param {Object} originalMessage - 原始消息
     * @param {Object} senderInfo - 发送者信息
     * @returns {Object} 准备转发的消息
     */
    prepareForwardMessage(originalMessage, senderInfo) {
        return {
            type: originalMessage.type,
            timestamp: Date.now(), // 使用服务器时间戳
            data: originalMessage.data,
            sender: senderInfo ? {
                id: senderInfo.id,
                ip: senderInfo.clientInfo?.ip || 'unknown',
                connectedAt: senderInfo.connectedAt
            } : {
                id: 'unknown',
                ip: 'unknown',
                connectedAt: new Date()
            },
            messageId: this.generateMessageId()
        };
    }

    /**
     * 转发消息给其他客户端
     * @param {Object} message - 要转发的消息
     * @param {WebSocket} excludeWs - 要排除的连接（通常是发送者）
     * @returns {Object} 转发结果
     */
    forwardMessage(message, excludeWs = null) {
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
        const successCount = this.connectionManager.broadcastToAll(message, excludeConnectionId);
        
        return {
            successCount: successCount,
            totalConnections: this.connectionManager.getConnectionCount(),
            excludedConnections: excludeConnectionId ? 1 : 0
        };
    }

    /**
     * 获取发送者信息
     * @param {WebSocket} ws - WebSocket连接
     * @returns {Object|null} 发送者信息
     */
    getSenderInfo(ws) {
        const connections = this.connectionManager.getConnections();
        return connections.find(conn => conn.ws === ws) || null;
    }

    /**
     * 过滤消息内容
     * @param {string} content - 原始内容
     * @returns {string} 过滤后的内容
     */
    filterMessageContent(content) {
        // 简单的内容过滤示例
        // 可以根据需要添加更复杂的过滤逻辑
        
        // 移除多余的空白字符
        let filtered = content.trim().replace(/\s+/g, ' ');
        
        // 简单的敏感词过滤（示例）
        const sensitiveWords = ['spam', 'abuse'];
        sensitiveWords.forEach(word => {
            const regex = new RegExp(word, 'gi');
            filtered = filtered.replace(regex, '*'.repeat(word.length));
        });
        
        return filtered;
    }

    /**
     * 记录消息历史
     * @param {Object} message - 消息对象
     * @param {Object} senderInfo - 发送者信息
     * @param {Object} forwardResult - 转发结果
     */
    recordMessage(message, senderInfo, forwardResult) {
        const record = {
            messageId: message.messageId,
            type: message.type,
            timestamp: message.timestamp,
            senderId: senderInfo ? senderInfo.id : 'unknown',
            senderIP: senderInfo ? senderInfo.clientInfo.ip : 'unknown',
            dataLength: JSON.stringify(message.data).length,
            forwardCount: forwardResult.successCount,
            recordedAt: Date.now()
        };

        this.messageHistory.push(record);

        // 限制历史记录大小
        if (this.messageHistory.length > this.maxHistorySize) {
            this.messageHistory.shift();
        }
    }

    /**
     * 生成消息ID
     * @returns {string} 消息ID
     */
    generateMessageId() {
        const timestamp = Date.now().toString(36);
        const random = Math.random().toString(36).substr(2, 5);
        return `msg_${timestamp}_${random}`;
    }

    /**
     * 获取消息统计信息
     * @returns {Object} 统计信息
     */
    getMessageStatistics() {
        const now = Date.now();
        const oneHourAgo = now - 3600000; // 1小时前
        const oneDayAgo = now - 86400000; // 1天前

        const recentMessages = this.messageHistory.filter(msg => msg.timestamp > oneHourAgo);
        const dailyMessages = this.messageHistory.filter(msg => msg.timestamp > oneDayAgo);

        const typeStats = {};
        this.messageHistory.forEach(msg => {
            typeStats[msg.type] = (typeStats[msg.type] || 0) + 1;
        });

        return {
            totalMessages: this.messageHistory.length,
            recentMessages: recentMessages.length,
            dailyMessages: dailyMessages.length,
            messagesByType: typeStats,
            averageForwardCount: this.messageHistory.length > 0 
                ? (this.messageHistory.reduce((sum, msg) => sum + msg.forwardCount, 0) / this.messageHistory.length).toFixed(2)
                : 0,
            historySize: this.messageHistory.length,
            maxHistorySize: this.maxHistorySize
        };
    }

    /**
     * 获取消息历史
     * @param {number} limit - 限制数量
     * @param {string} type - 消息类型过滤
     * @returns {Array} 消息历史
     */
    getMessageHistory(limit = 50, type = null) {
        let history = [...this.messageHistory];
        
        if (type) {
            history = history.filter(msg => msg.type === type);
        }
        
        return history
            .sort((a, b) => b.timestamp - a.timestamp)
            .slice(0, limit);
    }

    /**
     * 清除消息历史
     */
    clearMessageHistory() {
        const clearedCount = this.messageHistory.length;
        this.messageHistory = [];
        console.log(`🧹 已清除 ${clearedCount} 条消息历史记录`);
        return clearedCount;
    }

    /**
     * 发送系统广播消息
     * @param {string} data - 消息内容
     * @returns {Object} 发送结果
     */
    broadcastSystemMessage(data) {
        const systemMessage = {
            type: this.messageTypes.SYSTEM,
            timestamp: Date.now(),
            data: data,
            sender: {
                id: 'system',
                ip: 'server',
                connectedAt: new Date()
            },
            messageId: this.generateMessageId()
        };

        const result = this.connectionManager.broadcastToAll(systemMessage);
        
        // 记录系统消息
        this.recordMessage(systemMessage, { 
            id: 'system', 
            clientInfo: { ip: 'server' } 
        }, result);

        return {
            success: true,
            message: systemMessage,
            forwardResult: result
        };
    }
}

module.exports = MessageHandler;