/**
 * HTTP服务器类
 * 提供静态文件服务，支持前端部署
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

class HTTPServer {
    constructor() {
        this.server = null;
        this.port = 8090; // 修改为8090，避免8080端口冲突
        this.isRunning = false;
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
     * 启动HTTP服务器
     * @param {number} port - 端口号
     * @returns {Promise<number>} 实际使用的端口号
     */
    async start(port = null) {
        if (this.isRunning) {
            throw new Error('HTTP服务器已经在运行中');
        }

        const targetPort = port || this.port;

        return new Promise((resolve, reject) => {
            this.server = http.createServer((req, res) => {
                this.handleRequest(req, res);
            });

            this.server.on('error', (error) => {
                console.error('HTTP服务器错误:', error);
                if (error.code === 'EADDRINUSE') {
                    // 端口被占用，尝试下一个端口
                    console.log(`⚠️  HTTP端口 ${targetPort} 被占用，尝试端口 ${targetPort + 1}`);
                    // 关闭当前服务器实例
                    if (this.server) {
                        this.server.close();
                    }
                    // 递归尝试下一个端口
                    this.start(targetPort + 1)
                        .then(resolve)
                        .catch(reject);
                } else {
                    reject(error);
                }
            });

            this.server.listen(targetPort, () => {
                this.port = targetPort;
                this.isRunning = true;
                console.log(`✅ HTTP服务器已启动`);
                console.log(`🌐 Web管理界面: http://localhost:${targetPort}`);
                console.log(`📁 静态文件目录: ${this.clientPath}`);
                resolve(targetPort);
            });
        });
    }

    /**
     * 处理HTTP请求
     * @param {http.IncomingMessage} req - 请求对象
     * @param {http.ServerResponse} res - 响应对象
     */
    handleRequest(req, res) {
        try {
            const parsedUrl = url.parse(req.url);
            let pathname = parsedUrl.pathname;

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
            console.error('处理请求时出错:', error);
            this.send500(res, error.message);
        }
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
     * 停止HTTP服务器
     * @returns {Promise<void>}
     */
    stop() {
        return new Promise((resolve) => {
            if (!this.isRunning) {
                resolve();
                return;
            }

            console.log('🛑 正在停止HTTP服务器...');

            this.server.close(() => {
                console.log('✅ HTTP服务器已关闭');
                this.isRunning = false;
                resolve();
            });
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
            clientPath: this.clientPath
        };
    }
}

module.exports = HTTPServer;