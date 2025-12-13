#!/usr/bin/env python3
"""极简测试版本 - 只保留核心功能，排查稳定性问题"""

from maix import camera, display, image, nn, app
import time
import gc  # 垃圾回收

print("=" * 50)
print("🤖 极简测试模式")
print("=" * 50)

# 初始化模型
print("📥 加载模型...")
try:
    detector = nn.YOLOv5(model="/root/models/maixhub/243027/model_243027.mud")
    print("✅ 模型加载成功")
except Exception as e:
    print(f"❌ 模型加载失败: {e}")
    exit(1)

# 初始化摄像头和显示
print("📷 初始化摄像头...")
try:
    cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
    dis = display.Display()
    print("✅ 摄像头和显示初始化完成")
except Exception as e:
    print(f"❌ 初始化失败: {e}")
    exit(1)

print("\n🎥 开始检测...\n")

frame_count = 0
start_time = time.time()
last_gc_time = time.time()

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

        # 每100帧统计一次
        if frame_count % 100 == 0:
            elapsed = time.time() - start_time
            fps = frame_count / elapsed
            print(f"📊 已处理{frame_count}帧 | 运行: {elapsed:.1f}秒 | FPS: {fps:.1f}")

        # 定期强制垃圾回收（每10秒）
        current_time = time.time()
        if current_time - last_gc_time >= 10.0:
            gc.collect()
            print(f"🧹 [帧{frame_count}] 执行垃圾回收")
            last_gc_time = current_time

        # 短暂延迟，降低CPU占用
        time.sleep(0.01)

except KeyboardInterrupt:
    print("\n⚠️  收到中断信号")
except Exception as e:
    print(f"\n❌ 发生错误: {e}")
    import traceback
    traceback.print_exc()
finally:
    total_time = time.time() - start_time
    print(f"\n✅ 程序退出")
    print(f"   总运行时间: {total_time:.1f}秒")
    print(f"   总帧数: {frame_count}")
    print(f"   平均FPS: {frame_count/total_time:.1f}")
