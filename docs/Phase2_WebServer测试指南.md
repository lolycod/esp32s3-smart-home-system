# Phase 2: Web Server测试指南

**目标**: 验证Web Server能够正常接收MaixCAM的WebSocket数据并代理MJPEG视频流

---

## ✅ 已完成的工作

1. **创建备份文件**: `web/server/lib/UnifiedServer.js.backup`（原始文件已保存）
2. **修改配置文件**: `web/server/config/server.config.js`（添加ai_detection消息类型）
3. **修改消息处理器**: `web/server/lib/MessageHandler.js`（添加AI检测消息处理逻辑）
4. **修改统一服务器**: `web/server/lib/UnifiedServer.js`（添加MJPEG代理路由）

### 新增功能
- ✅ 支持`ai_detection`消息类型（MaixCAM AI检测结果）
- ✅ MJPEG视频流代理（HTTP路由：`/maixcam_stream`）
- ✅ WebSocket消息转发（AI检测数据广播给所有浏览器客户端）
- ✅ 详细的错误处理和日志输出

---

## 📝 测试前准备

### 步骤1: 确保MaixCAM程序正在运行

**在MaixCAM设备上确认：**
1. 打开SSH连接到MaixCAM
2. 确认`main.py`程序正在运行（Phase 1应该还在运行）
3. 如果已停止，重新启动：
   ```bash
   cd /root
   python3 main.py
   ```
4. 确认看到以下日志：
   ```
   🎥 系统启动，开始检测...
   📊 已处理X帧 | 当前检测: X人, X火
   ```

### 步骤2: 安装Node.js依赖（如果还没有）

**注意**: 本项目使用原生Node.js http模块进行MJPEG代理，**不需要**额外安装`request`或`ws`包（ws已安装）。

**在项目web目录下执行：**
```bash
cd C:\Users\Link\Desktop\222\-esp32s3--master\-esp32s3--master\web
npm install
```

如果之前已安装过，可以跳过此步骤。

---

## 🚀 开始测试

### 测试步骤1: 启动Web Server

**在web目录下执行：**
```bash
cd C:\Users\Link\Desktop\222\-esp32s3--master\-esp32s3--master\web
node server/server.js
```

### 测试步骤2: 观察Web Server输出

**正常启动日志示例：**
```
🚀 启动智能终端管理系统统一服务器...
📋 服务器配置信息:
   - 默认端口: 8080 (HTTP + WebSocket)
   - 端口范围: 8080-8090
   - 最大连接数: 100
   - 心跳间隔: 30000ms
   - HTTP服务: 启用
   - WebSocket服务: 启用

✅ 统一服务器监听所有网络接口: 0.0.0.0:8080
✅ 统一服务器已启动
📡 监听端口: 8080
🌐 HTTP服务地址: http://localhost:8080
🔌 WebSocket地址: ws://localhost:8080
📁 静态文件目录: C:\Users\Link\Desktop\222\-esp32s3--master\-esp32s3--master\web\client

🎉 统一服务器启动成功！
📝 使用说明:
   1. 打开Web管理界面: http://localhost:8080
   2. WebSocket连接地址: ws://localhost:8080
   3. 外网访问地址: ws://www.lolycod123.top:8080
   4. 单端口设计，简化网络配置

⌨️  按 Ctrl+C 停止服务器
```

### 测试步骤3: 测试WebSocket连接

**目标**: 验证MaixCAM能否成功连接到Web Server

**在MaixCAM终端观察：**
- 如果之前显示`❌ 连接失败: [Errno 113] Host is unreachable`
- 现在应该显示：`✅ WebSocket连接成功`

**在Web Server终端观察：**
- 应该看到：
  ```
  🔌 新的WebSocket连接请求:
     - 客户端IP: 192.168.31.43
     - Origin: unknown
     - User-Agent: Python/3.x
     - 请求URL: /
  ```

**如果MaixCAM WebSocket连接失败：**
1. 检查防火墙是否阻止8080端口
2. 检查MaixCAM和电脑是否在同一WiFi网络
3. 使用`ipconfig`确认电脑IP地址是否正确（在`main.py`第20行配置）
4. 重启MaixCAM的`main.py`程序

### 测试步骤4: 测试MJPEG视频流代理

**目标**: 验证Web Server能否正常代理MaixCAM的视频流

**测试方法1：直接访问代理端点**
1. 打开浏览器
2. 访问：`http://localhost:8080/maixcam_stream`

**预期结果**：
- ✅ 能看到MaixCAM的实时视频画面（带检测框）
- ✅ 视频流畅，无明显卡顿
- ✅ 检测框正常显示（人类蓝色，火焰红色）

**测试方法2：使用curl测试**
```bash
curl -I http://localhost:8080/maixcam_stream
```

**预期响应头：**
```
HTTP/1.1 200 OK
Content-Type: multipart/x-mixed-replace; boundary=frame
Access-Control-Allow-Origin: *
Cache-Control: no-cache
Connection: close
Pragma: no-cache
```

**如果看到错误页面（MJPEG代理不可用）：**
1. 检查MaixCAM的`main.py`是否正常运行
2. 检查MaixCAM IP地址配置（UnifiedServer.js第201行，应为`192.168.31.43`）
3. 尝试直接访问MaixCAM：`http://192.168.31.43:8000`（如果也失败，说明MaixCAM的MJPEG服务器未启动）
4. 查看Web Server日志，确认错误信息

**在Web Server终端观察：**
- 成功时显示：
  ```
  🎥 MJPEG代理请求 - 目标: http://192.168.31.43:8000
  ✅ MJPEG代理连接成功 - 状态码: 200
  ```
- 失败时显示：
  ```
  🎥 MJPEG代理请求 - 目标: http://192.168.31.43:8000
  ❌ MJPEG代理请求失败: connect ETIMEDOUT
     - 目标地址: http://192.168.31.43:8000
     - 可能原因: MaixCAM未启动或网络不可达
  ```

### 测试步骤5: 测试AI检测数据转发

**目标**: 验证Web Server能否接收并转发AI检测数据

**测试方法：使用浏览器开发者工具**
1. 打开浏览器访问：`http://localhost:8080`
2. 按`F12`打开开发者工具
3. 切换到"控制台(Console)"标签
4. 在控制台输入以下代码建立WebSocket连接：
   ```javascript
   const ws = new WebSocket('ws://localhost:8080');
   ws.onopen = () => console.log('✅ WebSocket连接成功');
   ws.onmessage = (event) => {
       const msg = JSON.parse(event.data);
       if (msg.type === 'ai_detection') {
           console.log('🎯 收到AI检测数据:', msg.data.detections);
       }
   };
   ws.onerror = (err) => console.error('❌ WebSocket错误:', err);
   ```

**预期结果**：
- ✅ 控制台显示：`✅ WebSocket连接成功`
- ✅ 当MaixCAM检测到人或火时，控制台显示：
  ```
  🎯 收到AI检测数据: Array(1)
    0: {x: 100, y: 150, w: 200, h: 300, class_id: 0, class_name: "human", score: 0.95}
  ```

**在Web Server终端观察：**
- 应该看到：
  ```
  🎯 接收到AI检测数据: conn_xxxxx, 检测数量: 1
  📨 消息处理完成: 类型=ai_detection, 发送者=conn_xxxxx, 转发=1个客户端
  ```

**如果没有收到AI检测数据：**
1. 检查MaixCAM是否有检测到对象（查看MaixCAM终端输出）
2. 确认WebSocket连接已成功建立
3. 查看Web Server终端，检查是否有错误日志
4. 确认MaixCAM的`main.py`第234行发送逻辑是否正确

---

## ✅ 验收检查清单

### 检查项1: Web Server启动成功

**测试方法：**
- 运行`node server/server.js`
- 观察终端输出

**验收标准：**
- [ ] 服务器成功启动，监听8080端口
- [ ] 显示HTTP和WebSocket服务地址
- [ ] 没有错误信息

---

### 检查项2: MaixCAM WebSocket连接成功

**测试方法：**
- MaixCAM运行`main.py`
- 观察MaixCAM和Web Server的终端输出

**验收标准：**
- [ ] MaixCAM显示：`✅ WebSocket连接成功`
- [ ] Web Server显示：`🔌 新的WebSocket连接请求`
- [ ] 连接保持稳定，无频繁断线重连

---

### 检查项3: MJPEG视频流代理正常

**测试方法：**
- 浏览器访问`http://localhost:8080/maixcam_stream`

**验收标准：**
- [ ] 能看到MaixCAM的实时视频画面
- [ ] 视频流畅，延迟< 500ms
- [ ] 检测框正常显示
- [ ] 无黑屏或加载失败

**备注**: 如果MaixCAM的MJPEG服务器模块不可用，此项测试会失败（这是预期内的，Phase 1已知问题）。

---

### 检查项4: AI检测数据转发正常

**测试方法：**
- 浏览器WebSocket连接测试（步骤5）

**验收标准：**
- [ ] 浏览器能收到`ai_detection`类型消息
- [ ] 消息包含`detections`数组
- [ ] 检测数据格式正确（x, y, w, h, class_name, score字段）
- [ ] 实时性良好，延迟< 200ms

---

## ❌ 常见问题排查

### 问题1: Web Server启动失败 - 端口被占用

**现象：**
```
⚠️  端口 8080 被占用，尝试端口 8081
⚠️  端口 8081 被占用，尝试端口 8082
...
```

**解决方案：**
1. 查找占用8080端口的进程：
   ```bash
   # Windows
   netstat -ano | findstr :8080

   # 找到PID后杀掉进程
   taskkill /F /PID <PID>
   ```
2. 或者修改配置文件`server.config.js`，更改默认端口

---

### 问题2: MaixCAM WebSocket连接失败

**现象：**
```
❌ 连接失败: [Errno 113] Host is unreachable
```

**可能原因：**
1. Web Server未启动
2. 防火墙阻止8080端口
3. MaixCAM和电脑不在同一网络
4. `main.py`中的IP地址配置错误

**解决方案：**
1. 确认Web Server已启动：`netstat -ano | findstr :8080`
2. 关闭防火墙或添加8080端口例外：
   ```bash
   # Windows防火墙
   netsh advfirewall firewall add rule name="Node.js 8080" dir=in action=allow protocol=TCP localport=8080
   ```
3. 使用`ipconfig`确认电脑IP地址
4. 修改`maixcam/main.py`第20行：`WEB_SERVER_IP = "你的电脑IP"`
5. 重启MaixCAM的`main.py`程序

---

### 问题3: MJPEG代理返回503错误

**现象：**
```
❌ MJPEG代理请求失败: connect ETIMEDOUT
   - 目标地址: http://192.168.31.43:8000
   - 可能原因: MaixCAM未启动或网络不可达
```

**解决方案：**
1. 确认MaixCAM的`main.py`正在运行
2. 尝试直接访问MaixCAM：`http://192.168.31.43:8000`
   - 如果能访问，说明是代理配置问题
   - 如果不能访问，说明MaixCAM的MJPEG服务器未启动
3. 检查MaixCAM IP地址是否正确（`UnifiedServer.js`第201行）
4. 检查网络连接（ping 192.168.31.43）

---

### 问题4: 浏览器收不到AI检测数据

**现象：**
- WebSocket连接成功
- 但控制台没有`ai_detection`消息

**可能原因：**
1. MaixCAM没有检测到对象
2. MaixCAM的WebSocket未连接
3. Web Server消息转发有问题

**解决方案：**
1. 确认MaixCAM终端显示检测结果（例如：`🎯 [帧222] 检测结果: 1人, 0火`）
2. 确认MaixCAM终端显示`WebSocket: 已连接`
3. 在摄像头前放置测试对象（人或火焰图片）
4. 查看Web Server终端，确认是否有`🎯 接收到AI检测数据`日志
5. 检查浏览器WebSocket代码是否正确（参考测试步骤5）

---

### 问题5: WebSocket频繁断线重连

**现象：**
```
⚠️⚠️⚠️ WebSocket连接已关闭 ⚠️⚠️⚠️
🔄 WebSocket断开，尝试重连...
```

**可能原因：**
1. 网络不稳定
2. 心跳超时
3. 防火墙干扰

**解决方案：**
1. 检查WiFi信号强度
2. 减少网络负载（关闭其他下载/上传任务）
3. 调整心跳间隔（`server.config.js`第36行）
4. 临时关闭防火墙测试

---

## 📊 测试结果记录

请填写以下测试结果：

- [ ] **Web Server启动**: 成功监听8080端口
- [ ] **MaixCAM WebSocket连接**: 连接成功且稳定
- [ ] **MJPEG视频流代理**: 能正常访问视频流（或模块不可用但程序正常运行）
- [ ] **AI检测数据转发**: 浏览器能收到ai_detection消息
- [ ] **性能**: 视频延迟< 500ms，数据延迟< 200ms
- [ ] **稳定性**: 连续运行5分钟无崩溃或断线

---

## 🎯 Phase 2 完成标准

**必须达成（4项）：**
1. ✅ Web Server成功启动并监听8080端口
2. ✅ MaixCAM能通过WebSocket连接到Web Server
3. ✅ AI检测数据能正常转发给浏览器客户端
4. ✅ MJPEG代理路由已添加（无论MaixCAM的MJPEG服务器是否可用）

**可选达成（2项）：**
1. ⭐ MJPEG视频流代理正常工作（取决于MaixCAM的MJPEG服务器是否可用）
2. ⭐ 连续运行30分钟无错误

---

## 🔄 如何恢复备份

**如果新代码有问题，可以恢复原始代码：**

```bash
# 方法1：直接覆盖
cp web/server/lib/UnifiedServer.js.backup web/server/lib/UnifiedServer.js

# 方法2：重命名
mv web/server/lib/UnifiedServer.js web/server/lib/UnifiedServer.js.new
mv web/server/lib/UnifiedServer.js.backup web/server/lib/UnifiedServer.js
```

**注意**: 如果恢复备份，还需要同时恢复配置文件和MessageHandler.js：
- `web/server/config/server.config.js`（手动还原，删除ai_detection类型）
- `web/server/lib/MessageHandler.js`（手动还原，删除handleAIDetectionMessage方法）

---

## ✅ 测试完成后

**如果所有必须项都通过：**
1. 保持Web Server运行
2. 保持MaixCAM程序运行
3. 告诉我"Phase 2测试通过"
4. 我会开始Phase 3（修改Web前端）

**如果有问题：**
1. 记录错误信息（截图或复制终端输出）
2. 告诉我具体的问题
3. 我会帮你排查

---

**祝测试顺利！有任何问题随时问我。** 🚀
