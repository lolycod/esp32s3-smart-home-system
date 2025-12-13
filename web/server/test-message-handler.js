/**
 * MessageHandler测试文件
 * 用于测试消息处理器的各项功能
 */

const MessageHandler = require('./lib/MessageHandler');
const ConnectionManager = require('./lib/ConnectionManager');
const WebSocket = require('ws');

// 模拟WebSocket连接对象
class MockWebSocket {
    constructor(id) {
        this.id = id;
        this.readyState = WebSocket.OPEN;
        this.messagesSent = [];
    }

    send(data) {
        this.messagesSent.push(data);
        console.log(`MockWS ${this.id} 发送消息:`, JSON.parse(data));
    }

    close(code, reason) {
        this.readyState = WebSocket.CLOSED;
        console.log(`MockWS ${this.id} 关闭连接: ${code} - ${reason}`);
    }
}

// 模拟HTTP请求对象
function createMockRequest(ip = '127.0.0.1', port = 12345) {
    return {
        socket: {
            remoteAddress: ip,
            remotePort: port
        },
        headers: {
            'user-agent': 'Test Client',
            'origin': 'http://localhost:3000',
            'host': 'localhost:3000'
        }
    };
}

async function testMessageHandler() {
    console.log('🧪 开始测试MessageHandler...');
    console.log('');

    const connectionManager = new ConnectionManager();
    const messageHandler = new MessageHandler(connectionManager);

    try {
        // 设置测试连接
        const ws1 = new MockWebSocket('ws1');
        const ws2 = new MockWebSocket('ws2');
        const ws3 = new MockWebSocket('ws3');

        const conn1Id = connectionManager.addConnection(ws1, createMockRequest('192.168.1.100'));
        const conn2Id = connectionManager.addConnection(ws2, createMockRequest('192.168.1.101'));
        const conn3Id = connectionManager.addConnection(ws3, createMockRequest('192.168.1.102'));

        console.log(`✅ 设置了3个测试连接: ${conn1Id}, ${conn2Id}, ${conn3Id}`);
        console.log('');

        // 测试1: 处理有效的用户消息
        console.log('1️⃣ 测试处理有效的用户消息...');
        const validMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: '这是一条测试消息'
        });

        const result1 = messageHandler.processMessage(ws1, Buffer.from(validMessage));
        console.log(`✅ 处理结果: ${result1.success ? '成功' : '失败'}`);
        if (result1.success) {
            console.log(`   转发给 ${result1.forwardResult.successCount} 个客户端`);
        }
        console.log('');

        // 测试2: 处理无效的消息格式
        console.log('2️⃣ 测试处理无效的消息格式...');
        const invalidMessage = 'invalid json';
        const result2 = messageHandler.processMessage(ws1, Buffer.from(invalidMessage));
        console.log(`✅ 处理结果: ${result2.success ? '成功' : '失败'}`);
        if (!result2.success) {
            console.log(`   错误信息: ${result2.error}`);
        }
        console.log('');

        // 测试3: 处理缺少必需字段的消息
        console.log('3️⃣ 测试处理缺少必需字段的消息...');
        const incompleteMessage = JSON.stringify({
            type: 'message',
            // 缺少timestamp和data字段
        });
        const result3 = messageHandler.processMessage(ws1, Buffer.from(incompleteMessage));
        console.log(`✅ 处理结果: ${result3.success ? '成功' : '失败'}`);
        if (!result3.success) {
            console.log(`   错误信息: ${result3.error}`);
        }
        console.log('');

        // 测试4: 处理系统消息
        console.log('4️⃣ 测试处理系统消息...');
        const systemMessage = JSON.stringify({
            type: 'system',
            timestamp: Date.now(),
            data: '这是一条系统消息'
        });
        const result4 = messageHandler.processMessage(ws2, Buffer.from(systemMessage));
        console.log(`✅ 处理结果: ${result4.success ? '成功' : '失败'}`);
        if (result4.success) {
            console.log(`   消息类型已转换为: ${result4.message.type}`);
        }
        console.log('');

        // 测试5: 处理错误消息
        console.log('5️⃣ 测试处理错误消息...');
        const errorMessage = JSON.stringify({
            type: 'error',
            timestamp: Date.now(),
            data: '这是一条错误消息'
        });
        const result5 = messageHandler.processMessage(ws3, Buffer.from(errorMessage));
        console.log(`✅ 处理结果: ${result5.success ? '成功' : '失败'}`);
        if (result5.success) {
            console.log(`   消息类型已转换为: ${result5.message.type}`);
        }
        console.log('');

        // 测试6: 处理过长的消息
        console.log('6️⃣ 测试处理过长的消息...');
        const longMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: 'x'.repeat(15000) // 超过10000字符限制
        });
        const result6 = messageHandler.processMessage(ws1, Buffer.from(longMessage));
        console.log(`✅ 处理结果: ${result6.success ? '成功' : '失败'}`);
        if (!result6.success) {
            console.log(`   错误信息: ${result6.error}`);
        }
        console.log('');

        // 测试7: 处理空消息内容
        console.log('7️⃣ 测试处理空消息内容...');
        const emptyMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: ''
        });
        const result7 = messageHandler.processMessage(ws1, Buffer.from(emptyMessage));
        console.log(`✅ 处理结果: ${result7.success ? '成功' : '失败'}`);
        if (!result7.success) {
            console.log(`   错误信息: ${result7.error}`);
        }
        console.log('');

        // 测试8: 测试内容过滤
        console.log('8️⃣ 测试内容过滤...');
        const spamMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: 'This is spam content with abuse words'
        });
        const result8 = messageHandler.processMessage(ws1, Buffer.from(spamMessage));
        console.log(`✅ 处理结果: ${result8.success ? '成功' : '失败'}`);
        if (result8.success) {
            console.log(`   过滤后内容: ${result8.message.data}`);
        }
        console.log('');

        // 测试9: 发送系统广播消息
        console.log('9️⃣ 测试发送系统广播消息...');
        const broadcastResult = messageHandler.broadcastSystemMessage('服务器维护通知');
        console.log(`✅ 广播结果: ${broadcastResult.success ? '成功' : '失败'}`);
        if (broadcastResult.success) {
            console.log(`   发送给 ${broadcastResult.forwardResult.successCount} 个客户端`);
        }
        console.log('');

        // 测试10: 获取消息统计信息
        console.log('🔟 测试获取消息统计信息...');
        const stats = messageHandler.getMessageStatistics();
        console.log('✅ 消息统计信息:');
        console.log(`   - 总消息数: ${stats.totalMessages}`);
        console.log(`   - 最近消息数: ${stats.recentMessages}`);
        console.log(`   - 每日消息数: ${stats.dailyMessages}`);
        console.log(`   - 平均转发数: ${stats.averageForwardCount}`);
        console.log(`   - 按类型统计:`, stats.messagesByType);
        console.log('');

        // 测试11: 获取消息历史
        console.log('1️⃣1️⃣ 测试获取消息历史...');
        const history = messageHandler.getMessageHistory(5);
        console.log(`✅ 获取到 ${history.length} 条历史消息`);
        history.forEach((msg, index) => {
            console.log(`   ${index + 1}. [${msg.type}] ${new Date(msg.timestamp).toLocaleTimeString()} - 发送者: ${msg.senderId}`);
        });
        console.log('');

        // 测试12: 测试消息大小限制
        console.log('1️⃣2️⃣ 测试消息大小限制...');
        const hugeMsgData = 'x'.repeat(2 * 1024 * 1024); // 2MB消息
        const hugeMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: hugeMsgData
        });
        const result12 = messageHandler.processMessage(ws1, Buffer.from(hugeMessage));
        console.log(`✅ 处理结果: ${result12.success ? '成功' : '失败'}`);
        if (!result12.success) {
            console.log(`   错误信息: ${result12.error}`);
        }
        console.log('');

        // 测试13: 测试无效的消息类型
        console.log('1️⃣3️⃣ 测试无效的消息类型...');
        const invalidTypeMessage = JSON.stringify({
            type: 'invalid_type',
            timestamp: Date.now(),
            data: '无效类型的消息'
        });
        const result13 = messageHandler.processMessage(ws1, Buffer.from(invalidTypeMessage));
        console.log(`✅ 处理结果: ${result13.success ? '成功' : '失败'}`);
        if (!result13.success) {
            console.log(`   错误信息: ${result13.error}`);
        }
        console.log('');

        // 测试14: 清除消息历史
        console.log('1️⃣4️⃣ 测试清除消息历史...');
        const clearedCount = messageHandler.clearMessageHistory();
        console.log(`✅ 清除了 ${clearedCount} 条消息历史`);
        
        const newStats = messageHandler.getMessageStatistics();
        console.log(`   清除后总消息数: ${newStats.totalMessages}`);
        console.log('');

        // 测试15: 测试连接断开后的消息处理
        console.log('1️⃣5️⃣ 测试连接断开后的消息处理...');
        // 断开一个连接
        connectionManager.removeConnection(conn3Id);
        
        const afterDisconnectMessage = JSON.stringify({
            type: 'message',
            timestamp: Date.now(),
            data: '连接断开后的消息'
        });
        const result15 = messageHandler.processMessage(ws1, Buffer.from(afterDisconnectMessage));
        console.log(`✅ 处理结果: ${result15.success ? '成功' : '失败'}`);
        if (result15.success) {
            console.log(`   转发给 ${result15.forwardResult.successCount} 个客户端（应该少了1个）`);
        }
        console.log('');

        console.log('🎉 所有MessageHandler测试通过！');

    } catch (error) {
        console.error('❌ 测试失败:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

// 运行测试
testMessageHandler();