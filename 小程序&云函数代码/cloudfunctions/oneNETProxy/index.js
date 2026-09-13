// cloudfunctions/oneNETProxy/index.js
const cloud = require('wx-server-sdk');
cloud.init({
  env: cloud.DYNAMIC_CURRENT_ENV
});
// ============================================
// 敏感配置：安全存放在云函数中，不暴露给前端
// 【部署前请替换下面占位符为你自己OneNET平台信息】
// ============================================
const CONFIG = {
  baseUrl: 'https://iot-api.heclouds.com',
  product_id: 'YOUR_ONENET_PRODUCT_ID',
  device_name: 'YOUR_ONENET_DEVICE_NAME',
  // Token，自行在OneNET平台生成
  authHeader: 'YOUR_ONENET_AUTH_TOKEN'
};
/**
 * 云函数入口
 * @param {Object} event - 前端传入参数
 * @param {string} event.action - 'get' 获取设备属性 / 'set' 设置期望值
 * @param {Object} event.params - 当 action='set' 时，传入要设置的参数，如 { mode: true } 或 { cmd: true }
 */
exports.main = async (event, context) => {
  const { action, params } = event;
  
  console.log('云函数调用:', { action, params });
  
  let url, method, data;
  
  if (action === 'get') {
    url = '/thingmodel/query-device-property';
    method = 'GET';
    data = {
      product_id: CONFIG.product_id,
      device_name: CONFIG.device_name
    };
  } else if (action === 'set') {
    url = '/thingmodel/set-device-desired-property';
    method = 'POST';
    data = {
      product_id: CONFIG.product_id,
      device_name: CONFIG.device_name,
      params: params
    };
  } else {
    return {
      code: -1,
      msg: '无效的 action 参数，必须是 "get" 或 "set"'
    };
  }
  
  try {
    const axios = require('axios');
    
    const result = await axios({
      url: CONFIG.baseUrl + url,
      method: method,
      timeout: 15000,  // 延长到 15 秒
      headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json',
        'Authorization': CONFIG.authHeader
      },
      params: method === 'GET' ? data : undefined,
      data: method === 'POST' ? data : undefined
    });
    
    console.log('OneNET 响应:', result.data);
    
    return {
      code: 0,
      msg: 'success',
      data: result.data
    };
  } catch (err) {
    console.error('请求 OneNET 失败:', err.message);
    return {
      code: -1,
      msg: err.message || '网络请求失败'
    };
  }
};
