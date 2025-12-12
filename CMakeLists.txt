# ESP32项目CMake配置文件
cmake_minimum_required(VERSION 3.16)

# 设置额外的组件目录（五层架构）
set(EXTRA_COMPONENT_DIRS
    "components/hal"           # 硬件抽象层
    "components/drivers"       # 驱动层
    "components/services"      # 服务层
    "components/middleware"    # 中间件层
    "components/application"   # 应用层
)

# 包含ESP-IDF的项目配置
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 定义项目
project(sample_project)
