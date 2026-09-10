# APEX 迷你卡丁车

独立原型，端口 5175。npm install 后 npm run dev；npm run build 生成 dist。

支持 WASD/方向键驾驶、空格急刹、触屏按钮、跟车/俯视切换、圈速与本地最佳纪录。按赛道顺序经过三个检查区再跨过起点计圈。草地减速，外圈简化碰撞。并非真实赛车物理。

## Android / AR 移植边界

- 驾驶更新在 src/driving.js，无 DOM 或渲染依赖，可由不同输入/显示层调用。
- 所有模型为本地程序几何，没有外部素材请求。触控设备默认降低像素比和阴影分辨率。
- 已加入 Capacitor Android 原生容器工程；Android / AR 眼镜性能与兼容性仍需真机验证。
- 后续根据眼镜型号、系统和厂商 SDK 选择 Android 容器或 XR 接入。双目渲染、头部追踪、手柄输入和热功耗测试仍待实现，不能假定 Android WebView 支持眼镜 AR。


## 构建 Android

使用 Capacitor 7、Java 21、Android SDK 35。原生应用将 dist 资源内置到安装包，不连接开发服务器。Activity 固定为横屏（允许横屏翻转），使用硬件加速 WebView。

```sh
npm install
npm run android:sync
cd android
gradlew.bat assembleDebug
```

本机 Java 位于 C:/Program Files/Android/Android Studio/jbr；SDK 路径由 android/local.properties 设置（不提交到 Git）。Android Studio 可直接打开 android 目录。

输出：android/app/build/outputs/apk/debug/app-debug.apk。Debug APK 用于安装测试，并非商店发布签名版。最低安装 SDK 为 23，但实际 3D 渲染要求设备有可用的 WebGL2 WebView/GPU；不能只根据 Android 版本保证运行。

原生封装参考：https://capacitorjs.com/docs/android
