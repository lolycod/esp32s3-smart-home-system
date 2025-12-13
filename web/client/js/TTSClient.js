/**
 * 语音合成客户端（支持流式边播边合成 + 音量控制）
 */
class TTSClient {
  constructor(apiBaseUrl = (window.location.protocol === 'file:' ? 'http://localhost:8080' : window.location.origin)) {
    this.apiBaseUrl = apiBaseUrl;
    this.audioContext = null;
    this.gainNode = null;
    this.volume = 0.8; // 0.0 - 1.0
    this.isPlaying = false;
    this.currentSource = null;
    this.playHead = 0; // 调度起点（秒）
  }

  initAudioContext() {
    if (!this.audioContext) {
      this.audioContext = new (window.AudioContext || window.webkitAudioContext)();
      this.gainNode = this.audioContext.createGain();
      this.gainNode.gain.value = this.volume;
      this.gainNode.connect(this.audioContext.destination);
    }
    return this.audioContext;
  }

  // 非流式：一次性获取PCM并播放
  async speak(text, voice = null) {
    if (!text) return;
    if (this.isPlaying) this.stop();
    try {
      const requestBody = { text };
      if (voice) {
        requestBody.voice = voice;
      }

      const resp = await fetch(`${this.apiBaseUrl}/api/tts`, {
        method: 'POST', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestBody)
      });
      if (!resp.ok) throw new Error(`TTS API错误: ${resp.status}`);
      const arrayBuffer = await resp.arrayBuffer();
      const ac = this.initAudioContext();
      const audioBuffer = await this.pcmToAudioBuffer(arrayBuffer, ac);
      await this.playAudioBuffer(audioBuffer);
    } catch (e) {
      console.error('语音合成失败:', e);
      throw e;
    }
  }

  // 流式：SSE边收边播
  async speakStream(text) {
    if (!text) return;
    this.stop();
    const ac = this.initAudioContext();
    this.playHead = ac.currentTime + 0.05;
    this.isPlaying = true;
    try {
      const resp = await fetch(`${this.apiBaseUrl}/api/tts/stream`, {
        method: 'POST', headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text })
      });
      if (!resp.ok || !resp.body) throw new Error(`TTS流式API错误: ${resp.status}`);

      const reader = resp.body.getReader();
      const decoder = new TextDecoder();
      let buffer = '';

      const pump = async () => {
        const { done, value } = await reader.read();
        if (done) { this.isPlaying = false; return; }
        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split('\n');
        buffer = lines.pop();
        for (const line of lines) {
          if (!line.startsWith('data: ')) continue;
          const data = line.slice(6).trim();
          try {
            const evt = JSON.parse(data);
            if (evt.type === 'chunk' && evt.audio) {
              const pcm = this._b64ToArrayBuffer(evt.audio);
              await this._schedulePCMChunk(pcm);
            } else if (evt.type === 'end') {
              this.isPlaying = false;
            } else if (evt.type === 'error') {
              throw new Error(evt.error || 'TTS流错误');
            }
          } catch (_) { /* 忽略解析错误 */ }
        }
        return pump();
      };
      await pump();
    } catch (e) {
      this.isPlaying = false;
      throw e;
    }
  }

  _b64ToArrayBuffer(b64) {
    const binary = atob(b64);
    const len = binary.length;
    const bytes = new Uint8Array(len);
    for (let i = 0; i < len; i++) bytes[i] = binary.charCodeAt(i);
    return bytes.buffer;
  }

  async _schedulePCMChunk(pcmArrayBuffer) {
    const ac = this.initAudioContext();
    const pcm = new Int16Array(pcmArrayBuffer);
    if (pcm.length === 0) return;
    const audioBuffer = ac.createBuffer(1, pcm.length, 16000);
    const ch0 = audioBuffer.getChannelData(0);
    for (let i = 0; i < pcm.length; i++) ch0[i] = pcm[i] / 32768.0;

    const src = ac.createBufferSource();
    src.buffer = audioBuffer;
    if (!this.gainNode) { this.gainNode = ac.createGain(); this.gainNode.gain.value = this.volume; this.gainNode.connect(ac.destination); }
    src.connect(this.gainNode);

    const startAt = Math.max(ac.currentTime + 0.01, this.playHead);
    src.start(startAt);
    this.currentSource = src;
    this.playHead = startAt + audioBuffer.duration;
  }

  async pcmToAudioBuffer(pcmData, audioContext) {
    const sampleRate = 16000;
    const numChannels = 1;
    const pcmSamples = new Int16Array(pcmData);
    const audioBuffer = audioContext.createBuffer(numChannels, pcmSamples.length, sampleRate);
    const channelData = audioBuffer.getChannelData(0);
    for (let i = 0; i < pcmSamples.length; i++) channelData[i] = pcmSamples[i] / 32768.0;
    return audioBuffer;
  }

  playAudioBuffer(audioBuffer) {
    return new Promise((resolve, reject) => {
      try {
        const ac = this.initAudioContext();
        const source = ac.createBufferSource();
        source.buffer = audioBuffer;
        if (!this.gainNode) { this.gainNode = ac.createGain(); this.gainNode.gain.value = this.volume; this.gainNode.connect(ac.destination); }
        source.connect(this.gainNode);
        this.currentSource = source;
        this.isPlaying = true;
        source.onended = () => { this.isPlaying = false; this.currentSource = null; resolve(); };
        source.start(0);
      } catch (e) { this.isPlaying = false; this.currentSource = null; reject(e); }
    });
  }

  stop() {
    if (this.currentSource && this.isPlaying) {
      try {
        this.currentSource.stop();
        this.currentSource = null;
        this.isPlaying = false;
        console.log('⏹️  语音播放已停止');
      } catch (_) {}
    }
  }

  setVolume(volume) {
    const v = Math.max(0, Math.min(1, Number(volume)));
    if (!isNaN(v)) { this.volume = v; if (this.gainNode) this.gainNode.gain.value = this.volume; }
  }
  getVolume() { return this.volume; }
  getIsPlaying() { return this.isPlaying; }
}

