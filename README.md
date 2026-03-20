## 基于freej2me的j2me模拟器，支持3D游戏
参考：

[freej2me](https://github.com/hex007/freej2me)

[J2ME-Loader](https://github.com/nikita36078/J2ME-Loader)

[JL-Mod](https://github.com/woesss/JL-Mod)

在miyoomini 、gkdminiplus、ubuntu18上进行了测试

编译：

Java JAR：
```
ant -Dvariant=linux-aarch64
```

前端使用sdl2，需编译出sdl_interface：
```
cd cpp/sdl2
make
```
可通过 `TOOLCHAIN` 变量指定交叉编译工具链路径：
```
make TOOLCHAIN=/path/to/toolchain/bin/aarch64-xxx-
```

声音使用sdl2-mixer，需编译出libaudio

3D游戏支持，M3G需编译出libm3g（依赖EGL,GLES1），Mascot Capsule v3需编译出libmicro3d（依赖EGL,GLES2）

运行：
```
export JAVA_TOOL_OPTIONS='-Xverify:none -Djava.util.prefs.systemRoot=./.java -Djava.util.prefs.userRoot=./.java/.userPrefs -Djava.library.path=./'

java -jar freej2me-sdl.jar /path/to/game.jar [width height] [sound]
```
- width height 和 sound 均为可选参数，默认分辨率 240x320，音量 100
- 如果游戏目录下存在 `<游戏名>.conf` 配置文件（由OSD菜单自动生成），分辨率/机型/旋转将从配置文件加载
- 如果路径或文件名中包含分辨率提示（如 `320x240` 或 `320240`），首次启动时会自动匹配

可通过修改keymap.cfg，自定义按键映射。

替换font.ttf，更改字体。

[按键说明](https://github.com/aweigit/freej2me-miyoomini/blob/master/KEYMAP.md)

![都市摩天楼](https://github.com/aweigit/freej2me-miyoomini/blob/master/img/ubuntu18.png)

