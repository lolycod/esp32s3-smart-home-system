/**
 * 测试智谱AI API连接
 */

const https = require('https');

const API_KEY = '493d8b1ceaa044168412775f8a4dd707.bebu6cSgM4R7o7wj';
const BASE_URL = 'open.bigmodel.cn';
const API_PATH = '/api/paas/v4/chat/completions';

function testAIAPI() {
    console.log('🔍 测试智谱AI API连接...');
    
    const requestBody = JSON.stringify({
        model: 'glm-4-flash',
        messages: [
            {
                role: 'user',
                content: '你好，请简单回复一下'
            }
        ],
        stream: false,
        temperature: 0.7
    });

    const options = {
        hostname: BASE_URL,
        port: 443,
        path: API_PATH,
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': `Bearer ${API_KEY}`,
            'Content-Length': Buffer.byteLength(requestBody)
        }
    };

    console.log(`🔍 请求URL: https://${BASE_URL}${API_PATH}`);
    console.log(`🔍 API Key: ${API_KEY.substring(0, 10)}...`);

    const req = https.request(options, (res) => {
        console.log(`📡 响应状态码: ${res.statusCode}`);
        console.log(`📡 响应头:`, res.headers);

        let responseData = '';
        res.on('data', (chunk) => {
            responseData += chunk;
        });

        res.on('end', () => {
            console.log(`📡 响应数据:`, responseData);
            
            if (res.statusCode === 200) {
                try {
                    const parsed = JSON.parse(responseData);
                    console.log('✅ AI API测试成功!');
                    console.log('📝 AI回复:', parsed.choices?.[0]?.message?.content);
                } catch (e) {
                    console.error('❌ 解析响应JSON失败:', e.message);
                }
            } else {
                console.error('❌ AI API测试失败');
            }
        });
    });

    req.on('error', (error) => {
        console.error('❌ 网络请求错误:', error);
    });

    req.write(requestBody);
    req.end();
}

// 运行测试
testAIAPI();