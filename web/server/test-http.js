/**
 * HTTP服务器测试脚本
 */

const http = require('http');
const fs = require('fs');
const path = require('path');

function testHTTPServer() {
    const clientPath = path.join(__dirname, '../client');
    const testPort = 8081; // 使用不同的端口测试

    console.log('🧪 开始测试HTTP服务器...');
    console.log(`📁 静态文件目录: ${clientPath}`);
    console.log(`🔌 测试端口: ${testPort}`);

    // 检查client目录是否存在
    if (!fs.existsSync(clientPath)) {
        console.error('❌ client目录不存在:', clientPath);
        return;
    }

    // 检查index.html是否存在
    const indexPath = path.join(clientPath, 'index.html');
    if (!fs.existsSync(indexPath)) {
        console.error('❌ index.html文件不存在:', indexPath);
        return;
    }

    console.log('✅ 静态文件检查通过');

    // 创建简单的HTTP服务器
    const server = http.createServer((req, res) => {
        console.log(`📨 收到请求: ${req.method} ${req.url}`);

        let pathname = req.url;
        if (pathname === '/') {
            pathname = '/index.html';
        }

        const filePath = path.join(clientPath, pathname);
        console.log(`📄 尝试提供文件: ${filePath}`);

        fs.readFile(filePath, (err, data) => {
            if (err) {
                console.error(`❌ 文件读取失败: ${err.message}`);
                res.writeHead(404, {'Content-Type': 'text/html'});
                res.end('<h1>404 - 文件未找到</h1>');
                return;
            }

            const ext = path.extname(filePath).toLowerCase();
            const contentType = {
                '.html': 'text/html',
                '.js': 'application/javascript',
                '.css': 'text/css'
            }[ext] || 'text/plain';

            res.writeHead(200, {'Content-Type': contentType});
            res.end(data);
            console.log(`✅ 成功发送文件: ${filePath} (${data.length} bytes)`);
        });
    });

    server.listen(testPort, (err) => {
        if (err) {
            console.error('❌ HTTP服务器启动失败:', err.message);
            return;
        }

        console.log('✅ HTTP服务器测试启动成功');
        console.log(`🌐 测试地址: http://localhost:${testPort}`);
        console.log('');
        console.log('🔍 测试说明:');
        console.log(`   1. 打开浏览器访问: http://localhost:${testPort}`);
        console.log('   2. 检查是否能正常显示页面');
        console.log('   3. 检查CSS和JS文件是否能正常加载');
        console.log('');
        console.log('⏰ 测试服务器将在30秒后自动关闭');

        // 30秒后关闭服务器
        setTimeout(() => {
            server.close(() => {
                console.log('✅ 测试服务器已关闭');
            });
        }, 30000);
    });

    server.on('error', (err) => {
        console.error('❌ HTTP服务器错误:', err);
    });
}

// 运行测试
testHTTPServer();