# ESP32-S3-WROOM-1  
ESP32-S3-WROOM-1U

技术规格书

2.4 GHz Wi-Fi (802.11 b/g/n) + Bluetooth® 5 (LE) 模组  
内置 ESP32-S3 系列芯片，Xtensa® 双核 32 位 LX7 处理器  
Flash 最大可选 16 MB，PSRAM 最大可选 16 MB  
最多 36 个 GPIO，丰富的外设  
板载 PCB 天线或外部天线连接器

![](images/682b57d60d72e9d02711ab4bc17a76e2d25ad1428577e21672cdd74f613ba700.jpg)  
ESP32-S3-WROOM-1

![](images/c1110bca9c6c558ae760489a9a21b0cc2e18975e0fb222f6b6d68ef20a411408.jpg)  
ESP32-S3-WROOM-1U

# 1 模组概述

说明：

点击链接或扫描二维码确保您使用的是最新版本的文档：

https://www.espressif.com/documentation/esp32-s3-wroom-1_wroom-1u datasheet_cn.pdf

![](images/454d4ea86d367abc77f571390c00b201c773d1c5f8346f9e30517478d392a9c6.jpg)

# 1.1 特性

# CPU和片上存储器

- 内置 ESP32-S3 系列芯片，Xtensa® 双核 32 位 LX7 微处理器 (支持单精度浮点运算单元)，支持高达 240 MHz 的时钟频率  
384 KB ROM  
- 512 KB SRAM  
16 KB RTC SRAM  
- 最大 16 MB PSRAM

# Wi-Fi

- 802.11 b/g/n  
- 802.11n 模式下数据速率高达 150 Mbps  
- 帧聚合 (TX/RX A-MPDU, TX/RX A-MSDU)  
-  ${0.4\mu }\mathrm{s}$  保护间隔  
- 工作信道中心频率范围：2412 ~ 2484 MHz

# 蓝牙

- 低功耗蓝牙 (Bluetooth LE): Bluetooth 5、Bluetooth mesh  
- 速率支持 125 Kbps、500 Kbps、1 Mbps、2 Mbps  
- 广播扩展(Advertising Extensions)  
- 多广播 (Multiple Advertisement Sets)  
- 信道选择 (Channel Selection Algorithm #2)  
- Wi-Fi 与蓝牙共存，共用同一个天线

# 外设

- GPIO、SPI、LCD、Camera 接口、UART、I2C、I2S、红外遥控、脉冲计数器、LED PWM、全速 USB 2.0 OTG、USB Serial/JTAG 控制器、MCPWM、

SDIO 主机接口、GDMA、TWAI® 控制器（兼容 ISO 11898-1）、ADC、触摸传感器、温度传感器、定时器和看门狗

# 说明：

* 有关模组外设的详细信息，请参考《ESP32-S3 系列芯片技术规格书》。

# 模组集成元件

- 40MHz集成晶振  
- 最大 16 MB Quad SPI flash

# 天线选型

- 板载PCB天线(ESP32-S3-WROOM-1)  
通过连接器连接外部天线 (ESP32-S3-WROOM-1U)

# 工作条件

- 工作电压/供电电压： $3.0 \sim 3.6\mathrm{V}$  
- 工作环境温度：
-  $65^{\circ} \mathrm{C}$  版模组： $-40 \sim 65^{\circ} \mathrm{C}$ 
-  $85^{\circ} \mathrm{C}$  版模组： $-40 \sim 85^{\circ} \mathrm{C}$ 
-  $105^{\circ} \mathrm{C}$  版模组： $-40 \sim 105^{\circ} \mathrm{C}$

# 认证

- RF认证：见证书  
- 环保认证：RoHS/REACH

# 测试

- HTOL/HTSL/uHAST/TCT/ESD

# 1.2 描述

ESP32-S3-WROOM-1和ESP32-S3-WROOM-1U是两款通用型Wi-Fi+低功耗蓝牙MCU模组，搭载ESP32-S3系列芯片。除具有丰富的外设接口外，模组还拥有强大的神经网络运算能力和信号处理能力，适用于AloT领域的多种应用场景，例如唤醒词检测和语音命令识别、人脸检测和识别、智能家居、智能家电、智能控制面板、智能扬声器等。

ESP32-S3-WROOM-1采用PCB板载天线，ESP32-S3-WROOM-1U采用连接器连接外部天线。两款模组均有多种型号可供选择，具体见表1和2。其中，ESP32-S3-WROOM-1-H4和ESP32-S3-WROOM-1U-H4的工作环境温度为  $-40\sim 105^{\circ}\mathrm{C}$ ，内置ESP32-S3R8和ESP32-S3R16V的模组工作环境温度为  $-40\sim 65^{\circ}\mathrm{C}$ ，其他型号的工作环境温度均为  $-40\sim 85^{\circ}\mathrm{C}$  。请注意，针对R8和R16V系列模组(内置OctalSPIPSRAM)，若开启PSRAMECC功能，模组最大环境温度可以提高到  $85^{\circ}\mathrm{C}$ ，但是PSRAM的可用容量将减少1/16。

表 1: ESP32-S3-WROOM-1 系列型号对比  ${}^{1}$  

<table><tr><td>订购代码2</td><td>Flash3,4</td><td>PSRAM5</td><td>环境温度6(℃)</td><td>模组尺寸7(mm)</td></tr><tr><td>ESP32-S3-WROOM-1-N4</td><td>4 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td><td rowspan="11">18.0×25.5×3.1</td></tr><tr><td>ESP32-S3-WROOM-1-N8</td><td>8 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1-N16</td><td>16 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1-H4</td><td>4 MB (Quad SPI)</td><td>-</td><td>-40 ~ 105</td></tr><tr><td>ESP32-S3-WROOM-1-N4R2</td><td>4 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1-N8R2</td><td>8 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1-N16R2</td><td>16 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1-N4R8</td><td>4 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1-N8R8</td><td>8 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1-N16R8</td><td>16 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1-N16R16V8</td><td>16 MB (Quad SPI)</td><td>16 MB (Octal SPI)</td><td>-40 ~ 65</td></tr></table>

1 本表格中的注释内容与表2一致。

表 2: ESP32-S3-WROOM-1U 系列型号对比  

<table><tr><td>订购代码2</td><td>Flash3,4</td><td>PSRAM5</td><td>环境温度6(℃)</td><td>模组尺寸7(mm)</td></tr><tr><td>ESP32-S3-WROOM-1U-N4</td><td>4 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td><td rowspan="11">18.0×19.2×3.2</td></tr><tr><td>ESP32-S3-WROOM-1U-N8</td><td>8 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1U-N16</td><td>16 MB (Quad SPI)</td><td>-</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1U-H4</td><td>4 MB (Quad SPI)</td><td>-</td><td>-40 ~ 105</td></tr><tr><td>ESP32-S3-WROOM-1U-N4R2</td><td>4 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1U-N8R2</td><td>8 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1U-N16R2</td><td>16 MB (Quad SPI)</td><td>2 MB (Quad SPI)</td><td>-40 ~ 85</td></tr><tr><td>ESP32-S3-WROOM-1U-N4R8</td><td>4 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1U-N8R8</td><td>8 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1U-N16R8</td><td>16 MB (Quad SPI)</td><td>8 MB (Octal SPI)</td><td>-40 ~ 65</td></tr><tr><td>ESP32-S3-WROOM-1U-N16R16V8</td><td>16 MB (Quad SPI)</td><td>16 MB (Octal SPI)</td><td>-40 ~ 65</td></tr></table>

2 如需定制 ESP32-S3-WROOM-1-H4、ESP32-S3-WROOM-1U-H4 或 ESP32-S3-WROOM-1UN16R16V 模组，请联系我们。  
3 默认情况下，模组 SPI flash 支持的最大时钟频率为  $80 \mathrm{MHz}$ ，且不支持自动暂停功能。如需使用 120 MHz 的 flash 时钟频率或需要 flash 自动暂停功能，请联系我们。  
4 Flash 支持:

- 至少10万次编程/擦除周期  
- 至少20年数据保留时间

5 该模组使用封装在芯片中的 PSRAM。

6 环境温度指乐鑫模组外部的推荐环境温度。  
7 更多关于模组尺寸的信息，请参考章节 7.1 模组尺寸。  
8 注意, 仅 ESP32-S3-WROOM-1-N16R16V 和 ESP32-S3-WROOM-1U-N16R16V 的 VDD_SPI 电压为  $1.8 \mathrm{~V}$  。

两款模组采用的是 ESP32-S3 系列 * 芯片。ESP32-S3 系列芯片搭载 Xtensa® 32 位 LX7 双核处理器（支持单精度浮点运算单元），工作频率高达  $240 \mathrm{MHz}$  。CPU 电源可被关闭，利用低功耗协处理器监测外设的状态变化或某些模拟量是否超出阈值。

ESP32-S3 集成了丰富的外设，包括模组接口：SPI、LCD、Camera 接口、UART、I2C、I2S、红外遥控、脉冲计数器、LED PWM、USB Serial/JTAG 控制器、MCPWM、SDIO host、GDMA、TWAI® 控制器（兼容 ISO 11898-1）、ADC、触摸传感器、温度传感器、定时器和看门狗，和多达 45 个 GPIO。此外，ESP32-S3 还有一个全速 USB 2.0 On-The-Go (OTG) 接口用于 USB 通信。

# 说明：

* 关于 ESP32-S3 的更多信息请参考 《ESP32-S3 系列芯片技术规格书》。

# 1.3 应用

- 通用低功耗 IoT 传感器集线器  
- 通用低功耗 IoT 数据记录器  
摄像头视频流传输  
- OTT 电视盒/机顶盒设备  
- USB 设备  
- 语音识别  
图像识别  
- Mesh网络  
- 家庭自动化

- 智慧楼宇  
- 工业自动化  
智慧农业  
- 音频设备  
- 健康/医疗/看护  
- Wi-Fi 玩具  
- 可穿戴电子产品  
- 零售 & 餐饮

# 目录

# 1 模组概述 2

1.1 特性 2  
1.2 描述 3  
1.3 应用 4

# 2 功能框图 9

# 3 管脚定义 10

3.1 管脚布局 10  
3.2 管脚定义 10

3.3 Strapping 管脚 13

3.3.1 芯片启动模式控制 14  
3.3.2 VDD_SPI 电压控制 14  
3.3.3 ROM日志打印控制 15  
3.3.4 JTAG信号源控制 15

# 4 电气特性 16

4.1 绝对最大额定值 16  
4.2 建议工作条件 16  
4.3 直流电气特性 (3.3 V, 25 °C) 16  
4.4 功耗特性 17

4.4.1 Active模式下的RF功耗 17  
4.4.2 其他功耗模式下的功耗 17

4.5 Wi-Fi 射频 19

4.5.1 Wi-Fi射频标准 19  
4.5.2 Wi-Fi 射频发射器 (TX) 规格 19  
4.5.3 Wi-Fi 射频接收器 (RX) 规格 20

4.6 低功耗蓝牙射频 21

4.6.1 低功耗蓝牙射频发射器 (TX) 规格 21  
4.6.2 低功耗蓝牙射频接收器 (RX) 规格 23

# 5 模组原理图 26

# 6 外围设计原理图 28

# 7 模组尺寸和PCB封装图形 29

7.1 模组尺寸 29  
7.2 推荐PCB封装图 30  
7.3 外部天线连接器尺寸 32

# 8 产品处理 33

8.1 存储条件 33  
8.2 静电放电 (ESD) 33

8.3 炉温曲线 33  
8.3.1 回流焊温度曲线 33  
8.4 超声波振动 34

# 9 相关文档和资源 35

# 修订历史 36

# 表格

1 ESP32-S3-WROOM-1 系列型号对比 3  
2 ESP32-S3-WROOM-1U系列型号对比 3  
3 管脚定义 11  
4 Strapping 管脚默认配置 13  
5 Strapping 管脚的时序参数说明 13  
6 芯片启动模式控制 14  
7 VDD_SPI 电压控制 15  
8 JTAG信号源控制 15  
9 绝对最大额定值 16  
10 建议工作条件 16  
11 直流电气特性 (3.3 V, 25 °C)  
12 射频功耗 17  
13 Modem-sleep 模式下的功耗 18  
14 低功耗模式下的功耗 18  
15 Wi-Fi 射频标准 19  
16 频谱模板和EVM符合802.11标准时的发射功率 19  
17 发射EVM测试 19  
18 接收灵敏度 20  
19 最大接收电平 21  
20 接收邻道抑制 21  
21 低功耗蓝牙频率 21  
22 发射器特性-低功耗蓝牙1Mbps 21  
23 发射器特性-低功耗蓝牙2Mbps 22  
24 发射器特性-低功耗蓝牙125Kbps 22  
25 发射器特性-低功耗蓝牙500Kbps 23  
26 接收器特性-低功耗蓝牙1Mbps 23  
27 接收器特性-低功耗蓝牙2Mbps 24  
28 接收器特性-低功耗蓝牙125Kbps 24  
29 接收器特性-低功耗蓝牙500Kbps 25

# 插图

1 ESP32-S3-WROOM-1 功能框图 9  
2 ESP32-S3-WROOM-1U 功能框图 9  
3 管脚布局（顶视图） 10  
4 Strapping 管脚的时序参数图 14  
5 ESP32-S3-WROOM-1 原理图 26  
6 ESP32-S3-WROOM-1U 原理图 27  
7 外围设计原理图 28  
8 ESP32-S3-WROOM-1 模组尺寸 29  
9 ESP32-S3-WROOM-1U模组尺寸 29  
10 ESP32-S3-WROOM-1 推荐PCB封装图 30  
11 ESP32-S3-WROOM-1U 推荐PCB封装图 31  
12 外部天线连接器尺寸图 32  
13 回流焊温度曲线 33

# 2 功能框图

![](images/0a8d89a10ebc3ba7642b1a3ee6e8f5843f0e559a9803e50bf7aa120002a88eab.jpg)  
图1: ESP32-S3-WROOM-1功能框图

![](images/57068937b13b7e2f82c7950bdb64a170f5b6539d0e78635d0d85ecfdbbdf9e92.jpg)  
图2: ESP32-S3-WROOM-1U功能框图

# 3 管脚定义

# 3.1 管脚布局

管脚布局图显示了模组上管脚的大致位置。按比例绘制的实际布局请参考图7.1模组尺寸。

注意, ESP32-S3-WROOM-1U的管脚布局与ESP32-S3-WROOM-1相同,但没有禁止布线区(Keepout Zone)。

![](images/b16e62b23fbe748d4de64bbf6deb89000192a6d49fc93f769865b523e2147bee.jpg)  
图3: 管脚布局（顶视图）

# 3.2 管脚定义

模组共有41个管脚，具体描述参见表3管脚定义。

管脚名称释义、管脚功能释义、以及外设管脚分配请参考《ESP32-S3系列芯片技术规格书》。

表 3: 管脚定义  

<table><tr><td>名称</td><td>序号</td><td>类型a</td><td>功能</td></tr><tr><td>GND</td><td>1</td><td>P</td><td>接地</td></tr><tr><td>3V3</td><td>2</td><td>P</td><td>供电</td></tr><tr><td>EN</td><td>3</td><td>I</td><td>高电平:芯片使能;低电平:芯片关闭;注意不能让EN管脚浮空。</td></tr><tr><td>IO4</td><td>4</td><td>I/O/T</td><td>RTC(GPIO4, GPIO4, TOUCH4, ADC1_CH3</td></tr><tr><td>IO5</td><td>5</td><td>I/O/T</td><td>RTC(GPIO5, GPIO5, TOUCH5, ADC1_CH4</td></tr><tr><td>IO6</td><td>6</td><td>I/O/T</td><td>RTC(GPIO6, GPIO6, TOUCH6, ADC1_CH5</td></tr><tr><td>IO7</td><td>7</td><td>I/O/T</td><td>RTC(GPIO7, GPIO7, TOUCH7, ADC1_CH6</td></tr><tr><td>IO15</td><td>8</td><td>I/O/T</td><td>RTC(GPIO15, GPIO15, U0RTS, ADC2_CH4, XTAL_32K_P</td></tr><tr><td>IO16</td><td>9</td><td>I/O/T</td><td>RTC(GPIO16, GPIO16, U0CTS, ADC2_CH5, XTAL_32K_N</td></tr><tr><td>IO17</td><td>10</td><td>I/O/T</td><td>RTC(GPIO17, GPIO17, U1TXD, ADC2_CH6</td></tr><tr><td>IO18</td><td>11</td><td>I/O/T</td><td>RTC(GPIO18, GPIO18, U1RXD, ADC2_CH7, CLK_OUT3</td></tr><tr><td>IO8</td><td>12</td><td>I/O/T</td><td>RTC(GPIO8, GPIO8, TOUCH8, ADC1_CH7, SUBSPICS1</td></tr><tr><td>IO19</td><td>13</td><td>I/O/T</td><td>RTC(GPIO19, GPIO19, U1RTS, ADC2_CH8, CLK_OUT2, USB_D-</td></tr><tr><td>IO20</td><td>14</td><td>I/O/T</td><td>RTC(GPIO20, GPIO20, U1CTS, ADC2_CH9, CLK_OUT1, USB_D+</td></tr><tr><td>IO3</td><td>15</td><td>I/O/T</td><td>RTC(GPIO3, GPIO3, TOUCH3, ADC1_CH2</td></tr><tr><td>IO46</td><td>16</td><td>I/O/T</td><td>GPIO46</td></tr><tr><td>IO9</td><td>17</td><td>I/O/T</td><td>RTC(GPIO9, GPIO9, TOUCH9, ADC1_CH8, FSPIHD, SUBSPIHD</td></tr><tr><td>IO10</td><td>18</td><td>I/O/T</td><td>RTC(GPIO10, GPIO10, TOUCH10, ADC1_CH9, FSPICS0, FSPIIO4, SUBSPICS0</td></tr><tr><td>IO11</td><td>19</td><td>I/O/T</td><td>RTC(GPIO11, GPIO11, TOUCH11, ADC2_CH0, FSPID, FSPIIO5, SUBSPID</td></tr><tr><td>IO12</td><td>20</td><td>I/O/T</td><td>RTC(GPIO12, GPIO12, TOUCH12, ADC2_CH1, FSPICLK, FSPIIO6, SUBSPICLK</td></tr><tr><td>IO13</td><td>21</td><td>I/O/T</td><td>RTC(GPIO13, GPIO13, TOUCH13, ADC2_CH2, FSPIQ, FSPIIO7, SUBSPIQ</td></tr><tr><td>IO14</td><td>22</td><td>I/O/T</td><td>RTC(GPIO14, GPIO14, TOUCH14, ADC2_CH3, FSPIW, FSPIDQS, SUBSPIW</td></tr><tr><td>IO21</td><td>23</td><td>I/O/T</td><td>RTC(GPIO21, GPIO21</td></tr><tr><td>IO47c</td><td>24</td><td>I/O/T</td><td>SPICLK_P_DIFF, GPIO47, SUBSPICLK_P_DIFF</td></tr><tr><td>IO48c</td><td>25</td><td>I/O/T</td><td>SPICLK_N_DIFF, GPIO48, SUBSPICLK_N_DIFF</td></tr><tr><td>IO45</td><td>26</td><td>I/O/T</td><td>GPIO45</td></tr><tr><td>IO0</td><td>27</td><td>I/O/T</td><td>RTC(GPIO0, GPIO0</td></tr><tr><td>IO35b</td><td>28</td><td>I/O/T</td><td>SPIO6, GPIO35, FSPID, SUBSPID</td></tr><tr><td>IO36b</td><td>29</td><td>I/O/T</td><td>SPIO7, GPIO36, FSPICLK, SUBSPICLK</td></tr><tr><td>IO37b</td><td>30</td><td>I/O/T</td><td>SPIDQS, GPIO37, FSPIQ, SUBSPIQ</td></tr><tr><td>IO38</td><td>31</td><td>I/O/T</td><td>GPIO38, FSPIW, SUBSPIW</td></tr><tr><td>IO39</td><td>32</td><td>I/O/T</td><td>MTCK, GPIO39, CLK_OUT3, SUBSPICS1</td></tr><tr><td>IO40</td><td>33</td><td>I/O/T</td><td>MTDO, GPIO40, CLK_OUT2</td></tr><tr><td>IO41</td><td>34</td><td>I/O/T</td><td>MTDI, GPIO41, CLK_OUT1</td></tr></table>

见下页

表3-接上页  

<table><tr><td>名称</td><td>序号</td><td>类型a</td><td>功能</td></tr><tr><td>IO42</td><td>35</td><td>I/O/T</td><td>MTMS, GPIO42</td></tr><tr><td>RXD0</td><td>36</td><td>I/O/T</td><td>U0RXD, GPIO44, CLK_OUT2</td></tr><tr><td>TXD0</td><td>37</td><td>I/O/T</td><td>U0TXD, GPIO43, CLK_OUT1</td></tr><tr><td>IO2</td><td>38</td><td>I/O/T</td><td>RTC(GPIO2, GPIO2, TOUCH2, ADC1_CH1</td></tr><tr><td>IO1</td><td>39</td><td>I/O/T</td><td>RTC(GPIO1, GPIO1, TOUCH1, ADC1_CH0</td></tr><tr><td>GND</td><td>40</td><td>P</td><td>接地</td></tr><tr><td>EPAD</td><td>41</td><td>P</td><td>接地</td></tr></table>

a P: 电源; I: 输入; O: 输出; T: 可设置为高阻。加粗字体为管脚的默认功能。管脚  $28 \sim 30$  的默认功能由 eFuse 位决定。  
在集成 Octal SPI PSRAM（即内置芯片为 ESP32-S3R8 或 ESP32-S3R16V）的模组中，管脚 IO35、IO36、IO37 已连接至模组内部集成的 Octal SPI PSRAM，不可用于其他功能。  
在内置芯片为 ESP32-S3R16V 的模组中，由于 ESP32-S3R16V 芯片的 VDD_SPI 电压已设置为  $1.8 \mathrm{~V}$ ，所以，不同于其他 GPIO，VDD_SPI 电源域中的 GPIO47 和 GPIO48 工作电压也为  $1.8 \mathrm{~V}$ 。

# 3.3 Strapping 管脚

说明：

以下内容摘自《ESP32-S3系列芯片技术规格书》>章节Strapping管脚。芯片的Strapping管脚与模组管脚的对应关系，可参考章节5模组原理图。

模组每次上电或复位时，都需要一些初始配置参数，如加载模组的启动模式、flash存储器的电压等。这些参数通过strapping管脚控制。复位放开后，strapping管脚和普通IO管脚功能相同。

模组复位时，strapping管脚在复位时控制以下参数：

- 芯片启动模式-GPIO0和GPIO46  
VDD_SPI电压-GPIO45  
- ROM 代码日志打印 - GPIO46  
- JTAG 信号源 - GPIO3

GPIO0、GPIO45 和 GPIO46 在芯片复位时连接芯片内部的弱上拉/下拉电阻。如果 strapping 管脚没有外部连接或者连接的外部线路处于高阻抗状态，这些电阻将决定 strapping 管脚的默认值。

表 4: Strapping 管脚默认配置  

<table><tr><td>Strapping管脚</td><td>默认配置</td><td>值</td></tr><tr><td>GPIO0</td><td>上拉</td><td>1</td></tr><tr><td>GPIO3</td><td>浮空</td><td>-</td></tr><tr><td>GPIO45</td><td>下拉</td><td>0</td></tr><tr><td>GPIO46</td><td>下拉</td><td>0</td></tr></table>

要改变strapping管脚的值，可以连接外部下拉/上拉电阻。如果ESP32-S3用作主机MCU的从设备，strapping管脚的电平也可通过主机MCU控制。

所有 strapping 管脚都有锁存器。系统复位时，锁存器采样并存储相应 strapping 管脚的值，一直保持到芯片掉电或关闭。锁存器的状态无法用其他方式更改。因此，strapping 管脚的值在芯片工作时一直可读取，并可在芯片复位后作为普通 IO 管脚使用。

Strapping 管脚的时序参数包括建立时间和保持时间。更多信息，详见表 5 和图 4。

表 5: Strapping 管脚的时序参数说明  

<table><tr><td>参数</td><td>说明</td><td>最小值 (ms)</td></tr><tr><td>tsu</td><td>建立时间，即拉高 CHIPPU 激活芯片前，电源轨达到稳定所需的时间</td><td>0</td></tr><tr><td>tH</td><td>保持时间，即 CHIPPU 已拉高、strapping 管脚变为普通 IO 管脚开始工作前，可读取 strapping 管脚值的时间</td><td>3</td></tr></table>

![](images/c5a284c54a256116d9943ae9e658b2d66fbe409b1e386044fced659e04675e72.jpg)  
图4:Strapping管脚的时序参数图

# 3.3.1 芯片启动模式控制

复位释放后，GPIO0和GPIO46共同决定启动模式。详见表6芯片启动模式控制。

表 6: 芯片启动模式控制  

<table><tr><td>启动模式</td><td>GPIO0</td><td>GPIO46</td></tr><tr><td>默认配值</td><td>1(上拉)</td><td>0(下拉)</td></tr><tr><td>SPI Boot（默认）</td><td>1</td><td>任意值</td></tr><tr><td>Joint Download Boot¹</td><td>0</td><td>0</td></tr></table>

1 Joint Download Boot 模式下支持以下下载方式：  
- USB Download Boot:  
- USB-Serial-JTAG Download Boot  
- USB-OTG Download Boot  
- UART Download Boot

在SPI Boot模式下，ROM引导加载程序通过从SPI flash中读取程序来启动系统。

在 Joint Download Boot 模式下，用户可通过 USB 或 UART0 接口将二进制文件下载至 flash，或将二进制文件下载至 SRAM 并运行 SRAM 中的程序。

除了SPI Boot和Joint Download Boot模式,ESP32-S3还支持SPI Download Boot模式,详见《ESP32-S3技术参考手册》>章节芯片Boot控制。

# 3.3.2 VDD_SPI 电压控制

电压有两种控制方式，具体取决于EFUSE_VDD_SPI FORCE的值。

表 7: VDD_SPI 电压控制  

<table><tr><td>EFUSE_VDD_SPI FORCE</td><td>GPIO45</td><td>eFuse1</td><td>电压</td><td>VDD_SPI 电源2</td></tr><tr><td rowspan="2">0</td><td>0</td><td rowspan="2">忽略</td><td>3.3 V</td><td>VDD3P3_RTC 通过 RSPI 供电</td></tr><tr><td>1</td><td>1.8 V</td><td>Flash 稳压器</td></tr><tr><td rowspan="2">1</td><td rowspan="2">忽略</td><td>0</td><td>1.8 V</td><td>Flash 稳压器</td></tr><tr><td>1</td><td>3.3 V</td><td>VDD3P3_RTC 通过 RSPI 供电</td></tr></table>

1 eFuse: EFUSE_VDD_SPI_TIEH  
2 请参考《ESP32-S3系列芯片技术规格书》>章节电源管理

# 3.3.3 ROM日志打印控制

系统启动过程中，ROM代码日志可打印至：

- (默认) UART 和 USB 串口/JTAG 控制器。  
- USB串口/JTAG控制器。  
- UART。

通过配置寄存器和 eFuse 可分别关闭 UART 和 USB 串口/JTAG 控制器的 ROM 代码日志打印功能。详细信息请参考《ESP32-S3 技术参考手册》>章节芯片 Boot 控制。

# 3.3.4 JTAG信号源控制

在系统启动早期阶段，GPIO3可用于控制JTAG信号源。该管脚没有内部上下拉电阻，strapping的值必须由不处于高阻抗状态的外部电路控制。

如表8所示，GPIO3与EFUSE_DISPAD_JTAG、EFUSE_DIS_USB_JTAG和EFUSESTRAP_JTAG_SEL共同控制JTAG信号源。

表 8: JTAG 信号源控制  

<table><tr><td>eFuse 1a</td><td>eFuse 2b</td><td>eFuse 3c</td><td>GPIO3</td><td>JTAG信号源</td></tr><tr><td rowspan="3">0</td><td rowspan="3">0</td><td>0</td><td>忽略</td><td>USB串口/JTAG控制器</td></tr><tr><td rowspan="2">1</td><td>0</td><td>JTAG管脚MTDI、MTCK、MTMS和MTDO</td></tr><tr><td>1</td><td>USB串口/JTAG控制器</td></tr><tr><td>0</td><td>1</td><td>忽略</td><td>忽略</td><td>JTAG管脚MTDI、MTCK、MTMS和MTDO</td></tr><tr><td>1</td><td>0</td><td>忽略</td><td>忽略</td><td>USB串口/JTAG控制器</td></tr><tr><td>1</td><td>1</td><td>忽略</td><td>忽略</td><td>JTAG关闭</td></tr></table>

a eFuse 1: EFUSE_DISPAD_JTAG  
$^b$  eFuse 2: EFUSE_DIS_USB_JTAG  
eFuse 3: EFUSE Strap_JTAG_SEL

# 4 电气特性

# 4.1 绝对最大额定值

超出表9绝对最大额定值可能导致器件永久性损坏。这只是强调的额定值，不涉及器件在这些或其它条件下超出表10建议工作条件技术规格指标的功能性操作。长时间暴露在绝对最大额定条件下可能会影响模组的可靠性。

表 9: 绝对最大额定值  

<table><tr><td>符号</td><td>参数</td><td>最小值</td><td>最大值</td><td>单位</td></tr><tr><td>VDD33</td><td>电源管脚电压</td><td>-0.3</td><td>3.6</td><td>V</td></tr><tr><td>TSTORE</td><td>存储温度</td><td>-40</td><td>105</td><td>°C</td></tr></table>

# 4.2 建议工作条件

表 10: 建议工作条件  

<table><tr><td>符号</td><td colspan="2">参数</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>VDD33</td><td colspan="2">电源管脚电压</td><td>3.0</td><td>3.3</td><td>3.6</td><td>V</td></tr><tr><td>I V D D</td><td colspan="2">外部电源的供电电流</td><td>0.5</td><td>—</td><td>—</td><td>A</td></tr><tr><td rowspan="3">TA</td><td rowspan="3">环境温度</td><td>65℃版</td><td rowspan="3">-40</td><td rowspan="3">—</td><td>65</td><td rowspan="3">℃</td></tr><tr><td>85℃版</td><td>85</td></tr><tr><td>105℃版</td><td>105</td></tr></table>

# 4.3 直流电气特性 (3.3 V, 25 °C)

表 11: 直流电气特性 (3.3 V, 25 °C)  

<table><tr><td>符号</td><td>参数</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>CIN</td><td>管脚电容</td><td>—</td><td>2</td><td>—</td><td>pF</td></tr><tr><td>VIH</td><td>高电平输入电压</td><td>0.75 × VDD1</td><td>—</td><td>VDD1+ 0.3</td><td>V</td></tr><tr><td>VIL</td><td>低电平输入电压</td><td>-0.3</td><td>—</td><td>0.25 × VDD1</td><td>V</td></tr><tr><td>IH</td><td>高电平输入电流</td><td>—</td><td>—</td><td>50</td><td>nA</td></tr><tr><td>IL</td><td>低电平输入电流</td><td>—</td><td>—</td><td>50</td><td>nA</td></tr><tr><td>VOH2</td><td>高电平输出电压</td><td>0.8 × VDD1</td><td>—</td><td>—</td><td>V</td></tr><tr><td>VOL2</td><td>低电平输出电压</td><td>—</td><td>—</td><td>0.1 × VDD1</td><td>V</td></tr><tr><td>IOH</td><td>高电平拉电流 (VDD1= 3.3 V, VOH &gt;= 2.64 V, PAD_DRVER = 3)</td><td>—</td><td>40</td><td>—</td><td>mA</td></tr><tr><td>IOL</td><td>低电平灌电流 (VDD1= 3.3 V, VOL = 0.495 V, PAD_DRVER = 3)</td><td>—</td><td>28</td><td>—</td><td>mA</td></tr><tr><td>RPU</td><td>内部弱上拉电阻</td><td>—</td><td>45</td><td>—</td><td>kΩ</td></tr><tr><td>RPD</td><td>内部弱下拉电阻</td><td>—</td><td>45</td><td>—</td><td>kΩ</td></tr><tr><td>VIH_nRST</td><td>芯片复位释放电压(EN 管脚应满足电压范围)</td><td>0.75 × VDD1</td><td>—</td><td>VDD1+ 0.3</td><td>V</td></tr></table>

表11-接上页  

<table><tr><td>符号</td><td>参数</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>VIL_nRST</td><td>芯片复位电压(EN管脚应满足电压范围)</td><td>-0.3</td><td>—</td><td>0.25 × VDD¹</td><td>V</td></tr></table>

1 VDD是I/O的供电电源。  
$^{2}\mathsf{V}_{OH}$  和  $\mathsf{V}_{OL}$  为负载是高阻条件下的测量值。

# 4.4 功耗特性

# 4.4.1 Active 模式下的 RF 功耗

因使用了先进的电源管理技术，模组可以在不同的功耗模式之间切换。关于不同功耗模式的描述，详见《ESP32-S3系列芯片技术规格书》的低功耗管理章节。

表 12: 射频功耗  

<table><tr><td>工作模式</td><td colspan="2">描述</td><td>峰值 (mA)</td></tr><tr><td rowspan="6">Active(射频工作)</td><td rowspan="4">TX</td><td>802.11b, 1 Mbps, @20.5 dBm</td><td>355</td></tr><tr><td>802.11g, 54 Mbps, @18 dBm</td><td>297</td></tr><tr><td>802.11n, HT20, MCS7, @17.5 dBm</td><td>286</td></tr><tr><td>802.11n, HT40, MCS7, @17 dBm</td><td>285</td></tr><tr><td rowspan="2">RX</td><td>802.11b/g/n, HT20</td><td>95</td></tr><tr><td>802.11n, HT40</td><td>97</td></tr></table>

1 以上功耗数据是基于  $3.3 \mathrm{~V}$  电源、 $25^{\circ} \mathrm{C}$  环境温度，在 RF 接口处完成的测试结果。所有发射数据均基于  $100 \%$  的占空比测得。  
2 测量 RX 功耗数据时，外设处于关闭状态，CPU 处于空闲状态。

# 说明：

以下内容摘自《ESP32-S3系列芯片技术规格书》的其他功耗模式下的功耗章节。

# 4.4.2 其他功耗模式下的功耗

请注意，若模组内置芯片封装内有PSRAM，功耗数据可能略高于下表数据。

表 13: Modem-sleep 模式下的功耗  

<table><tr><td>工作模式</td><td>频率(MHz)</td><td>说明</td><td>典型值1(mA)</td><td>典型值2(mA)</td></tr><tr><td rowspan="20">Modem-sleep3</td><td rowspan="5">40</td><td>WAITI(双核均空闲)</td><td>13.2</td><td>18.8</td></tr><tr><td>单核执行32位数据访问指令,另一个核空闲</td><td>16.2</td><td>21.8</td></tr><tr><td>双核执行32位数据访问指令</td><td>18.7</td><td>24.4</td></tr><tr><td>单核执行128位数据访问指令,另一个核空闲</td><td>19.9</td><td>25.4</td></tr><tr><td>双核执行128位数据访问指令</td><td>23.0</td><td>28.8</td></tr><tr><td rowspan="5">80</td><td>WAITI</td><td>22.0</td><td>36.1</td></tr><tr><td>单核执行32位数据访问指令,另一个核空闲</td><td>28.4</td><td>42.6</td></tr><tr><td>双核执行32位数据访问指令</td><td>33.1</td><td>47.3</td></tr><tr><td>单核执行128位数据访问指令,另一个核空闲</td><td>35.1</td><td>49.6</td></tr><tr><td>双核执行128位数据访问指令</td><td>41.8</td><td>56.3</td></tr><tr><td rowspan="5">160</td><td>WAITI</td><td>27.6</td><td>42.3</td></tr><tr><td>单核执行32位数据访问指令,另一个核空闲</td><td>39.9</td><td>54.6</td></tr><tr><td>双核执行32位数据访问指令</td><td>49.6</td><td>64.1</td></tr><tr><td>单核执行128位数据访问指令,另一个核空闲</td><td>54.4</td><td>69.2</td></tr><tr><td>双核执行128位数据访问指令</td><td>66.7</td><td>81.1</td></tr><tr><td rowspan="5">240</td><td>WAITI</td><td>32.9</td><td>47.6</td></tr><tr><td>单核执行32位数据访问指令,另一个核空闲</td><td>51.2</td><td>65.9</td></tr><tr><td>双核执行32位数据访问指令</td><td>66.2</td><td>81.3</td></tr><tr><td>单核执行128位数据访问指令,另一个核空闲</td><td>72.4</td><td>87.9</td></tr><tr><td>双核执行128位数据访问指令</td><td>91.7</td><td>107.9</td></tr></table>

1 所有外设时钟关闭时的典型值。  
2 所有外设时钟打开时的典型值。实际情况下，外设在不同工作状态下电流会有所差异。  
3 Modem-sleep 模式下，Wi-Fi 设有时钟门控。该模式下，访问 flash 时功耗会增加。若 flash 速率为 80 Mbit/s, SPI 双线模式下 flash 的功耗为  $10 \mathrm{~mA}$  。

表 14: 低功耗模式下的功耗  

<table><tr><td>工作模式</td><td>说明</td><td>典型值(μA)</td></tr><tr><td>Light-sleep1</td><td>VDD_SPI和Wi-Fi掉电，所有GPIO设置为高阻状态</td><td>240</td></tr><tr><td rowspan="2">Deep-sleep</td><td>RTC存储器和RTC外设上电</td><td>8</td></tr><tr><td>RTC存储器上电，RTC外设掉电</td><td>7</td></tr><tr><td>关闭</td><td>CHIP_PU管脚拉低，芯片关闭</td><td>1</td></tr></table>

Light-sleep 模式下，SPI 相关管脚上拉。封装内有 PSRAM 的芯片请在典型值的基础上添加相应的 PSRAM 功耗：8 MB Octal PSRAM (3.3 V) 为  $140 \mu \mathrm{A}$ ；8 MB Octal PSRAM (1.8 V) 为  $200 \mu \mathrm{A}$ ；2 MB Quad PSRAM 为  $40 \mu \mathrm{A}$ 。

# 4.5 Wi-Fi 射频

# 4.5.1 Wi-Fi 射频标准

表 15: Wi-Fi 射频标准  

<table><tr><td colspan="2">名称</td><td>描述</td></tr><tr><td colspan="2">工作信道中心频率范围1</td><td>2412 ~ 2484 MHz</td></tr><tr><td colspan="2">Wi-Fi 协议</td><td>IEEE 802.11b/g/n</td></tr><tr><td rowspan="2">数据速率</td><td>20 MHz</td><td>11b: 1, 2, 5.5, 11 Mbps11g: 6, 9, 12, 18, 24, 36, 48, 54 Mbps11n: MCS0-7, 72.2 Mbps (Max)</td></tr><tr><td>40 MHz</td><td>11n: MCS0-7, 150 Mbps (Max)</td></tr><tr><td colspan="2">天线类型</td><td>PCB 天线, 外部天线连接器2</td></tr></table>

1 工作信道中心频率范围应符合国家或地区的规范标准。软件可以配置工作信道中心频率范围  
2 用外部天线连接器的模组输出阻抗为 50 ，不使用外部天线连接器的模组可无需关注输出阻抗。

# 4.5.2 Wi-Fi 射频发射器 (TX) 规格

根据产品或认证的要求，您可以配置发射器目标功率。默认功率详见表16频谱模板和EVM符合802.11标准时的发射功率。

表 16: 频谱模板和 EVM 符合 802.11 标准时的发射功率  

<table><tr><td>速率</td><td>最小值(dBm)</td><td>典型值(dBm)</td><td>最大值(dBm)</td></tr><tr><td>802.11b, 1 Mbps</td><td>—</td><td>20.5</td><td>—</td></tr><tr><td>802.11b, 11 Mbps</td><td>—</td><td>20.5</td><td>—</td></tr><tr><td>802.11g, 6 Mbps</td><td>—</td><td>20.0</td><td>—</td></tr><tr><td>802.11g, 54 Mbps</td><td>—</td><td>18.0</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 0</td><td>—</td><td>19.0</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 7</td><td>—</td><td>17.5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 0</td><td>—</td><td>18.5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 7</td><td>—</td><td>17.0</td><td>—</td></tr></table>

见下页  
表 17: 发射 EVM 测试  

<table><tr><td>速率</td><td>最小值(dB)</td><td>典型值(dB)</td><td>标准限值(dB)</td></tr><tr><td>802.11b, 1 Mbps, @20.5 dBm</td><td>—</td><td>-24.5</td><td>-10</td></tr><tr><td>802.11b, 11 Mbps, @20.5 dBm</td><td>—</td><td>-24.5</td><td>-10</td></tr><tr><td>802.11g, 6 Mbps, @20 dBm</td><td>—</td><td>-23.0</td><td>-5</td></tr><tr><td>802.11g, 54 Mbps, @18 dBm</td><td>—</td><td>-29.5</td><td>-25</td></tr><tr><td>802.11n, HT20, MCS 0, @19 dBm</td><td>—</td><td>-24.0</td><td>-5</td></tr><tr><td>802.11n, HT20, MCS 7, @17.5 dBm</td><td>—</td><td>-30.5</td><td>-27</td></tr><tr><td>802.11n, HT40, MCS 0, @18.5 dBm</td><td>—</td><td>-25.0</td><td>-5</td></tr></table>

表17-接上页  

<table><tr><td>速率</td><td>最小值(dB)</td><td>典型值(dB)</td><td>标准限值(dB)</td></tr><tr><td>802.11n, HT40, MCS 7, @17 dBm</td><td>—</td><td>-30.0</td><td>-27</td></tr></table>

# 4.5.3 Wi-Fi 射频接收器 (RX) 规格

表 18: 接收灵敏度  

<table><tr><td>速率</td><td>最小值(dBm)</td><td>典型值(dBm)</td><td>最大值(dBm)</td></tr><tr><td>802.11b, 1 Mbps</td><td>—</td><td>-98.2</td><td>—</td></tr><tr><td>802.11b, 2 Mbps</td><td>—</td><td>-95.6</td><td>—</td></tr><tr><td>802.11b, 5.5 Mbps</td><td>—</td><td>-92.8</td><td>—</td></tr><tr><td>802.11b, 11 Mbps</td><td>—</td><td>-88.5</td><td>—</td></tr><tr><td>802.11g, 6 Mbps</td><td>—</td><td>-93.0</td><td>—</td></tr><tr><td>802.11g, 9 Mbps</td><td>—</td><td>-92.0</td><td>—</td></tr><tr><td>802.11g, 12 Mbps</td><td>—</td><td>-90.8</td><td>—</td></tr><tr><td>802.11g, 18 Mbps</td><td>—</td><td>-88.5</td><td>—</td></tr><tr><td>802.11g, 24 Mbps</td><td>—</td><td>-85.5</td><td>—</td></tr><tr><td>802.11g, 36 Mbps</td><td>—</td><td>-82.2</td><td>—</td></tr><tr><td>802.11g, 48 Mbps</td><td>—</td><td>-78.0</td><td>—</td></tr><tr><td>802.11g, 54 Mbps</td><td>—</td><td>-76.2</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 0</td><td>—</td><td>-93.0</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 1</td><td>—</td><td>-90.6</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 2</td><td>—</td><td>-88.4</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 3</td><td>—</td><td>-84.8</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 4</td><td>—</td><td>-81.6</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 5</td><td>—</td><td>-77.4</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 6</td><td>—</td><td>-75.6</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 7</td><td>—</td><td>-74.2</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 0</td><td>—</td><td>-90.0</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 1</td><td>—</td><td>-87.5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 2</td><td>—</td><td>-85.0</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 3</td><td>—</td><td>-82.0</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 4</td><td>—</td><td>-78.5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 5</td><td>—</td><td>-74.4</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 6</td><td>—</td><td>-72.5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 7</td><td>—</td><td>-71.2</td><td>—</td></tr></table>

表 19: 最大接收电平  

<table><tr><td>速率</td><td>最小值(dBm)</td><td>典型值(dBm)</td><td>最大值(dBm)</td></tr><tr><td>802.11b, 1 Mbps</td><td>—</td><td>5</td><td>—</td></tr><tr><td>802.11b, 11 Mbps</td><td>—</td><td>5</td><td>—</td></tr><tr><td>802.11g, 6 Mbps</td><td>—</td><td>5</td><td>—</td></tr><tr><td>802.11g, 54 Mbps</td><td>—</td><td>0</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 0</td><td>—</td><td>5</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 7</td><td>—</td><td>0</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 0</td><td>—</td><td>5</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 7</td><td>—</td><td>0</td><td>—</td></tr></table>

表 20: 接收邻道抑制  

<table><tr><td>速率</td><td>最小值(dB)</td><td>典型值(dB)</td><td>最大值(dB)</td></tr><tr><td>802.11b, 1 Mbps</td><td>—</td><td>35</td><td>—</td></tr><tr><td>802.11b, 11 Mbps</td><td>—</td><td>35</td><td>—</td></tr><tr><td>802.11g, 6 Mbps</td><td>—</td><td>31</td><td>—</td></tr><tr><td>802.11g, 54 Mbps</td><td>—</td><td>14</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 0</td><td>—</td><td>31</td><td>—</td></tr><tr><td>802.11n, HT20, MCS 7</td><td>—</td><td>13</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 0</td><td>—</td><td>19</td><td>—</td></tr><tr><td>802.11n, HT40, MCS 7</td><td>—</td><td>8</td><td>—</td></tr></table>

# 4.6 低功耗蓝牙射频

表 21: 低功耗蓝牙频率  

<table><tr><td>参数</td><td>最小值
(MHz)</td><td>典型值
(MHz)</td><td>最大值
(MHz)</td></tr><tr><td>工作信道中心频率</td><td>2402</td><td>—</td><td>2480</td></tr></table>

# 4.6.1 低功耗蓝牙射频发射器 (TX) 规格

见下页  
表 22: 发射器特性 - 低功耗蓝牙 1 Mbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td rowspan="2">射频发射功率</td><td>射频功率控制范围</td><td>-24.00</td><td>0</td><td>20.00</td><td>dBm</td></tr><tr><td>增益控制步长</td><td>—</td><td>3.00</td><td>—</td><td>dB</td></tr><tr><td rowspan="3">载波频率偏移和漂移</td><td>|fn|n=0,1,2,...k 最大值</td><td>—</td><td>2.50</td><td>—</td><td>kHz</td></tr><tr><td>|f0-fn| 最大值</td><td>—</td><td>2.00</td><td>—</td><td>kHz</td></tr><tr><td>|fn-fn-5| 最大值</td><td>—</td><td>1.40</td><td>—</td><td>kHz</td></tr></table>

表22-接上页  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td></td><td>|f1-f0|</td><td>—</td><td>1.00</td><td>—</td><td>kHz</td></tr><tr><td rowspan="3">调制特性</td><td>Δ f1avg</td><td>—</td><td>249.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f2max 最小值(至少 99.9% 的 Δ f2max)</td><td>—</td><td>198.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f2avg/Δ f1avg</td><td>—</td><td>0.86</td><td>—</td><td>—</td></tr><tr><td rowspan="3">带内杂散发射</td><td>±2 MHz 偏移</td><td>—</td><td>-37.00</td><td>—</td><td>dBm</td></tr><tr><td>±3 MHz 偏移</td><td>—</td><td>-42.00</td><td>—</td><td>dBm</td></tr><tr><td>&gt;±3 MHz 偏移</td><td>—</td><td>-44.00</td><td>—</td><td>dBm</td></tr></table>

表 23: 发射器特性 - 低功耗蓝牙 2 Mbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td rowspan="2">射频发射功率</td><td>射频功率控制范围</td><td>-24.00</td><td>0</td><td>20.00</td><td>dBm</td></tr><tr><td>增益控制步长</td><td>—</td><td>3.00</td><td>—</td><td>dB</td></tr><tr><td rowspan="4">载波频率偏移和漂移</td><td>|fn|n=0,1,2,.k最大值</td><td>—</td><td>2.50</td><td>—</td><td>kHz</td></tr><tr><td>|f0-fn|最大值</td><td>—</td><td>2.00</td><td>—</td><td>kHz</td></tr><tr><td>|fn-fn-5|最大值</td><td>—</td><td>1.40</td><td>—</td><td>kHz</td></tr><tr><td>|f1-f0|</td><td>—</td><td>1.00</td><td>—</td><td>kHz</td></tr><tr><td rowspan="3">调制特性</td><td>Δ f1avg</td><td>—</td><td>499.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f2max最小值(至少99.9%的Δf2max)</td><td>—</td><td>416.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f2avg/Δ f1avg</td><td>—</td><td>0.89</td><td>—</td><td>—</td></tr><tr><td rowspan="3">带内杂散发射</td><td>±4 MHz偏移</td><td>—</td><td>-42.00</td><td>—</td><td>dBm</td></tr><tr><td>±5 MHz偏移</td><td>—</td><td>-44.00</td><td>—</td><td>dBm</td></tr><tr><td>&gt;±5 MHz偏移</td><td>—</td><td>-47.00</td><td>—</td><td>dBm</td></tr></table>

表 24: 发射器特性 - 低功耗蓝牙 125 Kbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td rowspan="2">射频发射功率</td><td>射频功率控制范围</td><td>-24.00</td><td>0</td><td>20.00</td><td>dBm</td></tr><tr><td>增益控制步长</td><td>—</td><td>3.00</td><td>—</td><td>dB</td></tr><tr><td rowspan="4">载波频率偏移和漂移</td><td>|fn|n=0,1,2,...k最大值</td><td>—</td><td>0.80</td><td>—</td><td>kHz</td></tr><tr><td>|f0-fn|最大值</td><td>—</td><td>1.00</td><td>—</td><td>kHz</td></tr><tr><td>|fn-fn-3|</td><td>—</td><td>0.30</td><td>—</td><td>kHz</td></tr><tr><td>|f0-f3|</td><td>—</td><td>1.00</td><td>—</td><td>kHz</td></tr><tr><td rowspan="2">调制特性</td><td>Δ f1avg</td><td>—</td><td>248.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f1max最小值(至少99.9%的Δ f1max)</td><td>—</td><td>222.00</td><td>—</td><td>kHz</td></tr><tr><td rowspan="3">带内杂散发射</td><td>±2 MHz偏移</td><td>—</td><td>-37.00</td><td>—</td><td>dBm</td></tr><tr><td>±3 MHz偏移</td><td>—</td><td>-42.00</td><td>—</td><td>dBm</td></tr><tr><td>&gt;±3 MHz偏移</td><td>—</td><td>-44.00</td><td>—</td><td>dBm</td></tr></table>

表 25: 发射器特性 - 低功耗蓝牙 500 Kbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td rowspan="2">射频发射功率</td><td>射频功率控制范围</td><td>-24.00</td><td>0</td><td>20.00</td><td>dBm</td></tr><tr><td>增益控制步长</td><td>—</td><td>3.00</td><td>—</td><td>dB</td></tr><tr><td rowspan="4">载波频率偏移和漂移</td><td>|fn|n=0,1,2,...k最大值</td><td>—</td><td>0.80</td><td>—</td><td>kHz</td></tr><tr><td>|f0-fn|最大值</td><td>—</td><td>1.00</td><td>—</td><td>kHz</td></tr><tr><td>|fn-fn-3|</td><td>—</td><td>0.85</td><td>—</td><td>kHz</td></tr><tr><td>|f0-f3|</td><td>—</td><td>0.34</td><td>—</td><td>kHz</td></tr><tr><td rowspan="2">调制特性</td><td>Δ f2avg</td><td>—</td><td>213.00</td><td>—</td><td>kHz</td></tr><tr><td>Δ f2max最小值(至少99.9%的Δf2max)</td><td>—</td><td>196.00</td><td>—</td><td>kHz</td></tr><tr><td rowspan="3">带内杂散发射</td><td>±2 MHz偏移</td><td>—</td><td>-37.00</td><td>—</td><td>dBm</td></tr><tr><td>±3 MHz偏移</td><td>—</td><td>-42.00</td><td>—</td><td>dBm</td></tr><tr><td>&gt;±3 MHz偏移</td><td>—</td><td>-44.00</td><td>—</td><td>dBm</td></tr></table>

# 4.6.2 低功耗蓝牙射频接收器 (RX) 规格

表 26: 接收器特性 - 低功耗蓝牙 1 Mbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>灵敏度 @30.8% PER</td><td>—</td><td>—</td><td>-96.5</td><td>—</td><td>dBm</td></tr><tr><td>最大接收信号 @30.8% PER</td><td>—</td><td>—</td><td>8</td><td>—</td><td>dBm</td></tr><tr><td>共信道抑制比 C/I</td><td>F = F0 MHz</td><td>—</td><td>8</td><td>—</td><td>dB</td></tr><tr><td rowspan="8">邻道选择性抑制比 C/I</td><td>F = F0 + 1 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 1 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 2 MHz</td><td>—</td><td>-23</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 2 MHz</td><td>—</td><td>-23</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 3 MHz</td><td>—</td><td>-34</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 3 MHz</td><td>—</td><td>-34</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 + 3 MHz</td><td>—</td><td>-36</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 - 3 MHz</td><td>—</td><td>-37</td><td>—</td><td>dB</td></tr><tr><td>镜像频率</td><td>—</td><td>—</td><td>-36</td><td>—</td><td>dB</td></tr><tr><td rowspan="2">邻道镜像频率干扰</td><td>F = Fimage + 1 MHz</td><td>—</td><td>-39</td><td>—</td><td>dB</td></tr><tr><td>F = Fimage - 1 MHz</td><td>—</td><td>-34</td><td>—</td><td>dB</td></tr><tr><td rowspan="4">带外阻塞</td><td>30 MHz ~ 2000 MHz</td><td>—</td><td>-12</td><td>—</td><td>dBm</td></tr><tr><td>2003 MHz ~ 2399 MHz</td><td>—</td><td>-18</td><td>—</td><td>dBm</td></tr><tr><td>2484 MHz ~ 2997 MHz</td><td>—</td><td>-16</td><td>—</td><td>dBm</td></tr><tr><td>3000 MHz ~ 12.75 GHz</td><td>—</td><td>-10</td><td>—</td><td>dBm</td></tr><tr><td>互调</td><td>—</td><td>—</td><td>-29</td><td>—</td><td>dBm</td></tr></table>

表 27: 接收器特性 - 低功耗蓝牙 2 Mbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>灵敏度 @30.8% PER</td><td>—</td><td>—</td><td>-92</td><td>—</td><td>dBm</td></tr><tr><td>最大接收信号 @30.8% PER</td><td>—</td><td>—</td><td>3</td><td>—</td><td>dBm</td></tr><tr><td>共信道干扰 C/I</td><td>F = F0 MHz</td><td>—</td><td>8</td><td>—</td><td>dB</td></tr><tr><td rowspan="8">邻道选择性抑制比 C/I</td><td>F = F0 + 2 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 2 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 4 MHz</td><td>—</td><td>-27</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 4 MHz</td><td>—</td><td>-27</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 6 MHz</td><td>—</td><td>-38</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 6 MHz</td><td>—</td><td>-38</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 + 6 MHz</td><td>—</td><td>-41</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 - 6 MHz</td><td>—</td><td>-41</td><td>—</td><td>dB</td></tr><tr><td>镜像频率</td><td>—</td><td>—</td><td>-27</td><td>—</td><td>dB</td></tr><tr><td rowspan="2">邻道镜像频率干扰</td><td>F = Fimage + 2 MHz</td><td>—</td><td>-38</td><td>—</td><td>dB</td></tr><tr><td>F = Fimage - 2 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td rowspan="4">带外阻塞</td><td>30 MHz ~ 2000 MHz</td><td>—</td><td>-15</td><td>—</td><td>dBm</td></tr><tr><td>2003 MHz ~ 2399 MHz</td><td>—</td><td>-21</td><td>—</td><td>dBm</td></tr><tr><td>2484 MHz ~ 2997 MHz</td><td>—</td><td>-21</td><td>—</td><td>dBm</td></tr><tr><td>3000 MHz ~ 12.75 GHz</td><td>—</td><td>-9</td><td>—</td><td>dBm</td></tr><tr><td>互调</td><td>—</td><td>—</td><td>-29</td><td>—</td><td>dBm</td></tr></table>

表 28: 接收器特性 - 低功耗蓝牙 125 Kbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>灵敏度 @30.8% PER</td><td>—</td><td>—</td><td>-103.5</td><td>—</td><td>dBm</td></tr><tr><td>最大接收信号 @30.8% PER</td><td>—</td><td>—</td><td>8</td><td>—</td><td>dBm</td></tr><tr><td>共信道抑制比 C/I</td><td>F = F0 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td rowspan="8">邻道选择性抑制比 C/I</td><td>F = F0 + 1 MHz</td><td>—</td><td>1</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 1 MHz</td><td>—</td><td>2</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 2 MHz</td><td>—</td><td>-26</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 2 MHz</td><td>—</td><td>-26</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 3 MHz</td><td>—</td><td>-36</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 3 MHz</td><td>—</td><td>-39</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 + 3 MHz</td><td>—</td><td>-42</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 - 3 MHz</td><td>—</td><td>-43</td><td>—</td><td>dB</td></tr><tr><td>镜像频率</td><td>—</td><td>—</td><td>-42</td><td>—</td><td>dB</td></tr><tr><td rowspan="2">邻道镜像频率干扰</td><td>F = Fimage + 1 MHz</td><td>—</td><td>-43</td><td>—</td><td>dB</td></tr><tr><td>F = Fimage - 1 MHz</td><td>—</td><td>-36</td><td>—</td><td>dB</td></tr></table>

表 29: 接收器特性 - 低功耗蓝牙 500 Kbps  

<table><tr><td>参数</td><td>描述</td><td>最小值</td><td>典型值</td><td>最大值</td><td>单位</td></tr><tr><td>灵敏度 @30.8% PER</td><td>—</td><td>—</td><td>-100</td><td>—</td><td>dBm</td></tr><tr><td>最大接收信号 @30.8% PER</td><td>—</td><td>—</td><td>8</td><td>—</td><td>dBm</td></tr><tr><td>共信道抑制比 C/I</td><td>F = F0 MHz</td><td>—</td><td>4</td><td>—</td><td>dB</td></tr><tr><td rowspan="8">邻道选择性抑制比 C/I</td><td>F = F0 + 1 MHz</td><td>—</td><td>1</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 1 MHz</td><td>—</td><td>0</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 2 MHz</td><td>—</td><td>-24</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 2 MHz</td><td>—</td><td>-24</td><td>—</td><td>dB</td></tr><tr><td>F = F0 + 3 MHz</td><td>—</td><td>-37</td><td>—</td><td>dB</td></tr><tr><td>F = F0 - 3 MHz</td><td>—</td><td>-39</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 + 3 MHz</td><td>—</td><td>-38</td><td>—</td><td>dB</td></tr><tr><td>F &gt; F0 - 3 MHz</td><td>—</td><td>-42</td><td>—</td><td>dB</td></tr><tr><td>镜像频率</td><td>—</td><td>—</td><td>-38</td><td>—</td><td>dB</td></tr><tr><td rowspan="2">邻道镜像频率干扰</td><td>F = Fimage + 1 MHz</td><td>—</td><td>-42</td><td>—</td><td>dB</td></tr><tr><td>F = Fimage - 1 MHz</td><td>—</td><td>-37</td><td>—</td><td>dB</td></tr></table>

# 5 模组原理图

模组内部元件的电路图。

![](images/603d5bb998cfdf26d5a6996967ce6a9fbf0aa28b22ea47e1d7e8a8a77e428dc3.jpg)  
图5: ESP32-S3-WROOM-1 原理图

![](images/55d27b012da768890a02235b8e28d2704e98a778880472a96dd60503dbeaf7bb.jpg)  
图6: ESP32-S3-WROOM-1U 原理图

在内置PSRAM的模组中，芯片已通过eFuse设置将VDD_SPI电压固定为3.3V或1.8V，因此这些模组的VDD_SPI电压不受GPIO45电平影响；但在使用其他模组时，请确保模组上电时外部电路不会将GPIO45拉高。

# 6 外围设计原理图

模组与外围器件（如电源、天线、复位按钮、JTAG接口、UART接口等）连接的应用电路图。

![](images/83ed667bab73e67c5696f153e1a9811056536f9dd1b4948d50342c66d8173d97.jpg)  
图7：外围设计原理图

- EPAD 可以不焊接到底板，但是焊接到底板的 GND 可以获得更好的散热特性。如果您想将 EPAD 焊接到底板，请确保使用适量焊膏，避免过量焊膏造成模组与底板距离过大，影响管脚与底板之间的贴合。  
- 为确保 ESP32-S3 芯片上电时的供电正常, EN 管脚处需要增加 RC 延迟电路。RC 通常建议为  $R = 10 \mathrm{k} \Omega$ ,  $C = 1 \mu \mathrm{F}$ , 但具体数值仍需根据模组电源的上电时序和芯片的上电复位时序进行调整。ESP32-S3 芯片的上电复位时序图可参考《ESP32-S3 系列芯片技术规格书》>章节电源。

# 7 模组尺寸和PCB封装图形

# 7.1 模组尺寸

![](images/8b0cac7f4457bf31febd41f5fcfe9d9ffb1ec24504c5d13a20423693f558bb25.jpg)  
图8: ESP32-S3-WROOM-1 模组尺寸

![](images/ebcdf783c66f577c14a7c4fcca1e02c8f5cc55a30d056347f192c08cfb6b4f24.jpg)  
图9: ESP32-S3-WROOM-1U模组尺寸

# 说明：

有关卷带、载盘和产品标签的信息，请参阅《乐鑫模组包装信息》。

# 7.2 推荐PCB封装图

本章节提供以下资源供您参考：

- 推荐PCB封装图，标有PCB设计所需的全部尺寸。详见图10 ESP32-S3-WROOM-1推荐PCB封装图和图11 ESP32-S3-WROOM-1U推荐PCB封装图。  
- 推荐PCB封装图的源文件，用于测量图10和11中未标注的尺寸。您可用AutodeskViewer查看ESP32-S3-WROOM-1和ESP32-S3-WROOM-1U的封装图源文件。  
- ESP32-S3-WROOM-1 的 3D 模型。请确保下载的 3D 模型为.STEP 格式（注意，部分浏览器可能会加.txt后缀）。

![](images/d628d650a641b62af1ff4496d502844bcca3da6c6d6de58ddf24681637e70bd3.jpg)  
图10: ESP32-S3-WROOM-1 推荐PCB封装图

![](images/5942e3b88f9e52235e537573ae4763cbe422ff212fe6319a5d32a06613aa9cfa.jpg)  
图11：ESP32-S3-WROOM-1U推荐PCB封装图

# 7.3 外部天线连接器尺寸

ESP32-S3-WROOM-1U采用图12外部天线连接器尺寸图所示的第一代外部天线连接器，该连接器兼容：

- 广濑(Hirose)的U.FL系列连接器  
- I-PEX的MHFI连接器  
- 安费诺 (Amphenol) 的 AMC 连接器

![](images/ad59e375a733a8dd32886c1302cb6ae72deab7d8029dd29da3e4ee627387e5ec.jpg)  
图12: 外部天线连接器尺寸图

# 8 产品处理

# 8.1 存储条件

密封在防潮袋(MBB)中的产品应储存在  $< 40^{\circ} \mathrm{C} / 90 \% \mathrm{RH}$  的非冷凝大气环境中。

模组的潮湿敏感度等级MSL为3级。

真空袋拆封后，在  $25 \pm 5^{\circ} \mathrm{C}$  、 $60 \%$  RH下，必须在168小时内使用完毕，否则就需要烘烤后才能二次上线。

# 8.2 静电放电 (ESD)

- 人体放电模式 (HBM): ±2000 V  
- 充电器件模式 (CDM): ±500 V

# 8.3 炉温曲线

# 8.3.1 回流焊温度曲线

建议模组只过一次回流焊。

![](images/ff20c955613952a385ba48d4707245a33c2dc219aabbeb1b87507f473af30fc8.jpg)  
图13：回流焊温度曲线

# 8.4 超声波振动

请避免将乐鑫模组暴露于超声波焊接机或超声波清洗机等超声波设备的振动中。超声波设备的振动可能与模组内部的晶振产生共振，导致晶振故障甚至失灵，进而致使模组无法工作或性能退化。

# 9 相关文档和资源

# 相关文档

- 《ESP32-S3技术规格书》- 提供ESP32-S3芯片的硬件技术规格。  
- 《ESP32-S3技术参考手册》-提供ESP32-S3芯片的存储器和外设的详细使用说明。  
- 《ESP32-S3 硬件设计指南》- 提供基于 ESP32-S3 芯片的产品设计规范。  
- 《ESP32-S3系列芯片勘误表》- 描述 ESP32-S3 系列芯片的已知错误。  
- 证书

https://espressif.com/zh-hans/support/documents/certificates

- ESP32-S3产品/工艺变更通知(PCN)

https://espressif.com/zh-hans/support/documents/pcns?keys=ESP32-S3

- ESP32-S3 公告 - 提供有关安全、bug、兼容性、器件可靠性的信息

https://espressif.com/zh-hans/support/documents/advisories?keys=ESP32-S3

文档更新和订阅通知

https://espressif.com/zh-hans/support/download/documents

# 开发者社区

- 《ESP32-S3 ESP-IDF 编程指南》- ESP-IDF 开发框架的文档中心。  
- ESP-IDF 及 GitHub 上的其它开发框架

https://github.com/espressif

- ESP32 论坛 - 工程师对工程师 (E2E) 的社区，您可以在这里提出问题、解决问题、分享知识、探索观点。https://esp32.com/  
- The ESP Journal - 分享乐鑫工程师的最佳实践、技术文章和工作随笔。

https://blog.espressif.com/

- SDK和演示、App、工具、AT等下载资源

https://espressif.com/zh-hans/support/download/sdks-demos

# 产品

- ESP32-S3 系列芯片 - ESP32-S3 全系列芯片。

https://espressif.com/zh-hans/products/socs?id=ESP32-S3

- ESP32-S3 系列模组 - ESP32-S3 全系列模组。

https://espressif.com/zh-hans/products/modules?id=ESP32-S3

- ESP32-S3 系列开发板 - ESP32-S3 全系列开发板。

https://espressif.com/zh-hans/products/devkits?id=ESP32-S3

- ESP Product Selector（乐鑫产品选型工具）- 通过筛选性能参数、进行产品对比快速定位您所需要的产品。

https://products.espressif.com/#/product selector?language=zh

# 联系我们

- 商务问题、技术支持、电路原理图 & PCB 设计审阅、购买样品（线上商店）、成为供应商、意见与建议 https://espressif.com/zh-hans/contact-us/sales-questions

修订历史  

<table><tr><td>日期</td><td>版本</td><td>发布说明</td></tr><tr><td>2023-11-24</td><td>v1.3</td><td>·新增模组型号 ESP32-S3-WROOM-1-N16R16V 和 ESP32-S3-WROOM-1U-N16R16V 并更新相关信息
·更新表 2 ESP32-S3-WROOM-1U 系列型号对比中的表注
·更新章节 3.3.1 芯片启动模式控制
·更新章节 5 模组原理图中的模组原理图
·更新章节 7.1 模组尺寸中的模组尺寸图
·其他微小改动</td></tr><tr><td>2023-03-07</td><td>v1.2</td><td>·更新章节 3.3 Strapping 管脚
·更新章节 4.4 功耗特性
·更新章节 4.6.1 低功耗蓝牙射频发射器 (TX) 规格 中射频发射功率的最小值
·更新章节 6 外围设计原理图 中的描述
·在章节 7.2 推荐 PCB 封装图 中增加描述
·更新章节 9 相关文档和资源
·其他微小改动</td></tr><tr><td>2022-07-22</td><td>v1.1</td><td>·更新表 1 和 2
·其他微小改动</td></tr><tr><td>2022-04-21</td><td>v1.0</td><td>·更新低功耗蓝牙射频数据
·更新表 14 内的功耗数据
·添加认证和测试信息
·更新章节 3.3</td></tr><tr><td>2021-10-29</td><td>v0.6</td><td>全面更新，针对芯片版本 revision 1</td></tr><tr><td>2021-07-19</td><td>v0.5.1</td><td>预发布，针对芯片版本 revision 0</td></tr></table>

![](images/41942e3145b51594e9d7434f0d0c19f5586b3316dda5be9603cfd975a9abf6f0.jpg)

www.espressif.com

# 免责声明和版权公告

本文档中的信息，包括供参考的URL地址，如有变更，恕不另行通知。

本文档可能引用了第三方的信息，所有引用的信息均为“按现状”提供，乐鑫不对信息的准确性、真实性做任何保证。

乐鑫不对本文档的内容做任何保证，包括内容的适销性、是否适用于特定用途，也不提供任何其他乐鑫提案、规格书或样品在他处提到的任何保证。

乐鑫不对本文档是否侵犯第三方权利做任何保证，也不对使用本文档内信息导致的任何侵犯知识产权的行为负责。本文档在此未以禁止反言或其他方式授予任何知识产权许可，不管是明示许可还是暗示许可。

Wi-Fi 联盟成员标志归 Wi-Fi 联盟所有。蓝牙标志是 Bluetooth SIG 的注册商标。

文档中提到的所有商标名称、商标和注册商标均属其各自所有者的财产，特此声明。

版权归 © 2023 乐鑫信息科技（上海）股份有限公司。保留所有权利。