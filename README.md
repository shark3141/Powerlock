# PowerLock —— OneNET 智能锁物联网演示系统

> 简单修改密钥信息即可使用


**免责声明**：本项目仅用于高校物联网课程学习、竞赛演示，不能直接作为入户门锁等安全产品使用，作者不承担任何实际使用带来的安全与财产风险。

# 项目简介

一套完整的物联网智能锁演示系统，技术栈：**STM32 \+ ESP8266 \+ 微信小程序 \+ 微信云开发 \+ OneNET 云平台**。

- **硬件端**：STM32 主控 \+ ESP8266 WiFi 模组，接入 OneNET 物联网平台，上报锁状态、充电状态，接收小程序下发的控制指令。

- **小程序端**：微信小程序可视化面板，查看设备实时状态，切换自动 / 手动模式，手动下发开锁控制命令。

- **后端代理**：微信云函数作为 API 中转，OneNET 鉴权密钥全部保存在云函数内，前端小程序不暴露任何密钥，防止密钥泄露。

# 整体架构

STM32 \+ ESP8266 硬件设备 ↔ OneNET 云平台 ↔ 微信云函数代理 ↔ 微信小程序前端

```text
STM32 采集锁状态
    │  串口 115200
ESP8266（MQTT 接入 OneNET）
    │
OneNET 云平台（物模型属性：mode / cmd / lockstate / streamstate）
    │  HTTP API（云函数代理）
微信云函数 oneNETProxy（保存鉴权信息）
    │  wx.cloud.callFunction
微信小程序前端
```

# 前置准备

部署前必须准备好以下账号与资源，后续所有 **YOUR\_XXX** 占位符的内容都来自这里。

|所需资源|用途|获取方式|
|---|---|---|
|微信小程序 AppID|导入并运行小程序|微信公众平台注册小程序|
|微信云环境 ID|云函数运行环境|微信公众平台 → 开发 → 云开发 → 顶部环境 ID|
|OneNET 产品 ID|识别产品|OneNET 平台创建产品|
|OneNET 设备名称|识别设备|OneNET 平台创建设备|
|OneNET 鉴权串（Authorization）|调用 OneNET OpenAPI 鉴权|OneNET 平台生成 2022 版鉴权信息|
|开发环境|编译与联调|微信开发者工具、STM32 开发环境（Keil）|

# 快速开始：小程序端部署

## 克隆代码

```bash
git clone https://github.com/shark3141/Powerlock.git
cd Powerlock
```

## 导入项目

1. 打开微信开发者工具，选择「导入项目」。

2. 项目目录选择仓库内 **miniprogram** 文件夹。

3. AppID 填写你自己小程序的 AppID（替换代码中的 **YOUR\_WX\_APPID**）。

## 替换脱敏占位符（重点）

仓库代码中所有带 **YOUR\_** 前缀的内容均为占位符，必须替换为你自己的真实信息后才能运行。对照下表逐一替换。

|占位符|所在文件|替换为|获取位置|
|---|---|---|---|
|YOUR\_WX\_APPID|project\.config\.json|小程序 AppID|微信公众平台|
|YOUR\_CLOUD\_ENV\_ID|app\.js|云环境 ID|微信云开发控制台|
|YOUR\_PRODUCT\_ID|app\.js、云函数|OneNET 产品 ID|OneNET 平台|
|YOUR\_DEVICE\_NAME|app\.js、云函数|OneNET 设备名称|OneNET 平台|
|YOUR\_ONENET\_AUTH\_HEADER|云函数|OneNET 鉴权串|OneNET 平台生成|

### project\.config\.json

找到 appid 字段并替换：

```json
"appid": "YOUR_WX_APPID"
```

### app\.js

```javascript
wx.cloud.init({
  env: 'YOUR_CLOUD_ENV_ID', // 替换为你的微信云开发环境 ID
  traceUser: true
});

globalData: {
  deviceConfig: {
    product_id: 'YOUR_PRODUCT_ID',  // OneNET 产品 ID
    device_name: 'YOUR_DEVICE_NAME' // OneNET 设备名称
  }
}
```

### 云函数 oneNETProxy/index\.js

```javascript
const CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  product_id: 'YOUR_PRODUCT_ID',          // OneNET 产品 ID
  device_name: 'YOUR_DEVICE_NAME',        // OneNET 设备名称
  authHeader: 'YOUR_ONENET_AUTH_HEADER'   // OneNET 鉴权 Authorization 串
};
```

## 云函数安装依赖并部署

1. 在 **cloudfunctions/oneNETProxy** 文件夹内打开终端，执行依赖安装：

```bash
npm install
```

会自动安装 **wx\-server\-sdk** 与 **axios**。

**第 2 步**：在微信开发者工具中，右键 **oneNETProxy**，选择「上传并部署：云端安装依赖」。

云函数部署完成后，小程序与 OneNET 通信的核心代理即就绪。这是最常见的遗漏点，部署后可在云开发控制台查看云函数日志验证。

## 编译预览

点击开发者工具「编译」，即可预览小程序界面；点击「刷新状态」按钮，即可通过云函数拉取 OneNET 设备属性。

# 硬件端部署（STM32 \+ ESP8266）

仓库内硬件代码同样做了脱敏，需要替换：WiFi 名称、WiFi 密码、OneNET 产品 ID、设备名称、设备鉴权信息。

1. 打开 ESP8266 源码。

2. 替换 WiFi SSID、WiFi Password 为你自己的路由器信息。(或者直接用手机热点+密码即可)

3. 修改 OneNET 产品 ID、设备名、设备鉴权信息（对应 **YOUR\_PRODUCT\_ID**、**YOUR\_DEVICE\_ID**、**YOUR\_AUTH\_TOKEN** 等占位符）。

4. 编译并烧录进 STM32，ESP8266 上电联网后自动连接 OneNET 平台。

5. 设备上线后，小程序面板即可读取设备上报的 4 个属性。

|属性|类型|读写|说明|
|---|---|---|---|
|mode|bool|可读写|自动 / 手动模式|
|cmd|bool|可读写|手动控制命令|
|lockstate|bool|只读|锁状态（上锁 / 未上锁）|
|streamstate|bool|只读|充电状态|

# OneNET 平台配置

1. 在 OneNET 平台「产品模型」中创建 4 个物模型属性，**标识符必须与代码严格一致（区分大小写）**：mode、cmd、lockstate、streamstate，均为布尔型。

2. 物模型读写权限：mode、cmd 设为**可读写**（小程序下发期望值，设备接收）；lockstate、streamstate 设为**只读**（仅硬件上报状态）。

3. 生成 2022 版 OpenAPI 鉴权串，填入云函数的 authHeader 字段。

标识符大小写不一致是「数据拉取为空」最常见的根因，配置时请逐字符核对。

# 功能说明

- 下拉刷新 / 点击「刷新状态」：从 OneNET 拉取设备最新状态。

- 自动模式：硬件自主控制锁，小程序 cmd 按钮禁用。

- 手动模式：小程序可发送 cmd 指令控制锁动作。

- 实时显示锁状态、充电状态、最后更新时间；网络异常时展示错误提示。

# 常见问题排错

## 云函数调用失败

- 检查云函数是否部署成功，云开发控制台查看调用日志。

- 检查云开发环境 ID 填写正确。

- 云开发控制台 → 安全设置，添加 OneNET 域名白名单：iot\-api\.heclouds\.com

## 获取设备数据返回空

- OneNET 物模型标识符大小写必须与代码完全一致。

- 设备必须在线，并成功上报属性到平台。

- 核对 Product ID、Device Name、鉴权串。

## 下发设置期望值失败

- 检查物模型属性是否开启「可写」权限。

- 鉴权串是否在有效期内。

# 项目文件目录

```text
PowerLockSim/
├── miniprogram/                 # 微信小程序前端目录
│   ├── app.js
│   ├── app.json
│   ├── app.wxss
│   ├── project.config.json
│   ├── sitemap.json
│   ├── pages/
│   │   └── index/
│   │       ├── index.js
│   │       ├── index.json
│   │       ├── index.wxml
│   │       └── index.wxss
│   └── cloudfunctions/
│       └── oneNETProxy/         # OneNET 代理云函数
│           ├── index.js
│           └── package.json
├── hardware/                    # STM32 + ESP8266 硬件源码
├── docs/                        # 原理图、部署文档、图片
├── .gitignore
└── README.md
```

# 部署核对清单

* [ ] 微信小程序 AppID 替换完成（project\.config\.json）

* [ ] 微信云环境 ID 替换完成（app\.js）

* [ ] 云函数内 OneNET Product ID、Device Name、鉴权串替换完成

* [ ] 云函数执行 npm install，并上传部署成功

* [ ] OneNET 平台创建对应 4 个物模型，标识符完全匹配

* [ ] 硬件代码 WiFi、OneNET 信息替换，设备成功上线

* [ ] 云开发后台添加域名白名单 iot\-api\.heclouds\.com

# License 与声明

MIT License，仅供学习交流，禁止用于商业产品与安防工程。OneNET 平台服务、商标与文档版权归属中国移动，使用 OneNET 平台请自行阅读并遵守其用户协议。
