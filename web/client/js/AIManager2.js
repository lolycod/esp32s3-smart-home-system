/**
 * AI对话管理器（含TTS自动播报与设备卡片渲染）
 */
class AIManager {
  constructor() {
    this.apiBaseUrl = window.location.origin;
    this.sessionId = this.generateSessionId();
    this.isProcessing = false;
    this.chatContainer = null;
    this.inputBox = null;
    this.sendButton = null;
    this.queryStateButton = null;
    this.clearButton = null;

    this.autoTTSToggle = null;
    this.playVoiceBtn = null;
    this.stopVoiceBtn = null;
    this.autoTTS = true;
    this.lastAIText = '';

    this.tts = new TTSClient(this.apiBaseUrl);
  }

  init(elements) {
    this.chatContainer = elements.chatContainer;
    this.inputBox = elements.inputBox;
    this.sendButton = elements.sendButton;
    this.queryStateButton = elements.queryStateButton;
    this.clearButton = elements.clearButton;
    this.autoTTSToggle = elements.autoTTSToggle;
    this.volumeSlider = elements.volumeSlider; this.volumeValue = elements.volumeValue; this.playVoiceBtn = elements.playVoiceBtn; this.stopVoiceBtn = elements.stopVoiceBtn;

    this.bindEvents();

    try {
      const saved = localStorage.getItem('aiAutoTTS');
      this.autoTTS = saved == null ? true : saved === 'true';
    } catch (_) {}
    if (this.autoTTSToggle) {
      this.autoTTSToggle.checked = this.autoTTS;
      this.autoTTSToggle.addEventListener('change', () => {
        this.autoTTS = !!this.autoTTSToggle.checked;
        try { localStorage.setItem('aiAutoTTS', String(this.autoTTS)); } catch(_){}
      });
    }
    if (this.playVoiceBtn) {
      this.playVoiceBtn.addEventListener('click', async () => {
        const text = (this.lastAIText || '').trim();
        if (text) {
          try { await this.tts.speakStream(text); } catch(e){ console.warn('TTS播放失败:', e.message); }
        }
      });
    }
    if (this.stopVoiceBtn) {
      this.stopVoiceBtn.addEventListener('click', () => { try { this.tts.stop(); } catch(_){} });
    }

    this.addSystemMessage('你好！我是AI助手，有什么可以帮助你的吗�?);
  }

  bindEvents() {
    if (this.sendButton) this.sendButton.addEventListener('click', () => this.sendMessage());
    if (this.inputBox) {
      this.inputBox.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) { e.preventDefault(); this.sendMessage(); }
      });
    }
    if (this.queryStateButton) this.queryStateButton.addEventListener('click', () => this.queryDeviceState());
    if (this.clearButton) this.clearButton.addEventListener('click', () => this.clearConversation());
  }

  async sendMessage() {
    const message = (this.inputBox?.value || '').trim();
    if (!message || this.isProcessing) return;
    this.addUserMessage(message);
    if (this.inputBox) this.inputBox.value = '';
    this.setProcessing(true);
    try {
      const aiMsg = this.createAIMessageElement();
      await this.sendStreamRequest(message, aiMsg);
    } catch (e) {
      this.addErrorMessage(`发送失�? ${e.message}`);
    } finally {
      this.setProcessing(false);
    }
  }

  sendStreamRequest(message, aiMessageElement) {
    return new Promise((resolve, reject) => {
      const url = `${this.apiBaseUrl}/api/chat`;
      fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ message, sessionId: this.sessionId, stream: true }) })
        .then(res => { if (!res.ok) throw new Error(`HTTP ${res.status}`); return res.body.getReader(); })
        .then(reader => {
          const decoder = new TextDecoder(); let buffer = ''; let full = '';
          const loop = () => reader.read().then(async ({ done, value }) => {
            if (done) { resolve(full); return; }
            buffer += decoder.decode(value, { stream: true });
            const lines = buffer.split('\n'); buffer = lines.pop();
            for (const line of lines) {
              if (!line.startsWith('data: ')) continue; const data = line.slice(6).trim();
              try {
                const parsed = JSON.parse(data);
                if (parsed.type === 'chunk') { full += parsed.content || ''; }
                else if (parsed.type === 'done') {
                  full = parsed.content || full;
                  const trimmed = String(full).trim();
                  const dc = this._tryParseDeviceControl(trimmed);
                  if (this._hasDeviceKeys(dc)) {
                    this.renderDeviceCards(aiMessageElement, dc); const sum = this._makeDeviceSummary(dc); this.lastAIText = sum; if (this.autoTTS && sum) { try { await this.tts.speakStream(sum); } catch(e){ console.warn('TTS���ž���:', e.message); } }
                  } else {
                    this.updateAIMessage(aiMessageElement, full);
                    this.lastAIText = this._extractSpeakText(trimmed);
                    if (this.autoTTS && this.lastAIText) {
                      try { await this.tts.speakStream(this.lastAIText); } catch(e){ console.warn('TTS播放警告:', e.message); }
                    }
                  }
                  this.finishAIMessage(aiMessageElement);
                  resolve(full);
                } else if (parsed.type === 'error') { reject(new Error(parsed.error || '服务错误')); }
              } catch (_) { /* 忽略不完整行 */ }
            }
            return loop();
          }).catch(reject);
          return loop();
        })
        .catch(reject);
    });
  }

  addUserMessage(message) {
    const el = document.createElement('div'); el.className = 'chat-message user-message';
    el.innerHTML = `<div class="message-content"><div class="message-bubble user-bubble"><p>${this.escapeHtml(message)}</p></div><div class="message-time">${this.getTimeString()}</div></div>`;
    this.chatContainer.appendChild(el); this.scrollToBottom();
  }

  createAIMessageElement() {
    const el = document.createElement('div'); el.className = 'chat-message ai-message';
    el.innerHTML = `<div class="message-content"><div class="message-bubble ai-bubble"><div class="typing-indicator"><span></span><span></span><span></span></div><div class="message-text" style="display:none;"></div></div><div class="message-time">${this.getTimeString()}</div></div>`;
    this.chatContainer.appendChild(el); return el;
  }

  updateAIMessage(element, content) {
    const typing = element.querySelector('.typing-indicator'); const box = element.querySelector('.message-text');
    if (typing) typing.style.display = 'none'; box.style.display = 'block'; box.innerHTML = this.formatMessage(content); this.scrollToBottom();
  }
  finishAIMessage(element) { element.classList.add('message-complete'); }

  addSystemMessage(message) {
    const el = document.createElement('div'); el.className = 'chat-message system-message';
    el.innerHTML = `<div class="message-content"><div class="message-bubble system-bubble"><p>${this.escapeHtml(message)}</p></div><div class="message-time">${this.getTimeString()}</div></div>`;
    this.chatContainer.appendChild(el); this.scrollToBottom();
  }
  addErrorMessage(message) {
    const el = document.createElement('div'); el.className = 'chat-message error-message';
    el.innerHTML = `<div class="message-content"><div class="message-bubble error-bubble"><p>⚠️ ${this.escapeHtml(message)}</p></div><div class="message-time">${this.getTimeString()}</div></div>`;
    this.chatContainer.appendChild(el); this.scrollToBottom();
  }

  async clearConversation() {
    try { await fetch(`${this.apiBaseUrl}/api/chat/clear`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ sessionId: this.sessionId }) }); } catch (_) {}
    this.chatContainer.innerHTML = ''; this.addSystemMessage('对话已清除，我们重新开始吧�?);
  }

  async queryDeviceState() {
    try {
      const res = await fetch(`${this.apiBaseUrl}/api/devices/state?sessionId=${this.sessionId}`); if (!res.ok) throw new Error('查询设备状态失�?);
      const data = await res.json(); const el = this.createAIMessageElement(); this.renderDeviceCards(el, data.deviceState || {});
    } catch (e) { this.addErrorMessage(`查询失败: ${e.message}`); }
  }

  setProcessing(processing) {
    this.isProcessing = processing; if (this.sendButton) this.sendButton.disabled = processing; if (this.inputBox) this.inputBox.disabled = processing; if (this.sendButton) this.sendButton.textContent = processing ? '发送中...' : '发�?;
  }

  // 渲染卡片（苹果风样式，配�?index.html 现有CSS�?  renderDeviceCards(element, dcRaw) {
    const typing = element.querySelector('.typing-indicator'); const box = element.querySelector('.message-text'); if (typing) typing.style.display = 'none';
    const norm = this._normalizeDeviceControl(dcRaw || {}); let html = '<div class="device-cards-container">';
    if (norm.fan) { const f = norm.fan; html += `
      <div class="device-card fan-card ${f.on ? 'device-on' : 'device-off'}">
        <div class="card-header"><span class="device-icon">🌀</span><span class="device-name">风扇</span><span class="device-status ${f.on ? 'status-on' : 'status-off'}">${f.statusText}</span></div>
        <div class="card-body"><div class="device-property"><span class="property-label">风�?/span><span class="property-value"><span class="speed-bar">${this.generateSpeedBar(f.speed || 0, 5)}</span><span class="speed-text">${f.speed || 0}�?/span></span></div>${f.reason ? `<div class="device-reason">${this.escapeHtml(f.reason)}</div>` : ''}</div>
      </div>`; }
    if (norm.led) { const l = norm.led; const tempClass = l.colorTemp === '冷光' ? 'cold' : 'warm'; html += `
      <div class="device-card led-card ${l.on ? 'device-on' : 'device-off'}">
        <div class="card-header"><span class="device-icon">💡</span><span class="device-name">LED�?/span><span class="device-status ${l.on ? 'status-on' : 'status-off'}">${l.statusText}</span></div>
        <div class="card-body"><div class="device-property"><span class="property-label">色温</span><span class="property-value color-temp ${tempClass}">${l.colorTemp}</span></div><div class="device-property"><span class="property-label">亮度</span><span class="property-value"><span class="brightness-bar"><span class="brightness-fill" style="width: ${l.brightness || 0}%"></span></span><span class="brightness-text">${l.brightness || 0}%</span></span></div>${l.reason ? `<div class="device-reason">${this.escapeHtml(l.reason)}</div>` : ''}</div>
      </div>`; }
    if (norm.curtain) { const c = norm.curtain; html += `
      <div class="device-card curtain-card ${c.isOpen ? 'device-on' : 'device-off'}">
        <div class="card-header"><span class="device-icon">🪟</span><span class="device-name">窗帘</span><span class="device-status ${c.isOpen ? 'status-on' : 'status-off'}">${c.statusText}</span></div>
        <div class="card-body"><div class="curtain-visual"><div class="curtain-icon ${c.isOpen ? 'open' : 'closed'}">${c.isOpen ? '�?�? : '▐▌'}</div></div>${c.reason ? `<div class="device-reason">${this.escapeHtml(c.reason)}</div>` : ''}</div>
      </div>`; }
    if (dcRaw && dcRaw['综合说明']) { html += `<div class="summary-card">${this.escapeHtml(String(dcRaw['综合说明']))}</div>`; }
    html += '</div>'; box.innerHTML = html; box.style.display = 'block'; this.scrollToBottom();
  }

  // 工具与提�?  escapeHtml(text) { const div = document.createElement('div'); div.textContent = text; return div.innerHTML; }
  getTimeString() { return new Date().toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit' }); }
  scrollToBottom() { setTimeout(() => { this.chatContainer.scrollTop = this.chatContainer.scrollHeight; }, 50); }
  generateSessionId() { return 'session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9); }
  generateSpeedBar(speed, max) { let bars=''; for (let i=1;i<=max;i++){ bars += `<span class="speed-level ${i<=speed?'active':''}"></span>`;} return bars; }

  formatMessage(content) {
    let textInput = String(content == null ? '' : content); const t = textInput.trim();
    if ((t.startsWith('{') && t.endsWith('}')) || (t.startsWith('[') && t.endsWith(']'))) {
      try { const obj = JSON.parse(t); if (this._hasDeviceKeys(obj)) return '<em>已应用设备控�?/em>'; const extracted = this._extractTextFromObject(obj); if (extracted) textInput = extracted; else return '<em>已处理结构化内容</em>'; } catch (_) {}
    }
    let formatted = this.escapeHtml(textInput); formatted = formatted.replace(/\n/g, '<br>'); formatted = formatted.replace(/```([\s\S]*?)```/g, '<pre><code>$1</code></pre>'); formatted = formatted.replace(/`([^`]+)`/g, '<code>$1</code>'); formatted = formatted.replace(/\*\*(.+?)\*\*/g, '<strong>$1</strong>'); formatted = formatted.replace(/\*(.+?)\*/g, '<em>$1</em>'); return formatted;
  }

  _tryParseDeviceControl(text) {
    if (!text) return null; const trimmed = String(text).trim(); const candidates = [trimmed];
    const fence = trimmed.match(/```(?:json)?\s*([\s\S]*?)```/i); if (fence && fence[1]) candidates.push(fence[1].trim());
    const fb = trimmed.indexOf('{'); const lb = trimmed.lastIndexOf('}'); if (fb !== -1 && lb > fb) candidates.push(trimmed.slice(fb, lb + 1));
    for (const c of candidates) { try { const obj = JSON.parse(c); if (obj && typeof obj === 'object') return obj; } catch (_) {} }
    return null;
  }
  _hasDeviceKeys(obj) { if (!obj || typeof obj !== 'object') return false; return !!(obj['风扇'] || obj['LED'] || obj['LED�?] || obj['窗帘'] || obj.fan || obj.led || obj.curtain); }
  _extractTextFromObject(obj) {
    if (obj == null) return ''; if (typeof obj === 'string') return obj; const prefer = ['content','text','message','answer','reply','output','result'];
    for (const k of prefer) { if (typeof obj[k] === 'string' && obj[k].trim()) return obj[k]; }
    const queue = [obj]; let depth = 0; while (queue.length && depth < 3) { const size = queue.length; for (let i = 0; i < size; i++) { const cur = queue.shift(); if (typeof cur === 'string' && cur.trim()) return cur; if (Array.isArray(cur)) { for (const it of cur) queue.push(it); } else if (cur && typeof cur === 'object') { for (const v of Object.values(cur)) queue.push(v); } } depth++; }
    return '';
  }
  _extractSpeakText(text) {
    const t = String(text || '').trim();
    try { const obj = JSON.parse(t); if (this._hasDeviceKeys(obj)) return ''; const ex = this._extractTextFromObject(obj); return ex || ''; } catch(_) { return t; }
  }
  _normalizeDeviceControl(dc) {
    const fanRaw = this._pickKey(dc, ['风扇','fan','Fan','FAN']);
    const ledRaw = this._pickKey(dc, ['LED�?,'LED','led']);
    const curtainRaw = this._pickKey(dc, ['窗帘','curtain','Curtain']);
    const norm = {};
    if (fanRaw) {
      const statusText = this._pickKey(fanRaw, ['开�?,'开','status','switch']);
      norm.fan = { on: statusText === '开' || !!fanRaw.on, statusText: statusText === true ? '开' : (statusText === false ? '�? : String(statusText || '�?)), speed: this._toInt(this._pickKey(fanRaw, ['风�?,'speed','风量','级别'])) || 0, reason: this._pickKey(fanRaw, ['理由','reason','说明']) || '' };
    }
    if (ledRaw) {
      const statusText = this._pickKey(ledRaw, ['开�?,'开','status','switch']); let colorTemp = this._pickKey(ledRaw, ['色温','colorTemp','color_temperature','color']) || '冷光'; colorTemp = /warm|�?.test(String(colorTemp)) ? '暖光' : '冷光';
      norm.led = { on: statusText === '开' || !!ledRaw.on, statusText: statusText === true ? '开' : (statusText === false ? '�? : String(statusText || '�?)), colorTemp, brightness: this._toInt(this._pickKey(ledRaw, ['亮度','brightness'])) || 0, reason: this._pickKey(ledRaw, ['理由','reason','说明']) || '' };
    }
    if (curtainRaw) {
      const statusText = this._pickKey(curtainRaw, ['开�?,'开','status','switch']); const isOpen = statusText === '开' || !!curtainRaw.on;
      norm.curtain = { isOpen, statusText: statusText === true ? '开' : (statusText === false ? '�? : String(statusText || (isOpen ? '开' : '�?))), reason: this._pickKey(curtainRaw, ['理由','reason','说明']) || '' };
    }
    return norm;
  }
  _pickKey(obj, keys) { if (!obj) return undefined; for (const k of keys) { if (Object.prototype.hasOwnProperty.call(obj, k)) { const v = obj[k]; if (v !== undefined && v !== null) return v; } } return undefined; }
  _toInt(v) { const n = parseInt(v, 10); return isNaN(n) ? 0 : n; }
}




  _makeDeviceSummary(dc){
    const n = this._normalizeDeviceControl(dc||{});
    const parts = [];
    if(n.fan){ const f=n.fan; parts.push("����" + (f.on?"�ѿ���":"�ѹر�") + (f.speed?`������${f.speed}��`:"")); }
    if(n.led){ const l=n.led; parts.push("LED��" + (l.on?"�ѿ���":"�ѹر�") + (typeof l.brightness==='number'?`������${l.brightness}%`:"") + (l.colorTemp?`��${l.colorTemp}`:"")); }
    if(n.curtain){ const c=n.curtain; parts.push("����" + (c.isOpen?"�Ѵ�":"�ѹر�")); }
    return parts.join("��");
  }
}

