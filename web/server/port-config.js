/**
 * 端口配置工具
 * 用于快速切换和测试不同端口配置
 */

const WebSocketServer = require('./lib/WebSocketServer');
const config = require('./config/server.config');

class PortConfigurator {
    constructor() {
        this.wss = new WebSocketServer();
        this.currentPort = null;
        this.testResults = [];
    }

    /**
     * 测试常用端口
     */
    async testCommonPorts() {
        const commonPorts = [80, 443, 8080, 3000, 3001, 8090];

        console.log('🧪 开始测试常用端口...');

        for (const port of commonPorts) {
            try {
                console.log(`\n🔍 测试端口 ${port}...`);
                await this.testPort(port);
                this.testResults.push({ port, status: 'success', message: '端口可用' });
                console.log(`✅ 端口 ${port} 测试成功`);

                // 如果测试成功，停止测试
                break;
            } catch (error) {
                this.testResults.push({ port, status: 'failed', message: error.message });
                console.log(`❌ 端口 ${port} 测试失败: ${error.message}`);
            }
        }

        return this.testResults;
    }

    /**
     * 测试指定端口
     */
    async testPort(port) {
        return new Promise((resolve, reject) => {
            const http = require('http');
            const server = http.createServer();

            server.listen(port, '0.0.0.0', () => {
                console.log(`✅ 端口 ${port} 可以监听`);
                server.close(() => {
                    resolve(port);
                });
            });

            server.on('error', (error) => {
                if (error.code === 'EADDRINUSE') {
                    reject(new Error('端口被占用'));
                } else if (error.code === 'EACCES') {
                    reject(new Error('权限不足（需要管理员权限）'));
                } else {
                    reject(error);
                }
            });
        });
    }

    /**
     * 启动服务器在推荐端口
     */
    async startOnRecommendedPort() {
        console.log('🚀 尝试在推荐端口启动服务器...');

        // 优先尝试8080端口
        try {
            this.currentPort = await this.wss.start(8080);
            console.log(`✅ 服务器成功启动在端口 ${this.currentPort}`);
            return this.currentPort;
        } catch (error) {
            console.log(`⚠️  8080端口不可用: ${error.message}`);
        }

        // 尝试其他常用端口
        const alternativePorts = [8090, 3001, 3002, 3010];

        for (const port of alternativePorts) {
            try {
                this.currentPort = await this.wss.start(port);
                console.log(`✅ 服务器成功启动在端口 ${this.currentPort}`);
                return this.currentPort;
            } catch (error) {
                console.log(`⚠️  ${port}端口不可用: ${error.message}`);
            }
        }

        throw new Error('所有常用端口都不可用，请检查系统配置');
    }

    /**
     * 获取当前服务器信息
     */
    getServerInfo() {
        if (!this.currentPort) {
            return null;
        }

        return {
            port: this.currentPort,
            localUrl: `ws://localhost:${this.currentPort}`,
            lanUrl: `ws://${this.getLocalIP()}:${this.currentPort}`,
            publicUrl: `ws://www.lolycod123.top:${this.currentPort}`
        };
    }

    /**
     * 获取本地IP地址
     */
    getLocalIP() {
        const os = require('os');
        const interfaces = os.networkInterfaces();

        for (const name of Object.keys(interfaces)) {
            for (const iface of interfaces[name]) {
                if (iface.family === 'IPv4' && !iface.internal) {
                    return iface.address;
                }
            }
        }

        return '127.0.0.1';
    }

    /**
     * 生成配置报告
     */
    generateConfigReport() {
        const serverInfo = this.getServerInfo();

        if (!serverInfo) {
            return '服务器尚未启动';
        }

        return `
🎯 **推荐配置方案**

**当前服务器配置：**
- 本地访问: ${serverInfo.localUrl}
- 局域网访问: ${serverInfo.lanUrl}
- 外网访问: ${serverInfo.publicUrl}

**DTU设备配置建议：**
1. 如果内网穿透支持端口映射，使用: ${serverInfo.publicUrl}
2. 如果使用80端口转发，使用: ws://www.lolycod123.top
3. 如果使用HTTPS，使用: wss://www.lolycod123.top

**端口测试报告：**
${this.testResults.map(result =>
    `- 端口 ${result.port}: ${result.status === 'success' ? '✅' : '❌'} ${result.message}`
).join('\n')}

**下一步操作：**
1. 检查你的内网穿透配置
2. 确认端口映射是否正确
3. 验证防火墙设置
4. 测试外网连接
        `.trim();
    }

    /**
     * 停止服务器
     */
    async stop() {
        if (this.wss) {
            await this.wss.stop();
            this.currentPort = null;
            console.log('✅ 服务器已停止');
        }
    }
}

// 如果直接运行此脚本
if (require.main === module) {
    const configurator = new PortConfigurator();

    console.log('🔧 WebSocket服务器端口配置工具');
    console.log('='.repeat(50));

    configurator.testCommonPorts()
        .then(() => configurator.startOnRecommendedPort())
        .then(port => {
            console.log('\n' + configurator.generateConfigReport());

            // 保持服务器运行
            console.log('\n🔄 服务器正在运行，按 Ctrl+C 停止...');

            process.on('SIGINT', async () => {
                console.log('\n🛑 正在停止服务器...');
                await configurator.stop();
                process.exit(0);
            });
        })
        .catch(error => {
            console.error('❌ 配置失败:', error.message);
            process.exit(1);
        });
}

module.exports = PortConfigurator;