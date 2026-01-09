/**
 * 手势和语音交互管理器
 * 处理手势识别结果和语音唤醒控制
 */

class GestureVoiceManager {
    constructor() {
        // 手势配置
        this.gestureEnabled = true;
        this.lastGesture = null;
        this.gestureCallbacks = [];

        // 语音配置
        this.voiceEnabled = false;
        this.isListening = false;
        this.recognition = null;
        this.wakeWord = '小月';
        this.isAwake = false;
        this.awakeTimeout = null;
        this.awakeTimeoutMs = 10000; // 唤醒后保持10秒活跃

        // 设备控制回调
        this.onDeviceControl = null;

        // 手势到设备的映射
        this.gestureActionMap = {
            'point_up': { device: 'LED灯', action: { '开关': '开', '亮度': 70, '色温': '暖光' }, label: '👆 指向上 → 开灯' },
            'victory': { device: '风扇', action: { '开关': '开', '风速': 3 }, label: '✌️ V手势 → 开风扇' },
            'open_palm': { device: 'all', action: { '开关': '关' }, label: '👋 张开手 → 全部关闭' },
            'thumbs_up': { device: '窗帘', action: { '开关': '开' }, label: '👍 竖拇指 → 开窗帘' },
            'fist': { device: 'security', action: { mode: 'armed' }, label: '✊ 握拳 → 安防模式' },
            'rock': { device: 'scene', action: { scene: 'party' }, label: '🤟 摇滚 → 派对模式' }
        };

        // UI元素
        this.statusIndicator = null;
        this.gestureDisplay = null;
        this.voiceButton = null;
    }

    /**
     * 初始化管理器
     */
    init(options = {}) {
        // 创建UI元素
        this.createUI();

        // 初始化语音识别
        this.initSpeechRecognition();

        // 设置回调
        if (options.onDeviceControl) {
            this.onDeviceControl = options.onDeviceControl;
        }

        console.log('✅ GestureVoiceManager 初始化完成');
        console.log('   - 手势识别: 启用');
        console.log('   - 语音唤醒: ' + (this.recognition ? '可用' : '不可用'));
    }

    /**
     * 创建控制UI
     * NOTE: 已禁用 - 使用摄像头视觉检测页面进行手势控制
     */
    createUI() {
        // 2026-01-09: 禁用浮动面板，改用视觉检测页面的摄像头进行交互
        // 如需恢复，删除下面的 return 语句
        console.log('ℹ️ 智能交互控制面板已禁用，使用摄像头进行手势控制');
        return;

        // 创建浮动控制面板
        const panel = document.createElement('div');
        panel.id = 'gesture-voice-panel';
        panel.innerHTML = `
            <style>
                #gesture-voice-panel {
                    position: fixed;
                    top: 20px;
                    right: 20px;
                    width: 280px;
                    background: linear-gradient(135deg, rgba(30, 30, 50, 0.95), rgba(20, 20, 40, 0.98));
                    border: 1px solid rgba(100, 200, 255, 0.3);
                    border-radius: 16px;
                    padding: 16px;
                    z-index: 10000;
                    font-family: 'Segoe UI', Arial, sans-serif;
                    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4), 0 0 20px rgba(100, 200, 255, 0.1);
                    backdrop-filter: blur(10px);
                }
                
                #gesture-voice-panel .panel-header {
                    display: flex;
                    align-items: center;
                    justify-content: space-between;
                    margin-bottom: 12px;
                    padding-bottom: 12px;
                    border-bottom: 1px solid rgba(100, 200, 255, 0.2);
                }
                
                #gesture-voice-panel .panel-title {
                    color: #64d8ff;
                    font-size: 14px;
                    font-weight: 600;
                    display: flex;
                    align-items: center;
                    gap: 8px;
                }
                
                #gesture-voice-panel .status-dot {
                    width: 8px;
                    height: 8px;
                    border-radius: 50%;
                    background: #4ade80;
                    animation: pulse 2s infinite;
                }
                
                @keyframes pulse {
                    0%, 100% { opacity: 1; transform: scale(1); }
                    50% { opacity: 0.5; transform: scale(1.2); }
                }
                
                #gesture-voice-panel .gesture-display {
                    background: rgba(0, 0, 0, 0.3);
                    border-radius: 12px;
                    padding: 16px;
                    text-align: center;
                    margin-bottom: 12px;
                    min-height: 60px;
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    justify-content: center;
                }
                
                #gesture-voice-panel .gesture-icon {
                    font-size: 32px;
                    margin-bottom: 8px;
                }
                
                #gesture-voice-panel .gesture-label {
                    color: #e0e0e0;
                    font-size: 13px;
                }
                
                #gesture-voice-panel .gesture-action {
                    color: #4ade80;
                    font-size: 12px;
                    margin-top: 4px;
                }
                
                #gesture-voice-panel .voice-section {
                    display: flex;
                    align-items: center;
                    gap: 12px;
                }
                
                #gesture-voice-panel .voice-btn {
                    flex: 1;
                    background: linear-gradient(135deg, #3b82f6, #1d4ed8);
                    border: none;
                    border-radius: 10px;
                    padding: 12px;
                    color: white;
                    font-size: 13px;
                    cursor: pointer;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    gap: 8px;
                    transition: all 0.3s ease;
                }
                
                #gesture-voice-panel .voice-btn:hover {
                    transform: translateY(-2px);
                    box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
                }
                
                #gesture-voice-panel .voice-btn.listening {
                    background: linear-gradient(135deg, #ef4444, #dc2626);
                    animation: listening-pulse 1s infinite;
                }
                
                @keyframes listening-pulse {
                    0%, 100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.4); }
                    50% { box-shadow: 0 0 0 10px rgba(239, 68, 68, 0); }
                }
                
                #gesture-voice-panel .voice-btn.awake {
                    background: linear-gradient(135deg, #22c55e, #16a34a);
                }
                
                #gesture-voice-panel .voice-status {
                    color: #9ca3af;
                    font-size: 11px;
                    text-align: center;
                    margin-top: 8px;
                }
                
                #gesture-voice-panel .minimize-btn {
                    background: none;
                    border: none;
                    color: #9ca3af;
                    cursor: pointer;
                    font-size: 16px;
                    padding: 4px 8px;
                    border-radius: 4px;
                    transition: all 0.2s;
                }
                
                #gesture-voice-panel .minimize-btn:hover {
                    background: rgba(255, 255, 255, 0.1);
                    color: white;
                }
                
                #gesture-voice-panel.minimized .panel-content {
                    display: none;
                }
            </style>
            
            <div class="panel-header">
                <div class="panel-title">
                    <span class="status-dot"></span>
                    <span>🤖 智能交互控制</span>
                </div>
                <button class="minimize-btn" onclick="this.closest('#gesture-voice-panel').classList.toggle('minimized')">−</button>
            </div>
            
            <div class="panel-content">
                <div class="gesture-display" id="gesture-display">
                    <div class="gesture-icon">👋</div>
                    <div class="gesture-label">等待手势...</div>
                    <div class="gesture-action">对着摄像头做手势</div>
                </div>
                
                <div class="voice-section">
                    <button class="voice-btn" id="voice-btn">
                        <span>🎙️</span>
                        <span>点击说话</span>
                    </button>
                </div>
                
                <div class="voice-status" id="voice-status">
                    说 "小月" 唤醒语音助手
                </div>
            </div>
        `;

        document.body.appendChild(panel);

        // 获取元素引用
        this.gestureDisplay = document.getElementById('gesture-display');
        this.voiceButton = document.getElementById('voice-btn');
        this.voiceStatus = document.getElementById('voice-status');

        // 绑定事件
        this.voiceButton.addEventListener('click', () => this.toggleVoice());
    }

    /**
     * 初始化语音识别 (Web Speech API)
     */
    initSpeechRecognition() {
        // 检查浏览器支持
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;

        if (!SpeechRecognition) {
            console.warn('⚠️ 浏览器不支持语音识别');
            this.voiceStatus.textContent = '浏览器不支持语音识别';
            this.voiceButton.disabled = true;
            return;
        }

        this.recognition = new SpeechRecognition();
        this.recognition.continuous = true;
        this.recognition.interimResults = true;
        this.recognition.lang = 'zh-CN';

        // 语音识别结果
        this.recognition.onresult = (event) => {
            let finalTranscript = '';
            let interimTranscript = '';

            for (let i = event.resultIndex; i < event.results.length; i++) {
                const transcript = event.results[i][0].transcript;
                if (event.results[i].isFinal) {
                    finalTranscript += transcript;
                } else {
                    interimTranscript = transcript;
                }
            }

            // 显示识别中的文字
            if (interimTranscript) {
                this.voiceStatus.textContent = `识别中: ${interimTranscript}`;
            }

            // 处理最终结果
            if (finalTranscript) {
                console.log('🎙️ 语音识别结果:', finalTranscript);
                this.handleVoiceCommand(finalTranscript);
            }
        };

        this.recognition.onerror = (event) => {
            console.error('语音识别错误:', event.error);
            this.voiceStatus.textContent = `识别错误: ${event.error}`;
            this.setListeningState(false);
        };

        this.recognition.onend = () => {
            if (this.isListening) {
                // 自动重新开始
                this.recognition.start();
            }
        };

        console.log('✅ 语音识别初始化成功');
    }

    /**
     * 切换语音监听状态
     */
    toggleVoice() {
        if (this.isListening) {
            this.stopListening();
        } else {
            this.startListening();
        }
    }

    /**
     * 开始语音监听
     */
    startListening() {
        if (!this.recognition) return;

        try {
            this.recognition.start();
            this.setListeningState(true);
            this.voiceStatus.textContent = '正在监听...说 "小月" 唤醒';
            console.log('🎙️ 开始语音监听');
        } catch (error) {
            console.error('启动语音识别失败:', error);
        }
    }

    /**
     * 停止语音监听
     */
    stopListening() {
        if (!this.recognition) return;

        this.isListening = false;
        this.recognition.stop();
        this.setListeningState(false);
        this.voiceStatus.textContent = '点击按钮开始监听';
        console.log('🎙️ 停止语音监听');
    }

    /**
     * 设置监听状态UI
     */
    setListeningState(listening) {
        this.isListening = listening;
        this.voiceButton.classList.toggle('listening', listening);
        this.voiceButton.innerHTML = listening
            ? '<span>🔴</span><span>正在监听...</span>'
            : '<span>🎙️</span><span>点击说话</span>';
    }

    /**
     * 处理语音指令
     */
    handleVoiceCommand(text) {
        const lowerText = text.toLowerCase();

        // 检查唤醒词
        if (lowerText.includes('小月') || lowerText.includes('小岳')) {
            this.activateWakeMode();
            this.voiceStatus.textContent = '✅ 已唤醒！请说指令...';
            return;
        }

        // 如果已唤醒，处理指令
        if (this.isAwake) {
            this.processVoiceCommand(text);
        } else {
            this.voiceStatus.textContent = `识别: "${text}" (说"小月"唤醒)`;
        }
    }

    /**
     * 激活唤醒模式
     */
    activateWakeMode() {
        this.isAwake = true;
        this.voiceButton.classList.add('awake');

        // 重置超时
        if (this.awakeTimeout) {
            clearTimeout(this.awakeTimeout);
        }

        this.awakeTimeout = setTimeout(() => {
            this.deactivateWakeMode();
        }, this.awakeTimeoutMs);

        console.log('🔔 语音助手已唤醒');
    }

    /**
     * 退出唤醒模式
     */
    deactivateWakeMode() {
        this.isAwake = false;
        this.voiceButton.classList.remove('awake');
        this.voiceStatus.textContent = '说 "小月" 唤醒语音助手';
        console.log('💤 语音助手已休眠');
    }

    /**
     * 处理语音控制指令
     */
    processVoiceCommand(text) {
        console.log('📝 处理语音指令:', text);

        // 重置唤醒超时
        this.activateWakeMode();

        // 简单的指令匹配
        const commands = [
            { keywords: ['开灯', '打开灯', '灯开'], action: { device: 'LED灯', action: { '开关': '开' } } },
            { keywords: ['关灯', '关闭灯', '灯关'], action: { device: 'LED灯', action: { '开关': '关' } } },
            { keywords: ['开风扇', '打开风扇'], action: { device: '风扇', action: { '开关': '开', '风速': 3 } } },
            { keywords: ['关风扇', '关闭风扇'], action: { device: '风扇', action: { '开关': '关' } } },
            { keywords: ['开窗帘', '打开窗帘'], action: { device: '窗帘', action: { '开关': '开' } } },
            { keywords: ['关窗帘', '关闭窗帘'], action: { device: '窗帘', action: { '开关': '关' } } },
            { keywords: ['全部关闭', '全关', '都关了'], action: { device: 'all', action: { '开关': '关' } } },
            { keywords: ['太热', '好热', '热死了'], action: { device: '风扇', action: { '开关': '开', '风速': 5 } } },
            { keywords: ['太亮', '好亮'], action: { device: 'LED灯', action: { '亮度': 30 } } },
            { keywords: ['太暗', '好暗'], action: { device: 'LED灯', action: { '亮度': 100 } } }
        ];

        for (const cmd of commands) {
            if (cmd.keywords.some(kw => text.includes(kw))) {
                this.voiceStatus.textContent = `✅ 执行: ${text}`;
                this.triggerDeviceControl(cmd.action);
                return;
            }
        }

        // 未匹配到指令，可以转给AI处理
        this.voiceStatus.textContent = `🤔 发送到AI: "${text}"`;
        // 可以调用AI对话接口处理更复杂的指令
    }

    /**
     * 处理手势控制消息（从WebSocket接收）
     */
    handleGestureMessage(message) {
        if (!this.gestureEnabled) return;

        const { gesture, confidence, action } = message.data;

        console.log(`✋ 收到手势: ${gesture} (置信度: ${confidence})`);

        // 更新UI
        this.updateGestureDisplay(gesture, confidence);

        // 触发设备控制
        if (action && action.device) {
            this.triggerDeviceControl(action);
        }

        // 触发回调
        this.gestureCallbacks.forEach(cb => cb(gesture, confidence, action));
    }

    /**
     * 更新手势显示
     */
    updateGestureDisplay(gesture, confidence) {
        const gestureInfo = this.gestureActionMap[gesture];

        if (gestureInfo) {
            const icons = {
                'point_up': '👆',
                'victory': '✌️',
                'open_palm': '👋',
                'thumbs_up': '👍',
                'fist': '✊',
                'rock': '🤟'
            };

            this.gestureDisplay.innerHTML = `
                <div class="gesture-icon">${icons[gesture] || '❓'}</div>
                <div class="gesture-label">${gestureInfo.label}</div>
                <div class="gesture-action">置信度: ${(confidence * 100).toFixed(0)}%</div>
            `;

            // 添加动画效果
            this.gestureDisplay.style.animation = 'none';
            setTimeout(() => {
                this.gestureDisplay.style.animation = 'pulse 0.5s ease';
            }, 10);
        }

        this.lastGesture = gesture;
    }

    /**
     * 触发设备控制
     */
    triggerDeviceControl(actionData) {
        console.log('🎮 触发设备控制:', actionData);

        if (this.onDeviceControl && typeof this.onDeviceControl === 'function') {
            this.onDeviceControl(actionData);
        }

        // 也可以发送WebSocket消息到ESP32
        this.sendDeviceCommand(actionData);
    }

    /**
     * 发送设备控制命令
     */
    sendDeviceCommand(actionData) {
        // 构建设备控制JSON（与AIService格式兼容）
        const deviceControl = {};

        if (actionData.device === 'all') {
            // 全部设备
            deviceControl['LED灯'] = { '开关': '关', '理由': '手势/语音控制' };
            deviceControl['风扇'] = { '开关': '关', '风速': 0, '理由': '手势/语音控制' };
            deviceControl['窗帘'] = { '开关': '关', '理由': '手势/语音控制' };
        } else if (actionData.device && actionData.action) {
            deviceControl[actionData.device] = {
                ...actionData.action,
                '理由': '手势/语音控制'
            };
        }

        deviceControl['综合说明'] = actionData.device === 'all' ? '已关闭所有设备' : `已控制${actionData.device}`;

        // 通过全局WebSocket发送（如果可用）
        if (window.wsConnection && window.wsConnection.readyState === WebSocket.OPEN) {
            window.wsConnection.send(JSON.stringify({
                type: 'device_control',
                timestamp: Date.now(),
                data: deviceControl
            }));
        }
    }

    /**
     * 注册手势回调
     */
    onGesture(callback) {
        this.gestureCallbacks.push(callback);
    }

    /**
     * 设置手势启用状态
     */
    setGestureEnabled(enabled) {
        this.gestureEnabled = enabled;
        console.log(`手势识别: ${enabled ? '启用' : '禁用'}`);
    }
}

// 导出全局实例
window.gestureVoiceManager = new GestureVoiceManager();
