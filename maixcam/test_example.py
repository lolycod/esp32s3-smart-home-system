#!/usr/bin/env python3
  """最小测试版本 - 排查稳定性问题"""

  from maix import camera, display, image, nn, app
  import time

  print("🤖 初始化YOLOv5模型...")
  detector = nn.YOLOv5(model="/root/models/maixhub/243027/model_243027.mud")
  print("✅ 模型加载成功")

  print("📷 初始化摄像头和显示屏...")
  cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
  dis = display.Display()
  print("✅ 初始化完成")

  print("🎥 开始检测（不连接WebSocket）...")
  frame_count = 0

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
              print(f"📊 已处理{frame_count}帧")

          # 添加短暂延迟降低负载
          time.sleep(0.05)

  except KeyboardInterrupt:
      print("\n⚠️ 收到中断信号")
  except Exception as e:
      print(f"\n❌ 发生错误: {e}")
      import traceback
      traceback.print_exc()
  finally:
      print("✅ 程序退出")