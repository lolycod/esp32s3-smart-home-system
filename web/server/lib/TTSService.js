/**
 * 讯飞语音合成服务
 * 提供文字转语音功能，通过WebSocket连接讯飞TTS API
 */

const WebSocket = require('ws');
const crypto = require('crypto');

class TTSService {
    constructor(appId, apiKey, apiSecret) {
        this.appId = appId || 'b5c17f79'; // 讯飞APPID
        this.apiKey = apiKey || 'e98653e4b07e91c2a7e0ae8e0a76088d'; // 讯飞APIKey
        this.apiSecret = apiSecret || 'Mzg1NjA1NWE5MTkzOGM4ZmE2MTdhNzM5'; // 讯飞APISecret
        this.hostUrl = 'wss://tts-api.xfyun.cn/v2/tts';

        // 音色配置
        this.voices = {
            'aisxping': { name: '小萍', gender: '女声', style: '温柔' },
            'aisjiuxu': { name: '小旭', gender: '男声', style: '温暖' },
            'aisxlin': { name: '小琳', gender: '女声', style: '甜美' },
            'aisjinger': { name: '小静', gender: '女声', style: '知性' },
            'aisbabyxu': { name: '小许', gender: '男声', style: '青春' },
            'aistom': { name: 'Tom', gender: '男声', style: '英文' }
        };

        // 默认音色
        this.defaultVoice = 'aisxping';
    }

    /**
     * 获取可用音色列表
     * @returns {Object} 音色列表和描述
     */
    getVoiceList() {
        return {
            voices: this.voices,
            default: this.defaultVoice
        };
    }

    /**
     * 验证音色是否有效
     * @param {string} voice - 音色ID
     * @returns {boolean} 是否有效
     */
    isValidVoice(voice) {
        return voice && this.voices.hasOwnProperty(voice);
    }

    /**
     * 生成鉴权URL
     * @returns {string} 鉴权后的WebSocket URL
     */
    getAuthUrl() {
        const { URL } = require('url');
        const urlParsed = new URL(this.hostUrl);
        const host = urlParsed.host;
        const path = urlParsed.pathname;

        // 生成RFC1123格式的date
        const date = new Date().toUTCString();

        // 拼接signature_origin
        const signatureOrigin = `host: ${host}\ndate: ${date}\nGET ${path} HTTP/1.1`;

        // 使用hmac-sha256加密
        const signature = crypto.createHmac('sha256', this.apiSecret)
            .update(signatureOrigin)
            .digest('base64');

        // 拼接authorization_origin
        const authorizationOrigin = `api_key="${this.apiKey}", algorithm="hmac-sha256", headers="host date request-line", signature="${signature}"`;

        // base64编码authorization
        const authorization = Buffer.from(authorizationOrigin).toString('base64');

        // 拼接最终URL
        return `${this.hostUrl}?authorization=${encodeURIComponent(authorization)}&date=${encodeURIComponent(date)}&host=${encodeURIComponent(host)}`;
    }

    /**
     * 文字转语音
     * @param {string} text - 要合成的文字
     * @param {function} onAudioChunk - 接收音频数据的回调函数
     * @param {function} onComplete - 合成完成的回调
     * @param {function} onError - 错误回调
     * @param {string} voice - 音色ID (可选)
     */
    synthesize(text, onAudioChunk, onComplete, onError, voice = null) {
        const authUrl = this.getAuthUrl();
        const ws = new WebSocket(authUrl);

        ws.on('open', () => {
            console.log('✅ TTS WebSocket连接成功');

            // 选择音色
            const selectedVoice = voice && this.voices[voice] ? voice : this.defaultVoice;

            // 构建请求参数
            const params = {
                common: {
                    app_id: this.appId
                },
                business: {
                    aue: "raw",           // 音频编码: raw=PCM
                    auf: "audio/L16;rate=16000", // 16k采样率
                    vcn: selectedVoice,   // 发音人: 动态选择
                    speed: 50,            // 语速: 50(默认)
                    volume: 80,           // 音量: 80
                    pitch: 50,            // 音高: 50(默认)
                    tte: "UTF8"           // 文本编码: UTF8
                },
                data: {
                    status: 2,            // 固定为2
                    text: Buffer.from(text).toString('base64') // base64编码文本
                }
            };

            console.log(`🎵 使用音色: ${selectedVoice} (${this.voices[selectedVoice]?.name})`);

            // 发送请求
            ws.send(JSON.stringify(params));
        });

        ws.on('message', (data) => {
            try {
                const response = JSON.parse(data);

                if (response.code !== 0) {
                    console.error('❌ TTS合成错误:', response.message);
                    onError(new Error(`TTS合成失败: ${response.message}`));
                    ws.close();
                    return;
                }

                // 接收音频数据
                if (response.data && response.data.audio) {
                    const audioData = Buffer.from(response.data.audio, 'base64');
                    onAudioChunk(audioData);
                }

                // 合成完成
                if (response.data && response.data.status === 2) {
                    console.log('✅ TTS合成完成');
                    onComplete();
                    ws.close();
                }
            } catch (e) {
                console.error('❌ TTS解析错误:', e);
                onError(e);
                ws.close();
            }
        });

        ws.on('error', (error) => {
            console.error('❌ TTS WebSocket错误:', error);
            onError(error);
        });

        ws.on('close', () => {
            console.log('🔌 TTS WebSocket连接关闭');
        });
    }

    /**
     * 流式合成(用于HTTP接口)
     * @param {string} text - 要合成的文字
     * @param {object} response - HTTP Response对象
     */
    synthesizeStream(text, response, voice = null) {
        return new Promise((resolve, reject) => {
            const chunks = [];

            this.synthesize(
                text,
                // onAudioChunk
                (audioData) => {
                    chunks.push(audioData);
                },
                // onComplete
                () => {
                    const audioBuffer = Buffer.concat(chunks);
                    resolve(audioBuffer);
                },
                // onError
                (error) => {
                    reject(error);
                },
                // voice
                voice
            );
        });
    }
}

module.exports = TTSService;
