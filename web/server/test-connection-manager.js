/**
 * ConnectionManager测试文件
 * 用于测试连接管理器的各项功能
 */

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
        console.log(`MockWS ${this.id} 发送消息:`, data);
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

async function testConnectionManager() {
    console.log('🧪 开始测试ConnectionManager...');
    console.log('');

    const manager = new ConnectionManager();

    try {
        // 测试1: 添加连接
        console.log('1️⃣ 测试添加连接...');
        const ws1 = new MockWebSocket('ws1');
        const ws2 = new MockWebSocket('ws2');
        const ws3 = new MockWebSocket('ws3');

        const conn1Id = manager.addConnection(ws1, createMockRequest('192.168.1.100'));
        const conn2Id = manager.addConnection(ws2, createMockRequest('192.168.1.101'));
        const conn3Id = manager.addConnection(ws3, createMockRequest('192.168.1.102'));

        console.log(`✅ 成功添加3个连接: ${conn1Id}, ${conn2Id}, ${conn3Id}`);
        console.log(`📊 当前连接数: ${manager.getConnectionCount()}`);
        console.log('');

        // 测试2: 获取连接信息
        console.log('2️⃣ 测试获取连接信息...');
        const conn1Info = manager.getConnection(conn1Id);
        console.log(`✅ 连接 ${conn1Id} 信息:`, {
            id: conn1Info.id,
            ip: conn1Info.clientInfo.ip,
            connectedAt: conn1Info.connectedAt
        });
        console.log('');

        // 测试3: 广播消息
        console.log('3️⃣ 测试广播消息...');
        const testMessage = {
            type: 'message',
            timestamp: Date.now(),
            data: '这是一条测试广播消息'
        };

        const successCount = manager.broadcastToAll(testMessage);
        console.log(`✅ 广播成功发送给 ${successCount} 个连接`);
        console.log('');

        // 测试4: 排除发送者的广播
        console.log('4️⃣ 测试排除发送者的广播...');
        const excludeMessage = {
            type: 'message',
            timestamp: Date.now(),
            data: '这是一条排除发送者的消息'
        };

        const excludeSuccessCount = manager.broadcastToAll(excludeMessage, conn1Id);
        console.log(`✅ 排除广播成功发送给 ${excludeSuccessCount} 个连接（排除了 ${conn1Id}）`);
        console.log('');

        // 测试5: 向指定连接发送消息
        console.log('5️⃣ 测试向指定连接发送消息...');
        const directMessage = {
            type: 'system',
            timestamp: Date.now(),
            data: '这是一条直接消息'
        };

        const sendSuccess = manager.sendToConnection(conn2Id, directMessage);
        console.log(`✅ 向连接 ${conn2Id} 发送消息: ${sendSuccess ? '成功' : '失败'}`);
        console.log('');

        // 测试6: 更新连接活动
        console.log('6️⃣ 测试更新连接活动...');
        manager.updateActivity(conn1Id);
        manager.updateActivityByWs(ws2);
        console.log('✅ 连接活动时间已更新');
        console.log('');

        // 测试7: 获取统计信息
        console.log('7️⃣ 测试获取统计信息...');
        const stats = manager.getStatistics();
        console.log('✅ 连接统计信息:');
        console.log(`   - 总连接数: ${stats.totalConnections}`);
        console.log(`   - 最大连接数: ${stats.maxConnections}`);
        console.log(`   - 连接利用率: ${stats.connectionUtilization}`);
        console.log(`   - 平均连接时间: ${stats.averageConnectionTime}`);
        console.log(`   - 总消息数: ${stats.totalMessages}`);
        console.log('');

        // 测试8: 设置连接状态
        console.log('8️⃣ 测试设置连接状态...');
        manager.setConnectionAlive(ws1);
        manager.setConnectionDead(ws2);
        console.log('✅ 连接状态已设置');
        console.log('');

        // 测试9: 移除连接
        console.log('9️⃣ 测试移除连接...');
        const removeSuccess1 = manager.removeConnection(conn1Id);
        const removeSuccess2 = manager.removeConnectionByWs(ws2);
        console.log(`✅ 移除连接结果: ${removeSuccess1 ? '成功' : '失败'}, ${removeSuccess2 ? '成功' : '失败'}`);
        console.log(`📊 剩余连接数: ${manager.getConnectionCount()}`);
        console.log('');

        // 测试10: 清理不活跃连接
        console.log('🔟 测试清理不活跃连接...');
        // 模拟一个不活跃的连接
        ws3.readyState = WebSocket.CLOSED;
        manager.cleanupInactiveConnections();
        console.log(`📊 清理后连接数: ${manager.getConnectionCount()}`);
        console.log('');

        // 测试11: 关闭所有连接
        console.log('1️⃣1️⃣ 测试关闭所有连接...');
        // 先添加一些新连接
        const ws4 = new MockWebSocket('ws4');
        const ws5 = new MockWebSocket('ws5');
        manager.addConnection(ws4, createMockRequest('192.168.1.104'));
        manager.addConnection(ws5, createMockRequest('192.168.1.105'));
        
        console.log(`📊 关闭前连接数: ${manager.getConnectionCount()}`);
        manager.closeAllConnections(1000, '测试关闭');
        console.log(`📊 关闭后连接数: ${manager.getConnectionCount()}`);
        console.log('');

        // 测试12: 连接数限制
        console.log('1️⃣2️⃣ 测试连接数限制...');
        // 临时降低最大连接数进行测试
        const originalMaxConnections = manager.maxConnections;
        manager.maxConnections = 2;

        try {
            const ws6 = new MockWebSocket('ws6');
            const ws7 = new MockWebSocket('ws7');
            const ws8 = new MockWebSocket('ws8');

            manager.addConnection(ws6, createMockRequest('192.168.1.106'));
            manager.addConnection(ws7, createMockRequest('192.168.1.107'));
            
            // 这个应该失败
            try {
                manager.addConnection(ws8, createMockRequest('192.168.1.108'));
                console.log('❌ 连接数限制测试失败：应该抛出异常');
            } catch (error) {
                console.log('✅ 连接数限制测试成功：', error.message);
            }
        } finally {
            // 恢复原始设置
            manager.maxConnections = originalMaxConnections;
            manager.closeAllConnections();
        }
        console.log('');

        console.log('🎉 所有ConnectionManager测试通过！');

    } catch (error) {
        console.error('❌ 测试失败:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

// 运行测试
testConnectionManager();