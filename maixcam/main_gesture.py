#!/usr/bin/env python3
"""
MaixCAM视觉检测系统 - 增强版（手势识别+目标检测）
功能：
1. YOLOv5目标检测（人类和火焰）
2. 手势识别控制（基于21点手部关键点）
3. WebSocket实时传输：检测结果（JSON）+ 图像帧（JPEG Binary）
4. 本地屏幕显示（带检测框和标签）

手势映射：
- 👆 单指指向上 → LED灯开
- ✌️ 比V手势 → 风扇开
- 👋 五指张开 → 全部关闭
- 👍 竖大拇指 → 窗帘开
- ✊ 握拳 → 安防模式

需要MaixPy固件版本 >= 4.9.3
"""

from maix import camera, display, image, nn, app
import socket
import json
import time
import os
import gc
import math

# ========================================
# 配置区域 - 请根据实际情况修改
# ========================================
WEB_SERVER_IP = "192.168.31.150"  # ⚠️ 改成你的电脑IP地址！
WEB_SERVER_PORT = 8080            # Web Server的WebSocket端口

# 传输配置
DETECTION_SEND_INTERVAL = 0.5     # 检测结果发送间隔（秒），2Hz
FRAME_SEND_INTERVAL = 0.1         # 图像帧发送间隔（秒），10 FPS
GESTURE_SEND_INTERVAL = 0.3       # 手势识别发送间隔（秒）
JPEG_QUALITY = 70                 # JPEG压缩质量

# 手势识别配置
GESTURE_CONFIDENCE_TH = 0.7       # 手势检测置信度阈值
GESTURE_STABLE_FRAMES = 3         # 手势稳定帧数（避免误触发）

# ========================================
# 初始化YOLOv5模型（目标检测）
# ========================================
print("🤖 初始化YOLOv5目标检测模型...")
yolo_model_path = "model_243027.mud"
if not os.path.exists(yolo_model_path):
    yolo_model_path = "/root/models/maixhub/243027/model_243027.mud"
yolo_detector = nn.YOLOv5(model=yolo_model_path)
print(f"✅ YOLOv5模型加载成功: {yolo_model_path}")
print(f"   - 检测类别: {yolo_detector.labels}")

# ========================================
# 初始化手部关键点检测模型
# ========================================
print("✋ 初始化手部关键点检测模型...")
hand_model_path = "/root/models/hand_landmarks.mud"
try:
    hand_detector = nn.HandLandmarks(model=hand_model_path)
    print(f"✅ 手部关键点模型加载成功")
    HAND_DETECTION_ENABLED = True
except Exception as e:
    print(f"⚠️ 手部关键点模型加载失败: {e}")
    print("   手势识别功能将被禁用")
    hand_detector = None
    HAND_DETECTION_ENABLED = False

# ========================================
# 初始化摄像头和显示屏
# ========================================
print("📷 初始化摄像头和显示屏...")
cam = camera.Camera(320, 224, yolo_detector.input_format())
dis = display.Display()
print("✅ 摄像头和显示屏初始化完成")

# ========================================
# 手势识别算法
# ========================================
def calculate_distance(p1, p2):
    """计算两点间距离"""
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

def is_finger_extended(landmarks, finger_idx):
    """
    判断手指是否伸展
    landmarks: 21个关键点 [(x,y,z), ...]
    finger_idx: 0=拇指, 1=食指, 2=中指, 3=无名指, 4=小指
    """
    # 手指关键点索引映射 (MCP, PIP, DIP, TIP)
    finger_tips = [4, 8, 12, 16, 20]  # 指尖
    finger_pips = [3, 6, 10, 14, 18]  # 近指节
    finger_mcps = [2, 5, 9, 13, 17]   # 掌指关节
    
    tip = landmarks[finger_tips[finger_idx]]
    pip = landmarks[finger_pips[finger_idx]]
    mcp = landmarks[finger_mcps[finger_idx]]
    
    # 拇指特殊处理（横向判断）
    if finger_idx == 0:
        return tip[0] > pip[0] + 10  # 右手拇指向右伸展
    else:
        # 其他手指：指尖Y坐标小于近指节（Y轴向下）
        return tip[1] < pip[1] - 10

def recognize_gesture(landmarks):
    """
    基于21个手部关键点识别手势
    返回: (手势名称, 置信度)
    """
    if len(landmarks) < 21:
        return ("unknown", 0.0)
    
    # 判断每个手指是否伸展
    fingers_extended = []
    for i in range(5):
        try:
            extended = is_finger_extended(landmarks, i)
            fingers_extended.append(extended)
        except:
            fingers_extended.append(False)
    
    thumb, index, middle, ring, pinky = fingers_extended
    
    # 手势识别规则
    # ✊ 握拳：所有手指都不伸展
    if not any(fingers_extended):
        return ("fist", 0.95)  # 握拳 → 安防模式
    
    # 👋 五指张开：所有手指都伸展
    if all(fingers_extended):
        return ("open_palm", 0.95)  # 张开手掌 → 全部关闭
    
    # 👆 单指指向上：只有食指伸展
    if index and not middle and not ring and not pinky:
        return ("point_up", 0.9)  # 指向上 → LED灯开
    
    # ✌️ 比V手势：食指和中指伸展
    if index and middle and not ring and not pinky:
        return ("victory", 0.9)  # V手势 → 风扇开
    
    # 👍 竖大拇指：只有拇指伸展
    if thumb and not index and not middle and not ring and not pinky:
        return ("thumbs_up", 0.9)  # 大拇指 → 窗帘开
    
    # 🤟 摇滚手势：拇指、食指、小指伸展
    if thumb and index and not middle and not ring and pinky:
        return ("rock", 0.85)  # 摇滚 → 特殊模式
    
    return ("unknown", 0.0)

def parse_hand_landmarks(obj):
    """
    从检测结果中解析21个手部关键点
    返回: [(x, y, z), ...] 共21个点
    """
    landmarks = []
    points = obj.points
    
    # 前8个值是手框的4个角坐标，跳过
    # 后面是21个关键点，每个点有x,y,z三个值
    for i in range(21):
        base_idx = 8 + i * 3
        if base_idx + 2 < len(points):
            x = points[base_idx]
            y = points[base_idx + 1]
            z = points[base_idx + 2]
            landmarks.append((x, y, z))
    
    return landmarks

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

        # 等待握手响应
        response = sock.recv(1024).decode()
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
print(f"   - 手势识别: {'启用' if HAND_DETECTION_ENABLED else '禁用'}")
print("="*50 + "\n")

frame_count = 0
last_detection_send_time = time.time()
last_frame_send_time = time.time()
last_gesture_send_time = time.time()
last_gc_time = time.time()
detection_count = {"human": 0, "fire": 0}

# 手势稳定计数器（避免误触发）
gesture_history = []
last_sent_gesture = None

try:
    while not app.need_exit():
        # 读取摄像头图像
        img = cam.read()

        # ========================================
        # 1. YOLOv5目标检测（人/火焰）
        # ========================================
        yolo_objs = yolo_detector.detect(img, conf_th=0.5, iou_th=0.45)

        # 统计检测结果
        detection_count = {"human": 0, "fire": 0}
        for obj in yolo_objs:
            if obj.class_id == 0:
                detection_count["human"] += 1
            else:
                detection_count["fire"] += 1

        # 绘制目标检测框
        for obj in yolo_objs:
            color = image.COLOR_BLUE if obj.class_id == 0 else image.COLOR_RED
            img.draw_rect(obj.x, obj.y, obj.w, obj.h, color=color, thickness=2)
            label = f'{yolo_detector.labels[obj.class_id]}: {obj.score:.2f}'
            img.draw_string(obj.x, obj.y - 10, label, color=color, scale=1.5)

        # ========================================
        # 2. 手势识别
        # ========================================
        current_gesture = None
        gesture_confidence = 0.0
        
        if HAND_DETECTION_ENABLED and hand_detector:
            try:
                hand_objs = hand_detector.detect(img, conf_th=GESTURE_CONFIDENCE_TH, iou_th=0.45, conf_th2=0.8)
                
                for hand_obj in hand_objs:
                    # 绘制手部关键点
                    hand_detector.draw_hand(img, hand_obj.class_id, hand_obj.points, 2, 6, box=True)
                    
                    # 解析关键点
                    landmarks = parse_hand_landmarks(hand_obj)
                    
                    if len(landmarks) >= 21:
                        # 识别手势
                        gesture, confidence = recognize_gesture(landmarks)
                        
                        if gesture != "unknown" and confidence > 0.8:
                            current_gesture = gesture
                            gesture_confidence = confidence
                            
                            # 显示手势标签
                            gesture_labels = {
                                "fist": "✊ 安防模式",
                                "open_palm": "👋 全部关闭",
                                "point_up": "👆 LED灯开",
                                "victory": "✌️ 风扇开",
                                "thumbs_up": "👍 窗帘开",
                                "rock": "🤟 特殊模式"
                            }
                            label_text = gesture_labels.get(gesture, gesture)
                            img.draw_string(10, 10, label_text, color=image.COLOR_GREEN, scale=2)
                            
            except Exception as e:
                pass  # 静默处理手势检测错误

        # 显示到本地屏幕
        dis.show(img)

        current_time = time.time()

        # ========================================
        # 发送图像帧
        # ========================================
        if current_time - last_frame_send_time >= FRAME_SEND_INTERVAL:
            try:
                jpeg_img = img.to_jpeg(quality=JPEG_QUALITY)
                jpeg_data = jpeg_img.to_bytes()

                if ws_sock:
                    if send_binary_frame(ws_sock, jpeg_data):
                        last_frame_send_time = current_time
                    else:
                        print("⚠️  图像发送失败，尝试重连...")
                        ws_sock = connect_websocket()
            except Exception as e:
                print(f"❌ 图像处理错误: {e}")

        # ========================================
        # 发送手势识别结果（带稳定性检测）
        # ========================================
        if current_time - last_gesture_send_time >= GESTURE_SEND_INTERVAL:
            if current_gesture:
                # 添加到历史记录
                gesture_history.append(current_gesture)
                if len(gesture_history) > GESTURE_STABLE_FRAMES:
                    gesture_history.pop(0)
                
                # 检查手势是否稳定（连续N帧相同）
                if len(gesture_history) >= GESTURE_STABLE_FRAMES:
                    if all(g == current_gesture for g in gesture_history):
                        # 手势稳定，且与上次发送不同，才发送
                        if current_gesture != last_sent_gesture:
                            gesture_message = {
                                "type": "gesture_control",
                                "timestamp": int(time.time() * 1000),
                                "data": {
                                    "gesture": current_gesture,
                                    "confidence": gesture_confidence,
                                    "action": get_gesture_action(current_gesture)
                                }
                            }
                            
                            if ws_sock and send_text_message(ws_sock, gesture_message):
                                print(f"✋ 手势识别: {current_gesture} (置信度: {gesture_confidence:.2f})")
                                last_sent_gesture = current_gesture
                                gesture_history.clear()  # 重置历史
            else:
                # 没有检测到手势，重置状态
                if len(gesture_history) > 0:
                    gesture_history.clear()
                    last_sent_gesture = None
            
            last_gesture_send_time = current_time

        # ========================================
        # 发送目标检测结果
        # ========================================
        if current_time - last_detection_send_time >= DETECTION_SEND_INTERVAL:
            detections = []
            for obj in yolo_objs:
                detections.append({
                    "x": obj.x,
                    "y": obj.y,
                    "w": obj.w,
                    "h": obj.h,
                    "class_id": obj.class_id,
                    "class_name": yolo_detector.labels[obj.class_id],
                    "score": obj.score
                })

            message = {
                "type": "ai_detection",
                "timestamp": int(time.time() * 1000),
                "data": {
                    "detections": detections,
                    "frame_width": yolo_detector.input_width(),
                    "frame_height": yolo_detector.input_height()
                }
            }

            if ws_sock:
                if not send_text_message(ws_sock, message):
                    print("⚠️  检测数据发送失败，尝试重连...")
                    ws_sock = connect_websocket()
                else:
                    if len(yolo_objs) > 0:
                        print(f"🎯 [帧{frame_count}] 检测: {detection_count['human']}人, {detection_count['fire']}火")

            last_detection_send_time = current_time

        frame_count += 1

        # 每100帧显示统计
        if frame_count % 100 == 0:
            print(f"📊 已处理{frame_count}帧")

        # 定期垃圾回收
        if current_time - last_gc_time >= 10.0:
            gc.collect()
            last_gc_time = current_time

except KeyboardInterrupt:
    print("\n⚠️  收到中断信号，正在退出...")
except Exception as e:
    print(f"\n❌ 发生错误: {e}")
    import traceback
    traceback.print_exc()
finally:
    print("\n🧹 清理资源...")
    if ws_sock:
        ws_sock.close()
        print("   - WebSocket已关闭")
    print("✅ 程序退出")

def get_gesture_action(gesture):
    """将手势映射到设备控制动作"""
    gesture_actions = {
        "point_up": {"device": "LED灯", "action": {"开关": "开", "理由": "手势控制开灯"}},
        "victory": {"device": "风扇", "action": {"开关": "开", "风速": 3, "理由": "手势控制开风扇"}},
        "open_palm": {"device": "all", "action": {"开关": "关", "理由": "手势控制全部关闭"}},
        "thumbs_up": {"device": "窗帘", "action": {"开关": "开", "理由": "手势控制开窗帘"}},
        "fist": {"device": "security", "action": {"mode": "armed", "理由": "手势激活安防模式"}},
        "rock": {"device": "scene", "action": {"scene": "party", "理由": "手势激活派对模式"}}
    }
    return gesture_actions.get(gesture, {"device": "unknown", "action": {}})
