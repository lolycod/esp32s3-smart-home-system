/**
 * WebSocket连接测试工具
 * 用于诊断连接问题
 */

const WebSocket = require('ws');

function testConnection(serverUrl) {
    console.log(`🔍 测试WebSocket连接: ${serverUrl}`);
    console.log('⏰ 开始时间:', new Date().toLocaleString());

    const ws = new WebSocket(serverUrl, {
        handshakeTimeout: 10000, // 10秒超时
        protocolVersion: 13,
        origin: 'http://localhost:8090'
    });

    ws.on('open', function open() {
        console.log('✅ WebSocket连接成功建立');
        console.log('📊 连接状态:', ws.readyState);
        console.log('🏷️ 协议:', ws.protocol || '默认');
        console.log('📋 扩展:', ws.extensions || '无');

        // 发送测试消息
        const testMessage = {
            type: 'message',
            timestamp: Date.now(),
            data: '测试连接消息'
        };

        console.log('📤 发送测试消息:', testMessage);
        ws.send(JSON.stringify(testMessage));
    });

    ws.on('message', function message(data) {
        console.log('📥 收到消息:', data.toString());
    });

    ws.on('error', function error(err) {
        console.error('❌ WebSocket错误:', err.message);
        console.error('🔍 错误详情:', {
            code: err.code,
            errno: err.errno,
            syscall: err.syscall,
            address: err.address,
            port: err.port
        });
    });

    ws.on('close', function close(code, reason) {
        console.log(`🔌 连接关闭 - 代码: ${code}, 原因: ${reason || '无'}`);
        console.log('📚 关闭代码含义:');
        console.log('  1000: 正常关闭');
        console.log('  1006: 连接异常断开');
        console.log('  1002: 协议错误');
        console.log('  1003: 数据类型不支持');
    });

    ws.on('unexpected-response', function(request, response) {
        console.error('🚨 意外响应:', {
            statusCode: response.statusCode,
            statusMessage: response.statusMessage,
            headers: response.headers
        });
    });

    ws.on('ping', function(data) {
        console.log('🏓 收到Ping');
    });

    ws.on('pong', function(data) {
        console.log('🏸 收到Pong');
    });

    // 设置超时
    setTimeout(() => {
        if (ws.readyState === WebSocket.CONNECTING) {
            console.log('⏰ 连接超时，正在关闭...');
            ws.terminate();
        }
    }, 15000);

    // 保持连接一段时间进行测试
    setTimeout(() => {
        if (ws.readyState === WebSocket.OPEN) {
            console.log('🔄 测试完成，关闭连接...');
            ws.close(1000, '测试完成');
        }
    }, 30000);
}

// 获取命令行参数
const args = process.argv.slice(2);
if (args.length === 0) {
    console.log('使用方法: node test-connection.js <WebSocket_URL>');
    console.log('');
    console.log('示例:');
    console.log('  node test-connection.js ws://localhost:3000');
    console.log('  node test-connection.js ws://www.lolycod123.top:3000');
    console.log('  node test-connection.js wss://www.lolycod123.top');
    process.exit(1);
}

const serverUrl = args[0];
console.log('='.repeat(60));
console.log('🧪 WebSocket连接测试工具');
console.log('='.repeat(60));

testConnection(serverUrl);