/**
 * AI对话服务模块
 * 集成智谱AI GLM-4系列模型，提供流式和非流式对话能力
 */

const https = require('https');

class AIService {
    constructor(apiKey = process.env.ZHIPU_API_KEY || '588f9863b5f44a82abccf20015304675.4NHW6sZBkjNq7SxG') {
        this.apiKey = apiKey;
        this.baseURL = 'open.bigmodel.cn';
        this.apiPath = '/api/paas/v4/chat/completions';
        this.model = 'glm-4-plus'; // 默认使用glm-4-plus模型

        // 对话历史管理（按会话ID存储）
        this.conversationHistory = new Map();

        // 设备状态管理（按会话ID存储）
        this.deviceStates = new Map();

        // 设备控制回调（由UnifiedServer注册）
        this.onDeviceControl = null;
    }

    /**
     * 初始化默认设备状态
     */
    getDefaultDeviceState() {
        return {
            "风扇": {
                "开关": "关",
                "风速": 0
            },
            "LED灯": {
                "开关": "关",
                "色温": "冷光",
                "亮度": 50
            },
            "窗帘": {
                "开关": "关"
            }
        };
    }

    /**
     * 获取当前设备状态
     */
    getCurrentDeviceState(sessionId) {
        if (!this.deviceStates.has(sessionId)) {
            this.deviceStates.set(sessionId, this.getDefaultDeviceState());
        }
        return this.deviceStates.get(sessionId);
    }

    /**
     * 更新设备状态
     */
    updateDeviceState(sessionId, newState) {
        const currentState = this.getCurrentDeviceState(sessionId);

        // 合并新状态到当前状态
        if (newState.风扇) {
            Object.assign(currentState.风扇, newState.风扇);
        }
        if (newState.LED灯) {
            Object.assign(currentState.LED灯, newState.LED灯);
        }
        if (newState.窗帘) {
            Object.assign(currentState.窗帘, newState.窗帘);
        }

        this.deviceStates.set(sessionId, currentState);
    }

    /**
     * 发送聊天请求（流式输出）
     * @param {String} userMessage - 用户消息
     * @param {String} sessionId - 会话ID（用于保存上下文）
     * @param {Function} onChunk - 接收流式数据块的回调函数
     * @param {Function} onComplete - 完成时的回调
     * @param {Function} onError - 错误处理回调
     */
    sendChatStream(userMessage, sessionId = 'default', onChunk, onComplete, onError) {
        // 获取或初始化会话历史
        if (!this.conversationHistory.has(sessionId)) {
            this.conversationHistory.set(sessionId, []);
        }

        const messages = this.conversationHistory.get(sessionId);
        const currentDeviceState = this.getCurrentDeviceState(sessionId);

        // 如果是新会话，添加系统提示词
        if (messages.length === 0) {
            messages.push({
                role: 'system',
                content: `你是一位专业、贴心的智能家居管家AI助手，名字叫小月。请用温和、亲切的语气与用户交流，就像一位贴心的管家一样。

**你的性格特点：**
- 称呼用户为"老板"或"主人"
- 语气温和、礼貌、专业
- 主动关心用户的需求和舒适度
- 对用户的指令积极响应

**对话风格示例：**
- 用户说"你好" → 回复"您好，老板！很高兴为您服务，有什么可以帮助您的吗？"
- 用户说"谢谢" → 回复"不客气，老板！能为您服务是我的荣幸。"
- 用户说"晚安" → 回复"晚安，老板！祝您美梦，如需要我会随时待命。"

**设备控制功能：**
当用户需要控制家居设备时，你可以控制：
1. 风扇：开关状态和风速级别（开关："开|关"，风速：1-5档）
2. LED灯：开关、色温和亮度（开关："开|关"，色温："冷光|暖光"，亮度：0-100）
3. 窗帘：开关状态（开关："开|关"，开=打开窗帘，关=关闭窗帘）

**判断规则：**
1. 当用户明确要求控制设备时（如"打开灯"、"调高风扇"、"我要睡觉了"），**必须只返回纯JSON格式控制指令，不能有任何其他文字、说明或前缀**
2. 当用户只是普通聊天、问候、提问时，正常回复聊天内容
3. 基于当前设备状态进行相对调整

**重要：设备控制时的返回格式要求：**
- 必须以 { 开头，以 } 结尾
- 不能有任何JSON前的文字说明
- 不能有任何JSON后的补充内容
- 只返回纯JSON，没有其他任何内容

**控制设备时的JSON格式：**
{
  "风扇": {"开关": "开|关", "风速": 1-5, "理由": "简要理由"},
  "LED灯": {"开关": "开|关", "色温": "冷光|暖光", "亮度": 0-100, "理由": "简要理由"},
  "窗帘": {"开关": "开|关", "理由": "简要理由"},
  "综合说明": "环境调整说明(20字以内)"
}

**聊天示例：**
- "你好" → "您好，老板！很高兴为您服务，有什么可以帮助您的吗？"
- "今天天气怎么样？" → "老板，今天天气晴朗，温度适宜，是个不错的日子呢！您今天有什么安排吗？"
- "我累了" → "老板您辛苦了！需要我为您调整一下环境让您休息吗？比如调暗灯光或者打开风扇？"

**控制示例：**
- "打开灯" → 只返回纯JSON，例如：{"LED灯": {"开关": "开", "理由": "为您点亮灯光"}}
- "我要睡觉了" → 只返回纯JSON，例如：{"LED灯": {"开关": "关", "理由": "营造睡眠环境"}, "综合说明": "已为您调整睡眠环境"}

**错误示例（禁止这样回复）：**
❌ 您好，老板！理解您现在的感受。我建议... {"风扇": {...}}
❌ 好的，我来为您调整。 {"LED灯": {...}}

**正确示例（必须这样回复）：**
✅ {"风扇": {"开关": "开", "风速": 2, "理由": "开启风扇通风"}}
✅ {"LED灯": {"开关": "开", "色温": "暖光", "亮度": 70, "理由": "为您点亮灯光"}}

记住：控制设备时，整个回复就是一个JSON对象，不能有任何额外文字！`
            });
        }

        // 添加用户消息，同时附带当前设备状态
        messages.push({
            role: 'user',
            content: `当前设备状态：
${JSON.stringify(currentDeviceState, null, 2)}

用户指令：${userMessage}`
        });

        // 限制历史记录长度（保留最近10轮对话）
        if (messages.length > 20) {
            messages.splice(0, messages.length - 20);
        }

        const requestBody = JSON.stringify({
            model: this.model,
            messages: messages,
            stream: true,
            temperature: 0.75,
            top_p: 0.9,
            do_sample: true
        });

        const options = {
            hostname: this.baseURL,
            port: 443,
            path: this.apiPath,
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${this.apiKey}`,
                'Content-Length': Buffer.byteLength(requestBody)
            }
        };

        let fullResponse = '';

        const req = https.request(options, (res) => {
            // 检查HTTP状态码
            if (res.statusCode !== 200) {
                let errorData = '';
                res.on('data', chunk => errorData += chunk);
                res.on('end', () => {
                    onError(new Error(`API请求失败: HTTP ${res.statusCode} - ${errorData}`));
                });
                return;
            }

            res.setEncoding('utf8');

            let buffer = '';
            res.on('data', (chunk) => {
                buffer += chunk;
                const lines = buffer.split('\n');
                buffer = lines.pop(); // 保留不完整的行

                for (const line of lines) {
                    if (line.startsWith('data: ')) {
                        const data = line.slice(6).trim();

                        if (data === '[DONE]') {
                            // 流式输出结束
                            messages.push({
                                role: 'assistant',
                                content: fullResponse
                            });

                            // 尝试解析JSON并更新设备状态
                            try {
                                const deviceControl = JSON.parse(fullResponse);
                                this.updateDeviceState(sessionId, deviceControl);

                                // 🔥 触发设备控制回调，通知UnifiedServer广播命令
                                if (this.onDeviceControl && typeof this.onDeviceControl === 'function') {
                                    this.onDeviceControl(deviceControl);
                                }
                            } catch (e) {
                                // 不是JSON，说明是普通聊天，不更新设备状态
                                console.log('AI返回普通聊天内容，不更新设备状态');
                            }

                            onComplete(fullResponse);
                            return;
                        }

                        try {
                            const parsed = JSON.parse(data);
                            const delta = parsed.choices?.[0]?.delta;

                            if (delta && delta.content) {
                                fullResponse += delta.content;
                                onChunk(delta.content);
                            }
                        } catch (e) {
                            // 忽略JSON解析错误（可能是不完整的数据）
                            console.warn('JSON解析警告:', e.message);
                        }
                    }
                }
            });

            res.on('end', () => {
                if (fullResponse) {
                    messages.push({
                        role: 'assistant',
                        content: fullResponse
                    });
                }
                onComplete(fullResponse);
            });

            res.on('error', (error) => {
                onError(error);
            });
        });

        req.on('error', (error) => {
            onError(error);
        });

        req.write(requestBody);
        req.end();
    }

    /**
     * 发送聊天请求（非流式输出）
     * @param {String} userMessage - 用户消息
     * @param {String} sessionId - 会话ID
     * @returns {Promise<String>} - AI回复内容
     */
    async sendChat(userMessage, sessionId = 'default') {
        return new Promise((resolve, reject) => {
            if (!this.conversationHistory.has(sessionId)) {
                this.conversationHistory.set(sessionId, []);
            }

            const messages = this.conversationHistory.get(sessionId);
            const currentDeviceState = this.getCurrentDeviceState(sessionId);

            // 如果是新会话，添加系统提示词
            if (messages.length === 0) {
                messages.push({
                    role: 'system',
                    content: `你是一位专业、贴心的智能家居管家AI助手，名字叫小月。请用温和、亲切的语气与用户交流，就像一位贴心的管家一样。

**你的性格特点：**
- 称呼用户为"老板"或"主人"
- 语气温和、礼貌、专业
- 主动关心用户的需求和舒适度
- 对用户的指令积极响应

**对话风格示例：**
- 用户说"你好" → 回复"您好，老板！很高兴为您服务，有什么可以帮助您的吗？"
- 用户说"谢谢" → 回复"不客气，老板！能为您服务是我的荣幸。"
- 用户说"晚安" → 回复"晚安，老板！祝您美梦，如需要我会随时待命。"

**设备控制功能：**
当用户需要控制家居设备时，你可以控制：
1. 风扇：开关状态和风速级别（开关："开|关"，风速：1-5档）
2. LED灯：开关、色温和亮度（开关："开|关"，色温："冷光|暖光"，亮度：0-100）
3. 窗帘：开关状态（开关："开|关"，开=打开窗帘，关=关闭窗帘）

**判断规则：**
1. 当用户明确要求控制设备时（如"打开灯"、"调高风扇"、"我要睡觉了"），**必须只返回纯JSON格式控制指令，不能有任何其他文字、说明或前缀**
2. 当用户只是普通聊天、问候、提问时，正常回复聊天内容
3. 基于当前设备状态进行相对调整

**重要：设备控制时的返回格式要求：**
- 必须以 { 开头，以 } 结尾
- 不能有任何JSON前的文字说明
- 不能有任何JSON后的补充内容
- 只返回纯JSON，没有其他任何内容

**控制设备时的JSON格式：**
{
  "风扇": {"开关": "开|关", "风速": 1-5, "理由": "简要理由"},
  "LED灯": {"开关": "开|关", "色温": "冷光|暖光", "亮度": 0-100, "理由": "简要理由"},
  "窗帘": {"开关": "开|关", "理由": "简要理由"},
  "综合说明": "环境调整说明(20字以内)"
}

**聊天示例：**
- "你好" → "您好，老板！很高兴为您服务，有什么可以帮助您的吗？"
- "今天天气怎么样？" → "老板，今天天气晴朗，温度适宜，是个不错的日子呢！您今天有什么安排吗？"
- "我累了" → "老板您辛苦了！需要我为您调整一下环境让您休息吗？比如调暗灯光或者打开风扇？"

**控制示例：**
- "打开灯" → 只返回纯JSON，例如：{"LED灯": {"开关": "开", "理由": "为您点亮灯光"}}
- "我要睡觉了" → 只返回纯JSON，例如：{"LED灯": {"开关": "关", "理由": "营造睡眠环境"}, "综合说明": "已为您调整睡眠环境"}

**错误示例（禁止这样回复）：**
❌ 您好，老板！理解您现在的感受。我建议... {"风扇": {...}}
❌ 好的，我来为您调整。 {"LED灯": {...}}

**正确示例（必须这样回复）：**
✅ {"风扇": {"开关": "开", "风速": 2, "理由": "开启风扇通风"}}
✅ {"LED灯": {"开关": "开", "色温": "暖光", "亮度": 70, "理由": "为您点亮灯光"}}

记住：控制设备时，整个回复就是一个JSON对象，不能有任何额外文字！`
                });
            }

            // 添加用户消息，同时附带当前设备状态
            messages.push({
                role: 'user',
                content: `当前设备状态：
${JSON.stringify(currentDeviceState, null, 2)}

用户指令：${userMessage}`
            });

            // 限制历史记录长度
            if (messages.length > 20) {
                messages.splice(0, messages.length - 20);
            }

            const requestBody = JSON.stringify({
                model: this.model,
                messages: messages,
                stream: false,
                temperature: 0.75,
                top_p: 0.9
            });

            const options = {
                hostname: this.baseURL,
                port: 443,
                path: this.apiPath,
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${this.apiKey}`,
                    'Content-Length': Buffer.byteLength(requestBody)
                }
            };

            const req = https.request(options, (res) => {
                let data = '';

                res.on('data', (chunk) => {
                    data += chunk;
                });

                res.on('end', () => {
                    if (res.statusCode !== 200) {
                        reject(new Error(`API请求失败: HTTP ${res.statusCode} - ${data}`));
                        return;
                    }

                    try {
                        const parsed = JSON.parse(data);
                        const content = parsed.choices?.[0]?.message?.content;

                        if (content) {
                            messages.push({
                                role: 'assistant',
                                content: content
                            });

                            // 尝试解析JSON并更新设备状态
                            try {
                                const deviceControl = JSON.parse(content);
                                this.updateDeviceState(sessionId, deviceControl);

                                // 🔥 触发设备控制回调，通知UnifiedServer广播命令
                                if (this.onDeviceControl && typeof this.onDeviceControl === 'function') {
                                    this.onDeviceControl(deviceControl);
                                }
                            } catch (e) {
                                // 不是JSON，说明是普通聊天，不更新设备状态
                                console.log('AI返回普通聊天内容，不更新设备状态');
                            }

                            resolve(content);
                        } else {
                            reject(new Error('API返回数据格式错误'));
                        }
                    } catch (e) {
                        reject(new Error(`JSON解析失败: ${e.message}`));
                    }
                });
            });

            req.on('error', (error) => {
                reject(error);
            });

            req.write(requestBody);
            req.end();
        });
    }

    /**
     * 清除会话历史
     * @param {String} sessionId - 会话ID
     */
    clearHistory(sessionId = 'default') {
        this.conversationHistory.delete(sessionId);
    }

    /**
     * 获取会话历史
     * @param {String} sessionId - 会话ID
     * @returns {Array} - 消息历史
     */
    getHistory(sessionId = 'default') {
        return this.conversationHistory.get(sessionId) || [];
    }

    /**
     * 设置模型
     * @param {String} model - 模型名称 (glm-4-plus, glm-4-air, glm-4-flash等)
     */
    setModel(model) {
        this.model = model;
    }

    /**
     * 验证API Key是否有效
     * @returns {Promise<Boolean>}
     */
    async validateApiKey() {
        try {
            await this.sendChat('你好', 'test_validation');
            this.clearHistory('test_validation');
            return true;
        } catch (error) {
            return false;
        }
    }
}

module.exports = AIService;
