# AndroidSurfaceImgui

## 介绍

+ 支持RPC进行多进程/远程绘制。
+ 支持鼠标滚轮
+ 支持物理键盘输入
+ 支持横/竖屏自动切换

在Android使用上API创建Surface并进行[Dear ImGui](https://github.com/ocornut/imgui.git)的绘制。

**注意**：如果拥有Surface的进程的用户权限在`shell`之下，则你的Surface不会被系统添加到根视图，也就是不会显示出来，详情请看：[SurfaceFlinger.cpp](https://cs.android.com/android/platform/superproject/main/+/main:frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp;l=4769)

## 支持系统版本

仅标注大版本，小版本没特别说明的都是支持的。

| 古董版本 | 过渡版本 | 中间版本 | 新版本 |
| --- | --- | --- | --- |
| ✅ Android5 | ✅ Android8 | ✅ Android9 | ✅ Android12 |
| ✅ Android6 |  | ✅ Android10 | ✅ Android13 |
| ✅ Android7 |  | ✅ Android11 | ✅ Android14 |
|  |  |  | ✅ Android15 |

## 编译

0. 确保在项目根目录打开命令行终端 。
1. 执行`git submodule init` 。
2. 执行`git submodule update` 。
3. 执行`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=android-23 -DANDROID_ABI=${PLATFORM} -S . -B build -G Ninja`，其中`${NDK_PATH}`需要替换为你的NDK路径，`${PLATFORM}`替换成你想要编译出的目标平台。
4. 执行`cmake --build build --config Release`。
5. 编译完成。

你也可以直接使用`build.bat`来进行批量编译，默认编译`armeabi-v7a arm64-v8a x86 x86_64`四个版本，可修改脚本中的`ANDROID_ABIS`变量来进行更改。

脚本有三个可选参数分别为：NDK路径、最低支持SDK版本、CMake程序路径，不设置则脚本自动检测`NDK_PATH`与CMake工具链，如果都没有则使用脚本默认内置路径。

## 使用

例子请看：[src/test-ui/main.cc](https://github.com/Bzi-Han/AndroidSurfaceImgui/blob/main/src/test-ui/main.cc)

<img src="https://github.com/Bzi-Han/AndroidSurfaceImgui/blob/main/screenshot.jpg" height="500px" />

## RPC使用

1. Server：[src/test-ui/canvas.cc](https://github.com/Bzi-Han/AndroidSurfaceImgui/blob/main/src/test-ui/canvas.cc)
2. Client：[src/test-ui/render.cc](https://github.com/Bzi-Han/AndroidSurfaceImgui/blob/main/src/test-ui/render.cc)

[screenshot.webm](https://github.com/Bzi-Han/AndroidSurfaceImgui/assets/75075077/7b6f7adc-2b68-44d1-bf7a-53bcf0a151a3)

## TODO

+ [ ] 重构`ANativeWindowCreator.h`使得逻辑更加清晰，目前所有版本的兼容实现都挤在一起导致代码和日志都难以阅读(一坨)。
+ [ ] 整理一下`Android`在`PC(Windows)`上的远程绘制Demo。

## 特别鸣谢赞助

感谢以下赞助者对本项目的支持(不分先后)：

+ [不解释](https://github.com/Dasongzi1366)
+ [barry-ran](https://github.com/barry-ran)

谢谢大家的支持！❤️
