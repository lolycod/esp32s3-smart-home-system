#!/usr/bin/env python3
"""最小测试版本 - 排查稳定性问题"""

from maix import camera, display, image, nn, app
import time
import sys

# 重定向输出到文件
log_file = open("/root/test_log.txt", "w", buffering=1)
def log(msg):
    print(msg)
    log_file.write(msg + "\n")
    log_file.flush()

log("=" * 50)
log("🤖 初始化YOLOv5模型...")
try:
    detector = nn.YOLOv5(model="/root/models/maixhub/243027/model_243027.mud")
    log("✅ 模型加载成功")
except Exception as e:
    log(f"❌ 模型加载失败: {e}")
    sys.exit(1)

log("📷 初始化摄像头和显示屏...")
try:
    cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
    dis = display.Display()
    log("✅ 摄像头和显示屏初始化完成")
except Exception as e:
    log(f"❌ 初始化失败: {e}")
    sys.exit(1)

log("🎥 开始检测（不连接WebSocket）...")
log("=" * 50)

frame_count = 0
start_time = time.time()

try:
    while not app.need_exit():
        # 读取图像
        img = cam.read()

        # YOLOv5检测
        objs = detector.detect(img, conf_th=0.5, iou_th=0.45)

        # 绘制检测框
        for obj in objs:
            color = image.COLOR_BLUE if obj.class_id == 0 else image.COLOR_RED
            img.draw_rect(obj.x, obj.y, obj.w, obj.h, color=color, thickness=2)
            label = f'{detector.labels[obj.class_id]}: {obj.score:.2f}'
            img.draw_string(obj.x, obj.y - 10, label, color=color, scale=1.5)

        # 显示
        dis.show(img)

        frame_count += 1
        if frame_count % 100 == 0:
            elapsed = time.time() - start_time
            fps = frame_count / elapsed
            log(f"📊 已处理{frame_count}帧，运行时间: {elapsed:.1f}秒，FPS: {fps:.1f}")

        # 添加短暂延迟降低负载
        time.sleep(0.05)

except KeyboardInterrupt:
    log("\n⚠️ 收到中断信号")
except Exception as e:
    log(f"\n❌ 发生错误: {e}")
    import traceback
    traceback.print_exc()
    traceback.print_exc(file=log_file)
finally:
    total_time = time.time() - start_time
    log(f"✅ 程序退出，总运行时间: {total_time:.1f}秒，总帧数: {frame_count}")
    log_file.close()
