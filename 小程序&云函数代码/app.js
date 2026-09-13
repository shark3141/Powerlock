// app.js
App({
  onLaunch() {
    console.log('PowerLockSim 小程序启动');
    
    // 获取系统信息
    const systemInfo = wx.getSystemInfoSync();
    console.log('系统信息:', systemInfo.platform, systemInfo.version);
    
    // 初始化云开发
    if (!wx.cloud) {
      console.error('请使用 2.2.3 或以上基础库以支持云开发');
    } else {
      wx.cloud.init({
        // ============================================
        // ⚠️ 请替换为您的云环境 ID
        // 获取方式：微信公众平台 → 开发 → 云开发 → 顶部环境ID
        // 格式类似：powerlock-7gabc123
        // ============================================
        env: 'CLOUD_ENV_ID_PLACEHOLDER',
        traceUser: true
      });
      console.log('云开发初始化成功');
    }
  },
  globalData: {
    deviceConfig: {
      product_id: 'PRODUCT_ID_PLACEHOLDER',
      device_name: 'DEVICE_ID_PLACEHOLDER'
    }
  }
});
