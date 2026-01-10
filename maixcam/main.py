#!/usr/bin/env python3
"""
MaixCAM视觉检测系统 - 主程序（优化版）
功能：
1. YOLOv5目标检测（人类和火焰）
2. WebSocket实时传输：检测结果（JSON）+ 图像帧（JPEG Binary）
3. 本地屏幕显示（带检测框和标签）
4. 固定帧率传输，避免延时积累
"""

from maix import camera, display, image, nn, app
import socket
import json
import time
import os
import gc

# ========================================
# 配置区域 - 请根据实际情况修改
# ========================================
WEB_SERVER_IP = "192.168.31.150"  # ⚠️ 改成你的电脑IP地址！
WEB_SERVER_PORT = 8080            # Web Server的WebSocket端口

# 传输配置
DETECTION_SEND_INTERVAL = 0.5     # 检测结果发送间隔（秒），2Hz
FRAME_SEND_INTERVAL = 0.1         # 图像帧发送间隔（秒），10 FPS
JPEG_QUALITY = 70                 # JPEG压缩质量（60-80平衡质量和大小）

# ========================================
# 初始化YOLOv5模型
# ========================================
print("🤖 初始化YOLOv5模型...")
model_path = "model_243027.mud"
if not os.path.exists(model_path):
    model_path = "/root/models/maixhub/243027/model_243027.mud"
detector = nn.YOLOv5(model=model_path)
print(f"✅ 模型加载成功: {model_path}")
print(f"   - 检测类别: {detector.labels}")
print(f"   - 输入分辨率: {detector.input_width()}x{detector.input_height()}")

# ========================================
# 初始化摄像头和显示屏
# ========================================
print("📷 初始化摄像头和显示屏...")
cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
dis = display.Display()
print("✅ 摄像头和显示屏初始化完成")

# ========================================
# WebSocket辅助函数
# ========================================

def connect_websocket():
    """建立WebSocket连接"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5.0)
        sock.connect((WEB_SERVER_IP, WEB_SERVER_PORT))

        # 发送WebSocket握手请求
        handshake = (
            f"GET / HTTP/1.1\r\n"
            f"Host: {WEB_SERVER_IP}:{WEB_SERVER_PORT}\r\n"
            f"Upgrade: websocket\r\n"
            f"Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            f"Sec-WebSocket-Version: 13\r\n"
            f"\r\n"
        )
        sock.sendall(handshake.encode())

        response = sock.recv(1024).decode('utf-8', errors='ignore')  # 忽略解码错误

        if "101 Switching Protocols" in response:
            print(f"✅ WebSocket连接成功")
            return sock
        else:
            print(f"❌ WebSocket握手失败")
            sock.close()
            return None
    except Exception as e:
        print(f"❌ 连接失败: {e}")
        return None

def send_text_message(sock, message_dict):
    """发送WebSocket文本消息（带mask，符合RFC 6455规范）"""
    if not sock:
        return False

    try:
        json_str = json.dumps(message_dict)
        payload = json_str.encode('utf-8')

        # 构建WebSocket文本帧（opcode=0x81，带掩码）
        frame = bytearray([0x81])  # FIN=1, Opcode=1(文本)

        payload_len = len(payload)
        if payload_len < 126:
            frame.append(0x80 | payload_len)
        elif payload_len < 65536:
            frame.append(0x80 | 126)
            frame.extend(payload_len.to_bytes(2, 'big'))
        else:
            frame.append(0x80 | 127)
            frame.extend(payload_len.to_bytes(8, 'big'))

        # 生成4字节masking key
        masking_key = os.urandom(4)
        frame.extend(masking_key)

        # Mask payload
        masked_payload = bytearray(payload)
        for i in range(len(masked_payload)):
            masked_payload[i] ^= masking_key[i % 4]

        frame.extend(masked_payload)
        sock.sendall(frame)
        return True
    except Exception as e:
        print(f"❌ 发送文本消息失败: {e}")
        return False

def send_binary_frame(sock, jpeg_data):
    """发送WebSocket二进制帧（JPEG图像）"""
    if not sock:
        return False

    try:
        # 构建WebSocket二进制帧（opcode=0x82，带掩码）
        frame = bytearray([0x82])  # FIN=1, Opcode=2(二进制)

        payload_len = len(jpeg_data)
        if payload_len < 126:
            frame.append(0x80 | payload_len)
        elif payload_len < 65536:
            frame.append(0x80 | 126)
            frame.extend(payload_len.to_bytes(2, 'big'))
        else:
            frame.append(0x80 | 127)
            frame.extend(payload_len.to_bytes(8, 'big'))

        # 生成4字节masking key
        masking_key = os.urandom(4)
        frame.extend(masking_key)

        # Mask payload
        masked_payload = bytearray(jpeg_data)
        for i in range(len(masked_payload)):
            masked_payload[i] ^= masking_key[i % 4]

        frame.extend(masked_payload)
        sock.sendall(frame)
        return True
    except Exception as e:
        print(f"❌ 发送图像帧失败: {e}")
        return False

# ========================================
# 连接到Web Server
# ========================================
print(f"🔗 连接到Web Server: {WEB_SERVER_IP}:{WEB_SERVER_PORT}")
ws_sock = connect_websocket()

# ========================================
# 主检测循环
# ========================================
print("\n" + "="*50)
print("🎥 系统启动，开始检测...")
print("="*50)
print(f"📊 传输参数:")
print(f"   - 检测数据: {1/DETECTION_SEND_INTERVAL:.1f} Hz")
print(f"   - 图像帧: {1/FRAME_SEND_INTERVAL:.1f} FPS")
print(f"   - JPEG质量: {JPEG_QUALITY}")
print("="*50 + "\n")

frame_count = 0
last_detection_send_time = time.time()
last_frame_send_time = time.time()
last_gc_time = time.time()
detection_count = {"human": 0, "fire": 0}

try:
    while not app.need_exit():
        # 读取摄像头图像
        img = cam.read()

        # YOLOv5检测
        objs = detector.detect(img, conf_th=0.5, iou_th=0.45)

        # 统计检测结果
        detection_count = {"human": 0, "fire": 0}
        for obj in objs:
            if obj.class_id == 0:
                detection_count["human"] += 1
            else:
                detection_count["fire"] += 1

        # 绘制检测框到图像上
        for obj in objs:
            color = image.COLOR_BLUE if obj.class_id == 0 else image.COLOR_RED
            img.draw_rect(obj.x, obj.y, obj.w, obj.h, color=color, thickness=2)
            label = f'{detector.labels[obj.class_id]}: {obj.score:.2f}'
            img.draw_string(obj.x, obj.y - 10, label, color=color, scale=1.5)

        # 显示到本地屏幕（恢复每帧显示）
        dis.show(img)

        current_time = time.time()

        # 固定间隔发送图像帧（10 FPS，避免延时积累）
        if current_time - last_frame_send_time >= FRAME_SEND_INTERVAL:
            try:
                # 将图像转为JPEG并获取字节流
                jpeg_img = img.to_jpeg(quality=JPEG_QUALITY)
                jpeg_data = jpeg_img.to_bytes()  # 获取字节流

                # 发送binary frame
                if ws_sock:
                    if send_binary_frame(ws_sock, jpeg_data):
                        last_frame_send_time = current_time
                    else:
                        # 发送失败，尝试重连
                        print("⚠️  图像发送失败，尝试重连...")
                        ws_sock = connect_websocket()
            except Exception as e:
                print(f"❌ 图像处理错误: {e}")

        # 固定间隔发送检测结果（2 Hz）
        if current_time - last_detection_send_time >= DETECTION_SEND_INTERVAL:
            # 构建检测结果JSON（即使没有检测到物体也发送）
            detections = []
            for obj in objs:
                detections.append({
                    "x": obj.x,
                    "y": obj.y,
                    "w": obj.w,
                    "h": obj.h,
                    "class_id": obj.class_id,
                    "class_name": detector.labels[obj.class_id],
                    "score": obj.score
                })

            message = {
                "type": "ai_detection",
                "timestamp": int(time.time() * 1000),
                "data": {
                    "detections": detections,
                    "frame_width": detector.input_width(),
                    "frame_height": detector.input_height()
                }
            }

            # 发送检测结果（总是发送，即使为空）
            if ws_sock:
                if not send_text_message(ws_sock, message):
                    # 发送失败，尝试重连
                    print("⚠️  检测数据发送失败，尝试重连...")
                    ws_sock = connect_websocket()
                else:
                    # 发送成功，记录日志（只在有检测时输出到终端）
                    if len(objs) > 0:
                        ws_status = "已连接" if ws_sock else "未连接"
                        print(f"🎯 [帧{frame_count}] 检测结果: {detection_count['human']}人, {detection_count['fire']}火 | WebSocket: {ws_status}")

            last_detection_send_time = current_time

        frame_count += 1

        # 每100帧显示统计
        if frame_count % 100 == 0:
            print(f"📊 已处理{frame_count}帧 | 当前检测: {detection_count['human']}人, {detection_count['fire']}火")

        # 定期垃圾回收（每10秒）
        if current_time - last_gc_time >= 10.0:
            gc.collect()
            print(f"🧹 [帧{frame_count}] 执行垃圾回收")
            last_gc_time = current_time

except KeyboardInterrupt:
    print("\n⚠️  收到中断信号，正在退出...")
except Exception as e:
    print(f"\n❌ 发生错误: {e}")
    import traceback
    traceback.print_exc()
finally:
    # 清理资源
    print("\n🧹 清理资源...")
    if ws_sock:
        ws_sock.close()
        print("   - WebSocket已关闭")
    print("✅ 程序退出")
