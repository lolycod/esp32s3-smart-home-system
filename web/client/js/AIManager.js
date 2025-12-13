/**
 * AI对话管理器
 * 处理与后端AI服务的交互，管理对话历史和UI渲染
 */

class AIManager {
    constructor() {
        // 修复API基础URL，支持本地开发
        this.apiBaseUrl = window.location.protocol === 'file:'
            ? 'http://localhost:8080'
            : window.location.origin;
        this.sessionId = this.generateSessionId();
        this.isProcessing = false;

        // UI元素引用
        this.chatContainer = null;
        this.inputBox = null;
        this.sendButton = null;
        this.queryStateButton = null;
        this.ttsToggleButton = null;
        this.clearButton = null;
        this.voiceSelect = null;
        this.voicePreviewButton = null;
        this.voiceToggleButton = null;

        // 语音功能
        this.ttsClient = null;
        this.ttsEnabled = false;
        this.selectedVoice = '';
        this.voiceList = {};
        this.voiceEnabled = true; // 音色功能开关
    }

    /**
     * 初始化AI对话管理器
     * @param {Object} elements - UI元素
     */
    init(elements) {
        this.chatContainer = elements.chatContainer;
        this.inputBox = elements.inputBox;
        this.sendButton = elements.sendButton;
        this.queryStateButton = elements.queryStateButton;
        this.ttsToggleButton = elements.ttsToggleButton;
        this.clearButton = elements.clearButton;
        this.voiceSelect = elements.voiceSelect;
        this.voicePreviewButton = elements.voicePreviewButton;
        this.voiceToggleButton = elements.voiceToggleButton;

        // 初始化语音客户端
        this.ttsClient = new TTSClient(this.apiBaseUrl);

        // 加载音色列表
        this.loadVoiceList();

        // 从本地存储加载设置
        this.loadSettings();

        // 绑定事件
        this.bindEvents();

        // 更新UI状态
        this.updateVoiceToggleButton();
        this.updateVoiceControlVisibility();

        // 显示欢迎消息
        this.addSystemMessage('老板您好，我是您的AI机器人小月，很高兴为您服务');

        console.log('✅ AIManager初始化完成');
    }

    /**
     * 绑定UI事件
     */
    bindEvents() {
        // 发送按钮点击
        this.sendButton.addEventListener('click', () => {
            this.sendMessage();
        });

        // 回车键发送
        this.inputBox.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' && !e.shiftKey) {
                e.preventDefault();
                this.sendMessage();
            }
        });

        // 查询设备状态
        this.queryStateButton.addEventListener('click', () => {
            this.queryDeviceState();
        });

        // 语音开关
        this.ttsToggleButton.addEventListener('click', () => {
            this.toggleTTS();
        });

        // 清除对话
        this.clearButton.addEventListener('click', () => {
            this.clearChat();
        });

        // 音色选择器事件绑定
        this.updateVoiceBindEvents();
    }

    /**
     * 发送消息
     */
    async sendMessage() {
        const message = this.inputBox.value.trim();
        if (!message || this.isProcessing) return;

        this.isProcessing = true;
        this.updateSendButton(false);

        // 显示用户消息
        this.addUserMessage(message);
        this.inputBox.value = '';

        try {
            // 发送AI请求
            await this.sendChatRequest(message);
            console.log('AI响应完成');
        } catch (error) {
            console.error('AI对话错误:', error);
            this.addErrorMessage(`对话失败: ${error.message}`);
        } finally {
            this.isProcessing = false;
            this.updateSendButton(true);
        }
    }

    /**
     * 发送AI对话请求
     */
    async sendChatRequest(message) {
        // 创建AI消息元素
        const aiMessageElement = this.createMessageElement('ai');
        this.chatContainer.appendChild(aiMessageElement);
        this.scrollToBottom();

        try {
            const response = await fetch(`${this.apiBaseUrl}/api/chat`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    message: message,
                    sessionId: this.sessionId,
                    stream: false
                })
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const data = await response.json();

            if (data.content) {
                // 处理AI响应
                await this.handleAIResponse(data.content, aiMessageElement);
                return data.content;
            } else {
                throw new Error(data.error || '未知错误');
            }
        } catch (error) {
            console.error('AI对话请求失败:', error);
            this.addErrorMessage(`对话失败: ${error.message}`);
            // 移除AI消息元素
            if (aiMessageElement.parentNode) {
                aiMessageElement.parentNode.removeChild(aiMessageElement);
            }
            throw error;
        }
    }

    /**
     * 处理AI响应
     */
    async handleAIResponse(content, messageElement) {
        // 尝试解析为设备控制JSON
        const extractedJson = this.extractDeviceControlJSON(content);
        if (extractedJson) {
            try {
                const deviceControl = JSON.parse(extractedJson);
                console.log('✅ 检测到设备控制JSON,渲染卡片');
                this.renderDeviceCards(messageElement, deviceControl);

                // 播放综合说明
                if (this.ttsEnabled && deviceControl.综合说明) {
                    await this.playTTS(deviceControl.综合说明);
                }
            } catch (e) {
                console.error('设备控制JSON解析失败:', e);
                this.updateAIMessage(messageElement, content);
            }
        } else {
            // 普通聊天文本
            console.log('💬 普通聊天内容');
            this.updateAIMessage(messageElement, content);

            // 播放聊天内容
            if (this.ttsEnabled) {
                await this.playTTS(content);
            }
        }

        this.finishAIMessage(messageElement);
    }

    /**
     * 提取设备控制JSON
     * @param {string} content - AI响应内容
     * @returns {string|null} - 提取的JSON字符串，如果不是设备控制则返回null
     */
    extractDeviceControlJSON(content) {
        try {
            const trimmed = content.trim();

            // 尝试提取JSON（兼容混合格式）
            let jsonStr = trimmed;

            // 如果不是纯JSON格式，尝试提取其中的JSON部分
            if (!trimmed.startsWith('{') || !trimmed.endsWith('}')) {
                const match = trimmed.match(/\{[\s\S]*\}/);
                if (match) {
                    jsonStr = match[0];
                    console.log('🔧 从混合内容中提取JSON:', jsonStr);
                } else {
                    return null;
                }
            }

            const parsed = JSON.parse(jsonStr);
            const isDeviceControl = parsed.风扇 || parsed.LED灯 || parsed.窗帘 || parsed.空调;

            if (isDeviceControl) {
                console.log('✅ 识别到设备控制JSON:', parsed);
                return jsonStr;
            }

            return null;
        } catch (e) {
            console.error('❌ JSON解析失败:', e);
            return null;
        }
    }

    /**
     * 判断是否为设备控制JSON（保持向后兼容）
     */
    isDeviceControlJSON(content) {
        return this.extractDeviceControlJSON(content) !== null;
    }


    /**
     * 渲染设备卡片
     */
    renderDeviceCards(element, deviceControl) {
        const typingIndicator = element.querySelector('.typing-indicator');
        const messageText = element.querySelector('.message-text');

        if (typingIndicator) {
            typingIndicator.style.display = 'none';
        }

        let cardsHTML = '<div class="device-cards-container">';

        if (deviceControl.风扇) {
            cardsHTML += this.createFanCard(deviceControl.风扇);
        }
        if (deviceControl.LED灯) {
            cardsHTML += this.createLEDCard(deviceControl.LED灯);
        }
        if (deviceControl.窗帘) {
            cardsHTML += this.createCurtainCard(deviceControl.窗帘);
        }
        if (deviceControl.空调) {
            cardsHTML += this.createAirConditionerCard(deviceControl.空调);
        }
        if (deviceControl.综合说明) {
            cardsHTML += `<div class="summary-card">${this.escapeHtml(deviceControl.综合说明)}</div>`;
        }

        cardsHTML += '</div>';
        messageText.innerHTML = cardsHTML;
        messageText.style.display = 'block';
        this.scrollToBottom();
    }

    /**
     * 创建风扇卡片
     */
    createFanCard(fanData) {
        const isOn = fanData.开关 === "开";
        const speed = fanData.风速 || 0;
        const reason = fanData.理由 || "";

        return `
            <div class="device-card fan-card ${isOn ? 'device-on' : 'device-off'}">
                <div class="device-header">
                    <span class="device-icon">🌀</span>
                    <span class="device-name">风扇</span>
                    <span class="device-status">${isOn ? '开启' : '关闭'}</span>
                </div>
                <div class="device-details">
                    <div class="detail-item">
                        <span class="detail-label">状态:</span>
                        <span class="detail-value">${isOn ? '开启' : '关闭'}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">风速:</span>
                        <span class="detail-value">${speed}档</span>
                    </div>
                    ${reason ? `<div class="detail-reason">${this.escapeHtml(reason)}</div>` : ''}
                </div>
            </div>
        `;
    }

    /**
     * 创建LED卡片
     */
    createLEDCard(ledData) {
        const isOn = ledData.开关 === "开";
        const temp = ledData.色温 || "冷光";
        const brightness = ledData.亮度 || 0;
        const reason = ledData.理由 || "";

        return `
            <div class="device-card led-card ${isOn ? 'device-on' : 'device-off'}">
                <div class="device-header">
                    <span class="device-icon">💡</span>
                    <span class="device-name">LED灯</span>
                    <span class="device-status">${isOn ? '开启' : '关闭'}</span>
                </div>
                <div class="device-details">
                    <div class="detail-item">
                        <span class="detail-label">状态:</span>
                        <span class="detail-value">${isOn ? '开启' : '关闭'}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">色温:</span>
                        <span class="detail-value">${temp}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">亮度:</span>
                        <span class="detail-value">${brightness}%</span>
                    </div>
                    ${reason ? `<div class="detail-reason">${this.escapeHtml(reason)}</div>` : ''}
                </div>
            </div>
        `;
    }

    /**
     * 创建窗帘卡片
     */
    createCurtainCard(curtainData) {
        const isOpen = curtainData.开关 === "开";
        const reason = curtainData.理由 || "";

        return `
            <div class="device-card curtain-card ${isOpen ? 'device-on' : 'device-off'}">
                <div class="device-header">
                    <span class="device-icon">🪟</span>
                    <span class="device-name">窗帘</span>
                    <span class="device-status">${isOpen ? '打开' : '关闭'}</span>
                </div>
                <div class="device-details">
                    <div class="detail-item">
                        <span class="detail-label">状态:</span>
                        <span class="detail-value">${isOpen ? '打开' : '关闭'}</span>
                    </div>
                    ${reason ? `<div class="detail-reason">${this.escapeHtml(reason)}</div>` : ''}
                </div>
            </div>
        `;
    }

    /**
     * 创建空调卡片
     */
    createAirConditionerCard(acData) {
        const isOn = acData.开关 === "开";
        const temperature = acData.温度 || "25";
        const mode = acData.模式 || "制冷";
        const windSpeed = acData.风速 || "自动";
        const verticalWind = acData.上下排风 || "自动";
        const horizontalWind = acData.左右排风 || "自动";
        const reason = acData.理由 || "";

        return `
            <div class="device-card ac-card ${isOn ? 'device-on' : 'device-off'}">
                <div class="device-header">
                    <span class="device-icon">❄️</span>
                    <span class="device-name">空调</span>
                    <span class="device-status">${isOn ? '开启' : '关闭'}</span>
                </div>
                <div class="device-details">
                    <div class="detail-item">
                        <span class="detail-label">状态:</span>
                        <span class="detail-value">${isOn ? '开启' : '关闭'}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">温度:</span>
                        <span class="detail-value">${temperature}°C</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">模式:</span>
                        <span class="detail-value">${mode}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">风速:</span>
                        <span class="detail-value">${windSpeed}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">上下排风:</span>
                        <span class="detail-value">${verticalWind}</span>
                    </div>
                    <div class="detail-item">
                        <span class="detail-label">左右排风:</span>
                        <span class="detail-value">${horizontalWind}</span>
                    </div>
                    ${reason ? `<div class="detail-reason">${this.escapeHtml(reason)}</div>` : ''}
                </div>
            </div>
        `;
    }

    /**
     * 查询设备状态
     */
    async queryDeviceState() {
        try {
            const response = await fetch(`${this.apiBaseUrl}/api/devices/state?sessionId=${this.sessionId}`);

            if (!response.ok) {
                throw new Error('查询设备状态失败');
            }

            const data = await response.json();
            this.addSystemMessage('当前设备状态：');

            const messageElement = this.createMessageElement('ai');
            this.chatContainer.appendChild(messageElement);
            this.renderDeviceCards(messageElement, data.deviceState);

        } catch (error) {
            console.error('查询设备状态失败:', error);
            this.addErrorMessage(`查询失败: ${error.message}`);
        }
    }

    /**
     * 创建消息元素
     */
    createMessageElement(type) {
        const messageElement = document.createElement('div');
        messageElement.className = `chat-message ${type}-message`;

        if (type === 'ai') {
            messageElement.innerHTML = `
                <div class="typing-indicator">
                    <span></span><span></span><span></span>
                </div>
                <div class="message-text" style="display: none;"></div>
            `;
        } else {
            messageElement.innerHTML = `<div class="message-text"></div>`;
        }

        return messageElement;
    }

    /**
     * 添加用户消息
     */
    addUserMessage(message) {
        const messageElement = this.createMessageElement('user');
        messageElement.querySelector('.message-text').textContent = message;
        this.chatContainer.appendChild(messageElement);
        this.scrollToBottom();
    }

    /**
     * 添加系统消息
     */
    addSystemMessage(message) {
        const messageElement = this.createMessageElement('system');
        messageElement.className = 'chat-message system-message';
        messageElement.querySelector('.message-text').textContent = message;
        this.chatContainer.appendChild(messageElement);
        this.scrollToBottom();
    }

    /**
     * 添加错误消息
     */
    addErrorMessage(message) {
        const messageElement = this.createMessageElement('error');
        messageElement.className = 'chat-message error-message';
        messageElement.querySelector('.message-text').textContent = message;
        this.chatContainer.appendChild(messageElement);
        this.scrollToBottom();
    }

    /**
     * 更新AI消息内容
     */
    updateAIMessage(element, content) {
        const typingIndicator = element.querySelector('.typing-indicator');
        const messageText = element.querySelector('.message-text');

        if (typingIndicator) {
            typingIndicator.style.display = 'none';
        }

        messageText.innerHTML = this.formatMessage(content);
        messageText.style.display = 'block';
        this.scrollToBottom();
    }

    /**
     * 完成AI消息
     */
    finishAIMessage(element) {
        const typingIndicator = element.querySelector('.typing-indicator');
        if (typingIndicator) {
            typingIndicator.style.display = 'none';
        }
    }

    /**
     * 格式化消息内容
     */
    formatMessage(content) {
        return this.escapeHtml(content).replace(/\n/g, '<br>');
    }

    /**
     * HTML转义
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    /**
     * 更新发送按钮状态
     */
    updateSendButton(enabled) {
        this.sendButton.disabled = !enabled;
        this.sendButton.textContent = enabled ? '发送' : '发送中...';
    }

    /**
     * 滚动到底部
     */
    scrollToBottom() {
        this.chatContainer.scrollTop = this.chatContainer.scrollHeight;
    }

    /**
     * 清除对话
     */
    clearChat() {
        this.chatContainer.innerHTML = '';
        this.addSystemMessage('对话已清除。有什么可以帮助你的吗？');
    }

    /**
     * 生成会话ID
     */
    generateSessionId() {
        return 'session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
    }

    /**
     * 设置语音播放状态
     */
    setTTSEnabled(enabled) {
        this.ttsEnabled = enabled;
        console.log(enabled ? '🔊 语音播放已开启' : '🔇 语音播放已关闭');
    }

    /**
     * 获取语音播放状态
     */
    getTTSEnabled() {
        return this.ttsEnabled;
    }

    /**
     * 切换语音播放状态
     */
    toggleTTS() {
        this.ttsEnabled = !this.ttsEnabled;
        this.updateTTSButton();
        console.log(this.ttsEnabled ? '🔊 语音播放已开启' : '🔇 语音播放已关闭');
    }

    /**
     * 更新语音按钮显示
     */
    updateTTSButton() {
        if (this.ttsToggleButton) {
            this.ttsToggleButton.textContent = this.ttsEnabled ? '🔊 语音开启' : '🔇 语音关闭';
            this.ttsToggleButton.className = this.ttsEnabled ? 'btn-ai-tts active' : 'btn-ai-tts';
        }
    }

    /**
     * 加载音色列表
     */
    async loadVoiceList() {
        try {
            const response = await fetch(`${this.apiBaseUrl}/api/tts/voices`);
            const data = await response.json();

            if (data.success) {
                this.voiceList = data.data.voices;
                this.populateVoiceSelect(data.data);
                console.log('✅ 音色列表加载成功');
            } else {
                console.error('❌ 音色列表加载失败:', data.error);
            }
        } catch (error) {
            console.error('❌ 音色列表加载错误:', error);
        }
    }

    /**
     * 填充音色选择器
     */
    populateVoiceSelect(data) {
        if (!this.voiceSelect) return;

        // 清空现有选项
        this.voiceSelect.innerHTML = '<option value="">🎵 音色</option>';

        // 添加音色选项
        Object.entries(data.voices).forEach(([voiceId, voiceInfo]) => {
            const option = document.createElement('option');
            option.value = voiceId;
            option.textContent = `${voiceInfo.name} (${voiceInfo.gender} - ${voiceInfo.style})`;
            this.voiceSelect.appendChild(option);
        });

        // 设置默认选择
        if (this.selectedVoice && this.voiceList[this.selectedVoice]) {
            this.voiceSelect.value = this.selectedVoice;
        } else {
            this.voiceSelect.value = data.default;
            this.selectedVoice = data.default;
        }
    }

    /**
     * 加载设置
     */
    loadSettings() {
        const settings = localStorage.getItem('aiSettings');
        if (settings) {
            try {
                const parsed = JSON.parse(settings);
                this.selectedVoice = parsed.selectedVoice || '';
                this.ttsEnabled = parsed.ttsEnabled !== false; // 默认开启
                this.voiceEnabled = parsed.voiceEnabled !== false; // 默认开启
            } catch (error) {
                console.error('设置加载失败:', error);
            }
        }
    }

    /**
     * 保存设置
     */
    saveSettings() {
        const settings = {
            selectedVoice: this.selectedVoice,
            ttsEnabled: this.ttsEnabled,
            voiceEnabled: this.voiceEnabled
        };
        localStorage.setItem('aiSettings', JSON.stringify(settings));
    }

    /**
     * 音色预览
     */
    async previewVoice() {
        const selectedVoice = this.voiceSelect.value;
        if (!selectedVoice) {
            alert('请先选择音色');
            return;
        }

        const sampleText = `您好，我是小月！这是${this.voiceList[selectedVoice]?.name}的音色。`;

        try {
            this.voicePreviewButton.disabled = true;
            this.voicePreviewButton.textContent = '播放中...';

            const response = await fetch(`${this.apiBaseUrl}/api/tts`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    text: sampleText,
                    voice: selectedVoice
                })
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const arrayBuffer = await response.arrayBuffer();
            await this.ttsClient.playAudioBuffer(await this.ttsClient.pcmToAudioBuffer(arrayBuffer, this.ttsClient.initAudioContext()));

        } catch (error) {
            console.error('音色预览失败:', error);
            alert('音色预览失败，请稍后再试');
        } finally {
            this.voicePreviewButton.disabled = false;
            this.voicePreviewButton.textContent = '试听';
        }
    }

    /**
     * 更新音色选择事件绑定
     */
    updateVoiceBindEvents() {
        if (this.voiceSelect) {
            this.voiceSelect.addEventListener('change', () => {
                this.selectedVoice = this.voiceSelect.value;
                this.saveSettings();
                console.log(`🎵 音色已切换为: ${this.voiceList[this.selectedVoice]?.name || '默认'}`);
            });
        }

        if (this.voicePreviewButton) {
            this.voicePreviewButton.addEventListener('click', () => {
                this.previewVoice();
            });
        }

        if (this.voiceToggleButton) {
            this.voiceToggleButton.addEventListener('click', () => {
                this.toggleVoiceEnabled();
            });
        }
    }

    /**
     * 切换音色功能开关
     */
    toggleVoiceEnabled() {
        this.voiceEnabled = !this.voiceEnabled;
        this.updateVoiceToggleButton();
        this.updateVoiceControlVisibility();
        this.saveSettings();
        console.log(this.voiceEnabled ? '🎵 音色功能已开启' : '🔇 音色功能已关闭');
    }

    /**
     * 更新音色开关按钮显示
     */
    updateVoiceToggleButton() {
        if (this.voiceToggleButton) {
            this.voiceToggleButton.textContent = this.voiceEnabled ? '🎵 音色开启' : '🔇 音色关闭';
            this.voiceToggleButton.className = this.voiceEnabled ? 'btn-ai-voice-toggle' : 'btn-ai-voice-toggle disabled';
        }
    }

    /**
     * 更新音色控制组件的可见性
     */
    updateVoiceControlVisibility() {
        if (this.voiceSelect) {
            this.voiceSelect.style.display = this.voiceEnabled ? 'block' : 'none';
        }
        if (this.voicePreviewButton) {
            this.voicePreviewButton.style.display = this.voiceEnabled ? 'block' : 'none';
        }
    }

    /**
     * 修改playTTS方法，只有在音色功能开启时才使用选定的音色
     */
    async playTTS(text) {
        if (!this.ttsClient || !this.ttsEnabled) return;

        try {
            const voice = this.voiceEnabled ? this.selectedVoice : null;
            await this.ttsClient.speak(text, voice);
        } catch (error) {
            console.error('语音播放失败:', error);
        }
    }
}