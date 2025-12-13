/**
 * WebSocket服务器测试文件
 * 用于测试服务器的基本功能
 */

const WebSocketServer = require('./lib/WebSocketServer');

async function testServer() {
    console.log('🧪 开始测试WebSocket服务器...');
    
    const server = new WebSocketServer();
    
    try {
        // 测试服务器启动
        console.log('1️⃣ 测试服务器启动...');
        const port = await server.start();
        console.log(`✅ 服务器启动成功，端口: ${port}`);
        
        // 测试服务器状态
        console.log('2️⃣ 测试服务器状态...');
        const status = server.getStatus();
        console.log('✅ 服务器状态:', status);
        
        // 等待5秒
        console.log('3️⃣ 等待5秒...');
        await new Promise(resolve => setTimeout(resolve, 5000));
        
        // 测试服务器停止
        console.log('4️⃣ 测试服务器停止...');
        await server.stop();
        console.log('✅ 服务器停止成功');
        
        console.log('🎉 所有测试通过！');
        
    } catch (error) {
        console.error('❌ 测试失败:', error.message);
        process.exit(1);
    }
}

// 运行测试
testServer();