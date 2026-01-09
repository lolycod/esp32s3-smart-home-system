/**
 * 设备协议管理器
 * 统一管理设备控制协议，提供兼容性处理和验证
 */

class DeviceProtocolManager {
    constructor() {
        // 协议版本
        this.version = '2.0';

        // 支持的设备类型
        this.deviceTypes = {
            LED: 'LED',
            FAN: 'FAN',
            CURTAIN: 'CURTAIN'
        };

        // 设备处理器映射 - 模块化设备控制
        this.deviceHandlers = {
            '风扇': this.handleFanDevice.bind(this),
            'LED灯': this.handleLEDDevice.bind(this),
            '窗帘': this.handleCurtainDevice.bind(this)
        };

        // 默认参数
        this.defaults = {
            LED: {
                colorTemp: 'C',  // 冷光
                brightness: 50   // 50%亮度
            },
            FAN: {
                speed: 1         // 1档风速
            }
        };
    }

    /**
     * 解析并标准化设备命令
     * @param {string} command - 原始命令
     * @returns {Object} 标准化的命令对象
     */
    parseCommand(command) {
        if (!command || typeof command !== 'string') {
            throw new Error('命令不能为空');
        }

        const cmd = command.trim().toUpperCase();

        // LED命令处理
        if (cmd.startsWith('LED')) {
            return this.parseLEDCommand(cmd);
        }

        // 风扇命令处理
        if (cmd.startsWith('FAN')) {
            return this.parseFANCommand(cmd);
        }

        // 窗帘命令处理
        if (cmd.startsWith('CURTAIN')) {
            return this.parseCurtainCommand(cmd);
        }

        throw new Error(`不支持的命令格式: ${command}`);
    }

    /**
     * 解析LED命令（支持新旧格式）
     * @param {string} cmd - LED命令
     * @returns {Object} 解析结果
     */
    parseLEDCommand(cmd) {
        const result = {
            device: 'LED',
            original: cmd,
            version: '2.0'
        };

        // 新格式: LED<开关><色温><亮度> 例如: LED1C70, LED0W30
        const newFormatMatch = cmd.match(/^LED([01])([CW])(\d{1,2})$/);
        if (newFormatMatch) {
            result.switch = newFormatMatch[1] === '1';
            result.colorTemp = newFormatMatch[2];
            result.brightness = parseInt(newFormatMatch[3]);
            result.standardized = cmd;
            return result;
        }

        // 旧格式兼容: LED<编号><状态> 例如: LED11, LED20
        const oldFormatMatch = cmd.match(/^LED(\d)([01])$/);
        if (oldFormatMatch) {
            const ledNum = parseInt(oldFormatMatch[1]);
            const state = oldFormatMatch[2] === '1';

            result.ledNumber = ledNum;
            result.switch = state;
            result.colorTemp = this.defaults.LED.colorTemp;
            result.brightness = state ? this.defaults.LED.brightness : 0;
            result.version = '1.0';
            result.standardized = `LED${state ? '1' : '0'}${result.colorTemp}${String(result.brightness).padStart(2, '0')}`;
            result.converted = true;
            return result;
        }

        // LEDALL格式兼容: LEDALL<状态> 例如: LEDALL1, LEDALL0
        const allFormatMatch = cmd.match(/^LEDALL([01])$/);
        if (allFormatMatch) {
            const state = allFormatMatch[1] === '1';

            result.allLeds = true;
            result.switch = state;
            result.colorTemp = this.defaults.LED.colorTemp;
            result.brightness = state ? this.defaults.LED.brightness : 0;
            result.version = '1.0';
            result.standardized = `LED${state ? '1' : '0'}${result.colorTemp}${String(result.brightness).padStart(2, '0')}`;
            result.converted = true;
            return result;
        }

        throw new Error(`无效的LED命令格式: ${cmd}`);
    }

    /**
     * 解析风扇命令
     * @param {string} cmd - 风扇命令
     * @returns {Object} 解析结果
     */
    parseFANCommand(cmd) {
        const result = {
            device: 'FAN',
            original: cmd,
            version: '2.0'
        };

        // 格式: FAN<开关><档速> 例如: FAN13, FAN00
        const match = cmd.match(/^FAN([01])(\d)$/);
        if (match) {
            result.switch = match[1] === '1';
            result.speed = parseInt(match[2]);
            result.standardized = cmd;

            // 验证风速范围 (0-5档)
            if (result.speed < 0 || result.speed > 5) {
                throw new Error(`风扇档速范围错误: ${result.speed}, 应为0-5`);
            }

            return result;
        }

        throw new Error(`无效的风扇命令格式: ${cmd}`);
    }

    /**
     * 解析窗帘命令
     * @param {string} cmd - 窗帘命令
     * @returns {Object} 解析结果
     */
    parseCurtainCommand(cmd) {
        const result = {
            device: 'CURTAIN',
            original: cmd,
            version: '2.0'
        };

        // 格式: CURTAIN<开关> 例如: CURTAIN1, CURTAIN0
        const match = cmd.match(/^CURTAIN([01])$/);
        if (match) {
            result.switch = match[1] === '1';
            result.standardized = cmd;
            return result;
        }

        throw new Error(`无效的窗帘命令格式: ${cmd}`);
    }

    /**
     * 验证命令格式
     * @param {string} command - 待验证的命令
     * @returns {boolean} 是否有效
     */
    validateCommand(command) {
        try {
            this.parseCommand(command);
            return true;
        } catch (error) {
            return false;
        }
    }

    /**
     * 获取标准化命令
     * @param {string} command - 原始命令
     * @returns {string} 标准化后的命令
     */
    getStandardizedCommand(command) {
        const parsed = this.parseCommand(command);
        return parsed.standardized;
    }

    /**
     * 生成设备命令（从AI JSON转换）- 模块化处理
     * @param {Object} deviceControl - AI返回的设备控制JSON
     * @param {string} source - 命令来源 ('AI' | 'CONTROL' | 'DEBUG')
     * @returns {Array} 标准化命令数组
     */
    generateCommands(deviceControl, source = 'AI') {
        const commands = [];

        // 遍历所有设备类型，使用对应的处理器
        for (const [deviceName, deviceConfig] of Object.entries(deviceControl)) {
            if (deviceConfig && this.deviceHandlers[deviceName]) {
                try {
                    const command = this.deviceHandlers[deviceName](deviceConfig, source);
                    if (command) {
                        // 添加命令元数据
                        const commandWithMeta = {
                            command: command,
                            device: deviceName,
                            source: source,
                            timestamp: Date.now()
                        };
                        commands.push(commandWithMeta);
                    }
                } catch (error) {
                    console.error(`设备处理器错误 [${deviceName}]:`, error.message);
                }
            }
        }

        return commands;
    }

    /**
     * 风扇设备处理器
     * @param {Object} config - 风扇配置
     * @param {string} source - 命令来源
     * @returns {string} 风扇控制命令
     */
    handleFanDevice(config, source = 'AI') {
        const onOff = config.开关 === "开" ? "1" : "0";
        const speed = config.风速 || 0;
        return `FAN${onOff}${speed}`;
    }

    /**
     * LED设备处理器
     * @param {Object} config - LED配置
     * @param {string} source - 命令来源
     * @returns {string} LED控制命令
     */
    handleLEDDevice(config, source = 'AI') {
        const onOff = config.开关 === "开" ? "1" : "0";
        const temp = config.色温 === "冷光" ? "C" : "W";
        const brightness = String(config.亮度).padStart(2, '0');

        // 对于控制模式的LED，使用简化格式（向后兼容）
        if (source === 'CONTROL') {
            return `LED${onOff}${temp}${brightness}`;
        }

        // AI模式使用标准格式
        return `LED${onOff}${temp}${brightness}`;
    }

    /**
     * 窗帘设备处理器
     * @param {Object} config - 窗帘配置
     * @param {string} source - 命令来源
     * @returns {string} 窗帘控制命令
     */
    handleCurtainDevice(config, source = 'AI') {
        const onOff = config.开关 === "开" ? "1" : "0";
        return `CURTAIN${onOff}`;
    }

    /**
     * 注册新设备处理器 - 方便扩展
     * @param {string} deviceName - 设备名称
     * @param {Function} handler - 设备处理函数
     */
    registerDeviceHandler(deviceName, handler) {
        this.deviceHandlers[deviceName] = handler.bind(this);
        console.log(`✅ 设备处理器已注册: ${deviceName}`);
    }

    /**
     * 获取支持的设备列表
     * @returns {Array} 支持的设备名称列表
     */
    getSupportedDevices() {
        return Object.keys(this.deviceHandlers);
    }

    /**
     * 示例：扩展新设备 - 智能插座
     * 展示如何轻松添加新设备支持
     */
    initializeExampleDevices() {
        // 示例：添加智能插座设备处理器
        this.registerDeviceHandler('智能插座', (config, source = 'AI') => {
            const onOff = config.开关 === "开" ? "1" : "0";
            const socket = config.插座编号 || 1;
            return `SOCKET${socket}${onOff}`;
        });

        console.log(`📱 扩展设备初始化完成，支持设备: ${this.getSupportedDevices().join(', ')}`);
    }

    /**
     * 获取协议说明文档
     * @returns {Object} 协议文档对象
     */
    getProtocolDocumentation() {
        return {
            version: this.version,
            description: '智能家居设备控制协议 v2.0',
            devices: {
                LED: {
                    format: 'LED<开关><色温><亮度>',
                    examples: [
                        'LED1C70 - 开启,冷光,70%亮度',
                        'LED0W00 - 关闭,暖光,0%亮度'
                    ],
                    parameters: {
                        switch: '0=关闭, 1=开启',
                        colorTemp: 'C=冷光, W=暖光',
                        brightness: '00-99=亮度百分比'
                    },
                    compatibility: {
                        'LED11': 'LED1C50 (LED1开,默认冷光50%)',
                        'LED20': 'LED0C00 (LED2关)',
                        'LEDALL1': 'LED1C50 (全部开启)'
                    }
                },
                FAN: {
                    format: 'FAN<开关><档速>',
                    examples: [
                        'FAN13 - 开启,3档风速',
                        'FAN00 - 关闭'
                    ],
                    parameters: {
                        switch: '0=关闭, 1=开启',
                        speed: '0-5=风速档位'
                    }
                },
                CURTAIN: {
                    format: 'CURTAIN<开关>',
                    examples: [
                        'CURTAIN1 - 打开窗帘',
                        'CURTAIN0 - 关闭窗帘'
                    ],
                    parameters: {
                        switch: '0=关闭, 1=打开'
                    }
                }
            }
        };
    }
}

module.exports = DeviceProtocolManager;