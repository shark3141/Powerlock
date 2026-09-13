// pages/index/index.js
Page({
  data: {
    // 四个设备属性
    mode: false,        // false=自动模式，true=手动模式
    cmd: false,
    lockstate: false,
    streamstate: false,
    
    // UI 状态
    isLoading: false,
    isPosting: false,       // mode 切换中
    isCmdPosting: false,    // cmd 发送中（分开控制，互不影响）
    lastUpdateTime: '等待加载...',
    errorMsg: ''
  },

  onLoad: function() {
    var that = this;
    setTimeout(function() {
      that.fetchData();
    }, 300);
  },

  formatTime: function(date) {
    var d = date || new Date();
    var month = (d.getMonth() + 1).toString();
    var day = d.getDate().toString();
    var hours = d.getHours().toString();
    var minutes = d.getMinutes().toString();
    var seconds = d.getSeconds().toString();
    
    if (month.length < 2) month = '0' + month;
    if (day.length < 2) day = '0' + day;
    if (hours.length < 2) hours = '0' + hours;
    if (minutes.length < 2) minutes = '0' + minutes;
    if (seconds.length < 2) seconds = '0' + seconds;
    
    return month + '-' + day + ' ' + hours + ':' + minutes + ':' + seconds;
  },

  parsePropertyData: function(dataArray) {
    var result = { mode: false, cmd: false, lockstate: false, streamstate: false };
    if (!Array.isArray(dataArray)) return result;
    
    for (var i = 0; i < dataArray.length; i++) {
      var item = dataArray[i];
      var id = item.identifier;
      var val = item.value;
      
      if (typeof val === 'string') {
        val = val.toLowerCase() === 'true';
      } else if (typeof val === 'number') {
        val = Boolean(val);
      }
      
      if (id === 'mode') result.mode = val;
      else if (id === 'cmd') result.cmd = val;
      else if (id === 'lockstate') result.lockstate = val;
      else if (id === 'streamstate') result.streamstate = val;
    }
    return result;
  },

  // 获取设备数据（通过云函数）
  fetchData: function() {
    var that = this;
    if (that.data.isLoading) return;
    
    that.setData({ isLoading: true, errorMsg: '正在连接...' });
    
    wx.cloud.callFunction({
      name: 'oneNETProxy',
      data: { action: 'get' }
    }).then(function(res) {
      var result = res.result;
      
      if (result.code === 0) {
        var oneNETResponse = result.data;
        var dataArray = [];
        
        if (oneNETResponse && oneNETResponse.code === 0 && Array.isArray(oneNETResponse.data)) {
          dataArray = oneNETResponse.data;
        } else if (Array.isArray(oneNETResponse.data)) {
          dataArray = oneNETResponse.data;
        } else if (Array.isArray(oneNETResponse)) {
          dataArray = oneNETResponse;
        }
        
        var parsed = that.parsePropertyData(dataArray);
        
        that.setData({
          mode: parsed.mode,
          cmd: parsed.cmd,
          lockstate: parsed.lockstate,
          streamstate: parsed.streamstate,
          lastUpdateTime: that.formatTime(new Date()),
          errorMsg: ''
        });
      } else {
        that.setData({
          errorMsg: result.msg || '获取失败',
          lastUpdateTime: that.formatTime(new Date())
        });
      }
    }).catch(function(err) {
      that.setData({
        errorMsg: '云函数调用失败',
        lastUpdateTime: that.formatTime(new Date())
      });
    }).finally(function() {
      that.setData({ isLoading: false });
    });
  },

  // 切换到自动模式
  setAutoMode: function() {
    var that = this;
    // 如果已经是自动模式，不重复操作
    if (that.data.mode === false) {
      wx.showToast({ title: '已经是自动模式', icon: 'none' });
      return;
    }
    // 如果正在切换中，不重复操作
    if (that.data.isPosting) return;
    
    that.setMode(false);
  },

  // 切换到手动模式
  setManualMode: function() {
    var that = this;
    // 如果已经是手动模式，不重复操作
    if (that.data.mode === true) {
      wx.showToast({ title: '已经是手动模式', icon: 'none' });
      return;
    }
    // 如果正在切换中，不重复操作
    if (that.data.isPosting) return;
    
    that.setMode(true);
  },

  // 发送 mode 设置（通过云函数）
  setMode: function(newMode) {
    var that = this;
    
    // 设置切换中状态
    that.setData({ 
      isPosting: true, 
      errorMsg: '' 
    });
    
    // 显示加载提示
    wx.showLoading({ 
      title: '切换中...', 
      mask: true 
    });
    
    wx.cloud.callFunction({
      name: 'oneNETProxy',
      data: {
        action: 'set',
        params: { mode: newMode }
      }
    }).then(function(res) {
      var result = res.result;
      
      // 隐藏加载提示
      wx.hideLoading();
      
      if (result.code === 0) {
        var oneNETResponse = result.data;
        
        if (oneNETResponse && oneNETResponse.code === 0) {
          // 乐观更新界面
          that.setData({ 
            mode: newMode,
            lastUpdateTime: that.formatTime(new Date())
          });
          
          // 成功提示
          wx.showToast({ 
            title: newMode ? '已切换到手动模式' : '已切换到自动模式', 
            icon: 'success',
            duration: 2000
          });
          
          // 延迟刷新（等待设备同步）
          setTimeout(function() { 
            that.fetchData(); 
          }, 2500);
        } else {
          // 失败提示
          that.setData({ 
            errorMsg: oneNETResponse.msg || '切换失败' 
          });
          wx.showToast({ 
            title: '切换失败', 
            icon: 'error',
            duration: 2000
          });
        }
      } else {
        // 失败提示
        that.setData({ 
          errorMsg: result.msg || '请求失败' 
        });
        wx.showToast({ 
          title: '切换失败', 
          icon: 'error',
          duration: 2000
        });
      }
    }).catch(function(err) {
      // 隐藏加载提示
      wx.hideLoading();
      
      that.setData({ 
        errorMsg: '网络连接失败' 
      });
      wx.showToast({ 
        title: '网络异常', 
        icon: 'error',
        duration: 2000
      });
    }).finally(function() {
      that.setData({ isPosting: false });
    });
  },

  // 切换 cmd（仅手动模式可用）
  toggleCmd: function() {
    var that = this;
    
    // 只有在手动模式下才能操作 cmd
    if (!that.data.mode) {
      wx.showToast({ 
        title: '请先切换到手动模式', 
        icon: 'none',
        duration: 2000
      });
      return;
    }
    
    // 如果正在发送中，不重复操作
    if (that.data.isCmdPosting) return;
    
    var newCmd = !that.data.cmd;
    
    that.setData({ 
      isCmdPosting: true, 
      errorMsg: '' 
    });
    
    // 显示加载提示
    wx.showLoading({ 
      title: '发送中...', 
      mask: true 
    });
    
    wx.cloud.callFunction({
      name: 'oneNETProxy',
      data: {
        action: 'set',
        params: { cmd: newCmd }
      }
    }).then(function(res) {
      var result = res.result;
      
      // 隐藏加载提示
      wx.hideLoading();
      
      if (result.code === 0) {
        var oneNETResponse = result.data;
        
        if (oneNETResponse && oneNETResponse.code === 0) {
          // 乐观更新界面
          that.setData({ 
            cmd: newCmd, 
            lastUpdateTime: that.formatTime(new Date()) 
          });
          
          // 成功提示
          wx.showToast({ 
            title: newCmd ? '命令已开启' : '命令已关闭', 
            icon: 'success',
            duration: 2000
          });
          
          // 延迟刷新
          setTimeout(function() { 
            that.fetchData(); 
          }, 2500);
        } else {
          // 失败提示
          that.setData({ 
            errorMsg: oneNETResponse.msg || '命令发送失败' 
          });
          wx.showToast({ 
            title: '发送失败', 
            icon: 'error',
            duration: 2000
          });
        }
      } else {
        // 失败提示
        that.setData({ 
          errorMsg: result.msg || '请求失败' 
        });
        wx.showToast({ 
          title: '发送失败', 
          icon: 'error',
          duration: 2000
        });
      }
    }).catch(function(err) {
      // 隐藏加载提示
      wx.hideLoading();
      
      that.setData({ 
        errorMsg: '网络连接失败' 
      });
      wx.showToast({ 
        title: '网络异常', 
        icon: 'error',
        duration: 2000
      });
    }).finally(function() {
      that.setData({ isCmdPosting: false });
    });
  },

  onPullDownRefresh: function() {
    this.fetchData();
    setTimeout(function() { 
      wx.stopPullDownRefresh(); 
    }, 1000);
  }
});