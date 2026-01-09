/**
 * 统一服务器类
 * 整合HTTP和WebSocket服务到单一端口，简化网络配置
 * 基于WebSocketServer_backup.js的单端口设计模式
 */

const WebSocket = require('ws');
const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');
const config = require('../config/server.config');
const ConnectionManager = require('./ConnectionManager');
const MessageHandler = require('./MessageHandler');
const AIService = require('./AIService');
const TTSService = require('./TTSService');
const DeviceProtocolManager = require('./DeviceProtocolManager');

class UnifiedServer {
    constructor() {
        this.server = null; // HTTP服务器实例
        this.wss = null; // WebSocket服务器实例
        this.port = config.defaultPort;
        this.isRunning = false;
        this.connectionManager = new ConnectionManager();
        this.messageHandler = new MessageHandler(this.connectionManager);
        this.aiService = new AIService(); // AI对话服务
        // 优先从环境变量或配置中加载TTS凭据
        const ttsCfg = (config && config.tts) || {};
        const appId = process.env.TTS_APP_ID || ttsCfg.appId;
        const apiKey = process.env.TTS_API_KEY || ttsCfg.apiKey;
        const apiSecret = process.env.TTS_API_SECRET || ttsCfg.apiSecret;
        this.ttsService = new TTSService(appId, apiKey, apiSecret); // 语音合成服务
        this.protocolManager = new DeviceProtocolManager(); // 设备协议管理器

        // 注册设备控制回调
        this.aiService.onDeviceControl = (deviceControl) => {
            this.broadcastDeviceCommands(deviceControl);
        };

        // HTTP静态文件服务配置
        this.clientPath = path.join(__dirname, '../../client');
        this.mimeTypes = {
            '.html': 'text/html',
            '.js': 'application/javascript',
            '.css': 'text/css',
            '.json': 'application/json',
            '.png': 'image/png',
            '.jpg': 'image/jpeg',
            '.gif': 'image/gif',
            '.svg': 'image/svg+xml',
            '.ico': 'image/x-icon'
        };
    }

    /**
     * 启动统一服务器（HTTP + WebSocket）
     * @param {number} port - 端口号（可选）
     * @returns {Promise<number>} 返回实际使用的端口号
     */
    async start(port = null) {
        if (this.isRunning) {
            throw new Error('统一服务器已经在运行中');
        }

        const targetPort = port || this.port;
        
        try {
            // 启动服务器
            const actualPort = await this.startOnPort(targetPort);
            this.port = actualPort;
            this.isRunning = true;
            
            console.log(`✅ 统一服务器已启动`);
            console.log(`📡 监听端口: ${actualPort}`);
            console.log(`🌐 HTTP服务地址: http://localhost:${actualPort}`);
            console.log(`🔌 WebSocket地址: ws://localhost:${actualPort}`);
            console.log(`📁 静态文件目录: ${this.clientPath}`);
            
            return actualPort;
        } catch (error) {
            console.error('❌ 统一服务器启动失败:', error.message);
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
            // 创建HTTP服务器，处理静态文件请求
            this.server = http.createServer((req, res) => {
                this.handleHTTPRequest(req, res);
            });
            
            // 创建WebSocket服务器，基于HTTP服务器
            this.wss = new WebSocket.Server({
                server: this.server,
                perMessageDeflate: false,
                // 添加CORS支持
                verifyClient: (info, cb) => {
                    // 允许所有来源（生产环境应该限制具体域名）
                    console.log(`🔍 WebSocket连接验证 - Origin: ${info.origin}, Secure: ${info.secure}`);
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
                    console.log(`✅ 统一服务器监听所有网络接口: 0.0.0.0:${port}`);
                    resolve(port);
                }
            });

            // 处理服务器错误
            this.server.on('error', (error) => {
                console.error('统一服务器错误:', error);
                reject(error);
            });
        });
    }

    /**
     * 处理HTTP请求（静态文件服务 + API路由）
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleHTTPRequest(req, res) {
        try {
            const parsedUrl = url.parse(req.url, true);
            let pathname = parsedUrl.pathname;

            // API路由处理
            if (pathname.startsWith('/api/')) {
                this.handleAPIRequest(req, res, pathname, parsedUrl);
                return;
            }

            // 默认显示index.html
            if (pathname === '/') {
                pathname = '/index.html';
            }

            // 移除路径中的..防止目录遍历攻击
            pathname = path.normalize(pathname).replace(/^(\.\.[\/\\])+/, '');

            const filePath = path.join(this.clientPath, pathname);

            // 检查文件是否存在
            fs.access(filePath, fs.constants.F_OK, (err) => {
                if (err) {
                    this.send404(res, pathname);
                    return;
                }

                // 读取并发送文件
                this.serveFile(res, filePath);
            });

        } catch (error) {
            console.error('处理HTTP请求时出错:', error);
            this.send500(res, error.message);
        }
    }

    /**
     * 处理API请求
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     * @param {string} pathname - 路径名
     * @param {Object} parsedUrl - 解析后的URL对象
     */
    handleAPIRequest(req, res, pathname, parsedUrl) {
        // 设置CORS头
        res.setHeader('Access-Control-Allow-Origin', '*');
        res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
        res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

        // 处理OPTIONS预检请求
        if (req.method === 'OPTIONS') {
            res.writeHead(200);
            res.end();
            return;
        }

        // AI对话API
        if (pathname === '/api/chat' && req.method === 'POST') {
            this.handleChatAPI(req, res);
            return;
        }

        // 清除对话历史API
        if (pathname === '/api/chat/clear' && req.method === 'POST') {
            this.handleClearHistoryAPI(req, res);
            return;
        }

        // 获取对话历史API
        if (pathname === '/api/chat/history' && req.method === 'GET') {
            this.handleGetHistoryAPI(req, res, parsedUrl);
            return;
        }

        // 获取设备状态API
        if (pathname === '/api/devices/state' && req.method === 'GET') {
            this.handleGetDeviceStateAPI(req, res, parsedUrl);
            return;
        }

        // 语音合成API（一次性返回PCM）
        if (pathname === '/api/tts' && req.method === 'POST') {
            this.handleTTSAPI(req, res);
            return;
        }

        // 语音合成流式API（SSE推送base64 PCM分片）
        if (pathname === '/api/tts/stream' && req.method === 'POST') {
            this.handleTTSStreamAPI(req, res);
            return;
        }

        // 获取音色列表API
        if (pathname === '/api/tts/voices' && req.method === 'GET') {
            this.handleVoicesAPI(req, res);
            return;
        }

        // 协议验证API
        if (pathname === '/api/protocol/validate' && req.method === 'POST') {
            this.handleProtocolValidateAPI(req, res);
            return;
        }

        // 协议文档API
        if (pathname === '/api/protocol/docs' && req.method === 'GET') {
            this.handleProtocolDocsAPI(req, res);
            return;
        }

        // 协议解析API
        if (pathname === '/api/protocol/parse' && req.method === 'POST') {
            this.handleProtocolParseAPI(req, res);
            return;
        }

        // API未找到
        res.writeHead(404, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'API endpoint not found' }));
    }

    /**
     * 处理AI对话API请求
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleChatAPI(req, res) {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const { message, sessionId = 'default', stream = true } = data;

                if (!message || typeof message !== 'string') {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: '缺少message参数' }));
                    return;
                }

                console.log(`🤖 AI对话请求 - Session: ${sessionId}, Stream: ${stream}`);
                console.log(`   用户消息: ${message}`);

                if (stream) {
                    // 流式输出（SSE）
                    res.writeHead(200, {
                        'Content-Type': 'text/event-stream',
                        'Cache-Control': 'no-cache',
                        'Connection': 'keep-alive',
                        'Access-Control-Allow-Origin': '*'
                    });

                    // 发送数据块
                    const sendChunk = (content) => {
                        res.write(`data: ${JSON.stringify({ type: 'chunk', content })}\n\n`);
                    };

                    // 发送完成消息
                    const sendComplete = (fullContent) => {
                        res.write(`data: ${JSON.stringify({ type: 'done', content: fullContent })}\n\n`);
                        res.end();
                    };

                    // 发送错误消息
                    const sendError = (error) => {
                        res.write(`data: ${JSON.stringify({ type: 'error', error: error.message })}\n\n`);
                        res.end();
                    };

                    // 调用AI服务
                    this.aiService.sendChatStream(
                        message,
                        sessionId,
                        sendChunk,
                        sendComplete,
                        sendError
                    );
                } else {
                    // 非流式输出
                    this.aiService.sendChat(message, sessionId)
                        .then(response => {
                            res.writeHead(200, { 'Content-Type': 'application/json' });
                            res.end(JSON.stringify({ content: response }));
                        })
                        .catch(error => {
                            console.error('AI对话错误:', error);
                            res.writeHead(500, { 'Content-Type': 'application/json' });
                            res.end(JSON.stringify({ error: error.message }));
                        });
                }
            } catch (error) {
                console.error('解析请求体失败:', error);
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: '请求格式错误' }));
            }
        });
    }

    /**
     * 处理清除对话历史API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleClearHistoryAPI(req, res) {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const { sessionId = 'default' } = data;

                this.aiService.clearHistory(sessionId);

                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ success: true, message: '对话历史已清除' }));
            } catch (error) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: '请求格式错误' }));
            }
        });
    }

    /**
     * 处理获取对话历史API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     * @param {Object} parsedUrl - 解析后的URL对象
     */
    handleGetHistoryAPI(req, res, parsedUrl) {
        const sessionId = parsedUrl.query.sessionId || 'default';
        const history = this.aiService.getHistory(sessionId);

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ history }));
    }

    /**
     * 处理获取设备状态API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     * @param {Object} parsedUrl - 解析后的URL对象
     */
    handleGetDeviceStateAPI(req, res, parsedUrl) {
        const sessionId = parsedUrl.query.sessionId || 'default';
        const deviceState = this.aiService.getCurrentDeviceState(sessionId);

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ deviceState }));
    }

    /**
     * 处理语音合成API请求
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleTTSAPI(req, res) {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', async () => {
            try {
                const data = JSON.parse(body);
                const { text, voice } = data;

                if (!text || typeof text !== 'string') {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: '缺少text参数' }));
                    return;
                }

                // 验证音色参数
                if (voice && !this.ttsService.isValidVoice(voice)) {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: `无效的音色: ${voice}` }));
                    return;
                }

                console.log(`🔊 TTS合成请求 - 文本: ${text.substring(0, 50)}... 音色: ${voice || '默认'}`);

                // 调用TTS服务合成音频
                const audioBuffer = await this.ttsService.synthesizeStream(text, res, voice);

                // 返回音频数据(PCM格式)
                res.writeHead(200, {
                    'Content-Type': 'audio/pcm',
                    'Content-Length': audioBuffer.length,
                    'Access-Control-Allow-Origin': '*'
                });
                res.end(audioBuffer);

                console.log(`✅ TTS合成完成 - 音频大小: ${audioBuffer.length} 字节`);

            } catch (error) {
                console.error('TTS合成错误:', error);
                res.writeHead(500, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: error.message }));
            }
        });
    }

    /**
     * 流式TTS（SSE）API：边合成边推送base64 PCM数据
     */
    handleTTSStreamAPI(req, res) {
        let body = '';
        req.on('data', chunk => { body += chunk.toString(); });
        req.on('end', async () => {
            try {
                const data = JSON.parse(body);
                const { text } = data;
                if (!text || typeof text !== 'string') {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: '缺少text参数' }));
                    return;
                }

                // SSE headers
                res.writeHead(200, {
                    'Content-Type': 'text/event-stream',
                    'Cache-Control': 'no-cache',
                    'Connection': 'keep-alive',
                    'Access-Control-Allow-Origin': '*'
                });

                const send = (obj) => res.write(`data: ${JSON.stringify(obj)}\n\n`);

                // 通过TTSService进行合成并流式返回
                this.ttsService.synthesize(
                    text,
                    // onAudioChunk
                    (audioData) => {
                        const b64 = audioData.toString('base64');
                        send({ type: 'chunk', audio: b64 });
                    },
                    // onComplete
                    () => {
                        send({ type: 'end' });
                        res.end();
                    },
                    // onError
                    (error) => {
                        send({ type: 'error', error: error.message });
                        res.end();
                    }
                );

            } catch (error) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: '请求格式错误' }));
            }
        });
    }

    /**
     * 提供文件服务
     * @param {http.ServerResponse} res - 响应对象
     * @param {string} filePath - 文件路径
     */
    serveFile(res, filePath) {
        const ext = path.extname(filePath).toLowerCase();
        const contentType = this.mimeTypes[ext] || 'application/octet-stream';

        fs.readFile(filePath, (err, data) => {
            if (err) {
                console.error(`读取文件失败: ${filePath}`, err);
                this.send500(res, '文件读取失败');
                return;
            }

            res.writeHead(200, {
                'Content-Type': contentType,
                'Content-Length': data.length,
                'Cache-Control': 'no-cache'
            });

            res.end(data);
            console.log(`📄 提供文件: ${filePath}`);
        });
    }

    /**
     * 发送404错误
     * @param {http.ServerResponse} res - 响应对象
     * @param {string} pathname - 请求的路径
     */
    send404(res, pathname) {
        const message = `文件未找到: ${pathname}`;
        res.writeHead(404, {
            'Content-Type': 'text/html; charset=utf-8'
        });
        res.end(`
            <!DOCTYPE html>
            <html>
            <head>
                <title>404 - 页面未找到</title>
                <meta charset="utf-8">
                <style>
                    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
                    h1 { color: #e74c3c; }
                    .back-link { color: #3498db; text-decoration: none; margin-top: 20px; display: inline-block; }
                    .back-link:hover { text-decoration: underline; }
                </style>
            </head>
            <body>
                <h1>404 - 页面未找到</h1>
                <p>${message}</p>
                <a href="/" class="back-link">返回首页</a>
            </body>
            </html>
        `);
        console.log(`❌ 404错误: ${pathname}`);
    }

    /**
     * 发送500错误
     * @param {http.ServerResponse} res - 响应对象
     * @param {string} errorMessage - 错误信息
     */
    send500(res, errorMessage) {
        res.writeHead(500, {
            'Content-Type': 'text/html; charset=utf-8'
        });
        res.end(`
            <!DOCTYPE html>
            <html>
            <head>
                <title>500 - 服务器错误</title>
                <meta charset="utf-8">
                <style>
                    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
                    h1 { color: #e74c3c; }
                </style>
            </head>
            <body>
                <h1>500 - 服务器内部错误</h1>
                <p>服务器处理请求时发生错误</p>
                <p><small>${errorMessage}</small></p>
            </body>
            </html>
        `);
        console.log(`❌ 500错误: ${errorMessage}`);
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
     * 停止统一服务器
     * @returns {Promise<void>}
     */
    stop() {
        return new Promise((resolve) => {
            if (!this.isRunning) {
                resolve();
                return;
            }

            console.log('🛑 正在停止统一服务器...');

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
            uptime: this.isRunning ? process.uptime() : 0,
            clientPath: this.clientPath
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
     * 发送系统广播消息
     * @param {string} message - 消息内容
     * @returns {Object} 发送结果
     */
    broadcastSystemMessage(message) {
        return this.messageHandler.broadcastSystemMessage(message);
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
     * 清除消息历史
     * @returns {number} 清除的消息数量
     */
    clearMessageHistory() {
        return this.messageHandler.clearMessageHistory();
    }

    /**
     * 广播设备控制命令到所有WebSocket客户端
     * @param {Object} deviceControl - AI返回的设备控制JSON
     */
    /**
     * 广播AI设备控制命令到所有WebSocket客户端
     * @param {Object} deviceControl - AI返回的设备控制JSON
     * @param {string} source - 命令来源 ('AI' | 'CONTROL' | 'DEBUG')
     */
    broadcastDeviceCommands(deviceControl, source = 'AI') {
        try {
            // 使用协议管理器生成标准化命令（模块化处理）
            const commandResults = this.protocolManager.generateCommands(deviceControl, source);

            // 广播所有命令
            commandResults.forEach(cmdResult => {
                const message = {
                    type: "message",
                    timestamp: cmdResult.timestamp,
                    data: cmdResult.command,
                    meta: {
                        device: cmdResult.device,
                        source: cmdResult.source
                    }
                };
                this.connectionManager.broadcastToAll(message);
                console.log(`🎛️  [${cmdResult.source}] ${cmdResult.device} 控制命令已广播: ${cmdResult.command}`);
            });

            console.log(`✅ 设备控制完成,共广播 ${commandResults.length} 条命令 [来源: ${source}]`);
        } catch (error) {
            console.error('❌ 设备命令生成失败:', error.message);
        }
    }

    /**
     * 处理协议验证API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleProtocolValidateAPI(req, res) {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const { command } = data;

                if (!command) {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: '缺少command参数' }));
                    return;
                }

                const isValid = this.protocolManager.validateCommand(command);
                let result = { valid: isValid, command };

                if (isValid) {
                    try {
                        const parsed = this.protocolManager.parseCommand(command);
                        result.parsed = parsed;
                        result.standardized = parsed.standardized;
                    } catch (error) {
                        result.error = error.message;
                    }
                }

                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify(result));

            } catch (error) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({ error: '请求格式错误' }));
            }
        });
    }

    /**
     * 处理协议文档API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleProtocolDocsAPI(req, res) {
        try {
            const docs = this.protocolManager.getProtocolDocumentation();
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify(docs, null, 2));
        } catch (error) {
            res.writeHead(500, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ error: '获取协议文档失败' }));
        }
    }

    /**
     * 处理协议解析API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleProtocolParseAPI(req, res) {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const { command } = data;

                if (!command) {
                    res.writeHead(400, { 'Content-Type': 'application/json' });
                    res.end(JSON.stringify({ error: '缺少command参数' }));
                    return;
                }

                const parsed = this.protocolManager.parseCommand(command);
                res.writeHead(200, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify(parsed, null, 2));

            } catch (error) {
                res.writeHead(400, { 'Content-Type': 'application/json' });
                res.end(JSON.stringify({
                    error: error.message,
                    command: data?.command || 'unknown'
                }));
            }
        });
    }

    /**
     * 处理音色列表API
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleVoicesAPI(req, res) {
        try {
            const voiceList = this.ttsService.getVoiceList();
            res.writeHead(200, {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            });
            res.end(JSON.stringify({
                success: true,
                data: voiceList
            }));
        } catch (error) {
            console.error('获取音色列表失败:', error);
            res.writeHead(500, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({
                success: false,
                error: '获取音色列表失败'
            }));
        }
    }
}

module.exports = UnifiedServer;
