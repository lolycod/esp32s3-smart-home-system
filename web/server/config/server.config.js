/**
 * 服务器配置文件
 */

const config = {
  // 默认端口 - 采用单端口设计，统一使用8080端口（HTTP + WebSocket）
  defaultPort: 8080,

  // 端口范围（用于自动寻找可用端口）
  portRange: {
    min: 8080,
    max: 8090
  },

  // 统一服务器配置
  unified: {
    // 启用HTTP静态文件服务
    enableHTTP: true,

    // 启用WebSocket服务
    enableWebSocket: true,

    // 静态文件目录（相对于服务器根目录）
    staticPath: '../client'
  },

  // WebSocket配置
  websocket: {
    // 最大连接数
    maxConnections: 100,

    // 连接超时时间（毫秒）
    connectionTimeout: 30000,

    // 心跳间隔（毫秒）
    heartbeatInterval: 30000
  },

  // 消息配置
  message: {
    // 最大消息大小（字节）
    maxSize: 1024 * 1024, // 1MB

    // 消息类型
    types: {
      MESSAGE: 'message',
      SYSTEM: 'system',
      ERROR: 'error',
      AI_DETECTION: 'ai_detection',  // MaixCAM AI检测结果
      GESTURE_CONTROL: 'gesture_control'  // MaixCAM 手势控制
    }
  },

  // 日志配置
  logging: {
    // 是否启用日志
    enabled: true,

    // 日志级别
    level: 'info'
  }
  ,
  // TTS配置（可用环境变量覆盖）
  tts: {
    appId: 'a7ca559a',
    apiKey: '00af4af979e78bb3cba31a2987a48886',
    apiSecret: 'YWYwNzI0OGQyOTcwZTczZDFkZTVjMDJj'
  }
};

module.exports = config;
