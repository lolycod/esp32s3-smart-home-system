/**
 * 智能终端管理系统 - 主服务器文件
 * 启动统一服务器（HTTP + WebSocket）并处理进程信号
 */

const UnifiedServer = require('./lib/UnifiedServer');
const config = require('./config/server.config');

class TerminalManagerServer {
    constructor() {
        this.unifiedServer = new UnifiedServer();
        this.setupProcessHandlers();
    }

    /**
     * 启动统一服务器
     */
    async start() {
        try {
            console.log('🚀 启动智能终端管理系统统一服务器...');
            console.log('📋 服务器配置信息:');
            console.log(`   - 默认端口: ${config.defaultPort} (HTTP + WebSocket)`);
            console.log(`   - 端口范围: ${config.portRange.min}-${config.portRange.max}`);
            console.log(`   - 最大连接数: ${config.websocket.maxConnections}`);
            console.log(`   - 心跳间隔: ${config.websocket.heartbeatInterval}ms`);
            console.log(`   - HTTP服务: ${config.unified.enableHTTP ? '启用' : '禁用'}`);
            console.log(`   - WebSocket服务: ${config.unified.enableWebSocket ? '启用' : '禁用'}`);
            console.log('');

            // 启动统一服务器（HTTP + WebSocket）
            const port = await this.unifiedServer.start();

            console.log('');
            console.log('🎉 统一服务器启动成功！');
            console.log('📝 使用说明:');
            console.log(`   1. 打开Web管理界面: http://localhost:${port}`);
            console.log(`   2. WebSocket连接地址: ws://localhost:${port}`);
            console.log(`   3. 外网访问地址: ws://www.lolycod123.top:${port}`);
            console.log('   4. 单端口设计，简化网络配置');
            console.log('');
            console.log('⌨️  按 Ctrl+C 停止服务器');
            console.log('');

            // 定期显示服务器状态
            this.startStatusReporting();

        } catch (error) {
            console.error('💥 统一服务器启动失败:', error.message);
            process.exit(1);
        }
    }

    /**
     * 停止统一服务器
     */
    async stop() {
        console.log('');
        console.log('🛑 正在关闭统一服务器...');

        try {
            // 停止统一服务器（HTTP + WebSocket）
            await this.unifiedServer.stop();

            console.log('✅ 统一服务器已安全关闭');
            process.exit(0);
        } catch (error) {
            console.error('❌ 统一服务器关闭时出错:', error.message);
            process.exit(1);
        }
    }

    /**
     * 设置进程信号处理
     */
    setupProcessHandlers() {
        // 处理 Ctrl+C
        process.on('SIGINT', () => {
            console.log('');
            console.log('📡 收到停止信号 (SIGINT)');
            this.stop();
        });

        // 处理终止信号
        process.on('SIGTERM', () => {
            console.log('');
            console.log('📡 收到终止信号 (SIGTERM)');
            this.stop();
        });

        // 处理未捕获的异常
        process.on('uncaughtException', (error) => {
            console.error('💥 未捕获的异常:', error);
            this.stop();
        });

        // 处理未处理的Promise拒绝
        process.on('unhandledRejection', (reason, promise) => {
            console.error('💥 未处理的Promise拒绝:', reason);
            this.stop();
        });
    }

    /**
     * 开始状态报告
     */
    startStatusReporting() {
        // 每30秒显示一次服务器状态
        setInterval(() => {
            const serverStatus = this.unifiedServer.getStatus();
            const uptime = Math.floor(serverStatus.uptime);
            const hours = Math.floor(uptime / 3600);
            const minutes = Math.floor((uptime % 3600) / 60);
            const seconds = uptime % 60;

            console.log(`📊 统一服务器状态 - 运行时间: ${hours}h ${minutes}m ${seconds}s, WebSocket连接数: ${serverStatus.connectionCount}, 服务端口: ${serverStatus.port}`);
        }, 30000);
    }
}

// 启动服务器
const server = new TerminalManagerServer();
server.start();