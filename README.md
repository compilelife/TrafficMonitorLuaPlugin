[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)是一个很受欢迎的任务栏监控工具。它还支持插件开发。然而……插件开发起来很麻烦。

本项目支持用lua开发TrafficMonitor插件，尽可能降低开发难度。


# 安装

在release页面下载zip后，解压到[TrafficMonitor的插件目录](https://github.com/zhongyang219/TrafficMonitorPlugins/blob/main/README.md)即可

压缩包自带若干lua编写的插件，可以在“显示设置”中启用。

# lua插件开发指南

可以参考[plugins](./plugins/)目录下的例子。

## 简单例子

模板如下：

```lua
-- 显示在“显示设置”里的名称
name="nvidia显卡监测"
-- 样例文本，会按此文本计算插件的显示宽度。尽可能用最大可能宽度的样例文本。
sample="显卡: 100%"
-- 调用onUpdate的频率，单位秒
interval=3

-- 每隔interval秒调用一次，返回值将被绘制到任务栏
function onUpdate()
	return "显卡: "..tf.runCmdLine("nvidia-smi.exe --query-gpu=utilization.gpu --format=csv,noheader")
end

-- 用户在插件文本上点击左键时触发
function onClick()
end
```

> 请注意：如果您是中文环境，需要将lua文件的编码设置为GBK

## 插件的选项

直接在lua里修改和保存即可，记得为用户提供详细说明。可参考[赚钱.lua](./plugins/lua/赚钱.lua)

## 显示图像

目前不支持lua绘制图像，但，可以支持绘制指定图标。

在`onUpdate`返回的文本中，出现`[xxx]`格式的文本，会认为是要绘制`xxx.ico`。

如```hello world[smile]```会被渲染为`hello world?`

应将`xxx.ico`放在lua文件同级的images目录下（将来可能变动）。

# Demo

在[plugins](./plugins/)目录下有我编写的几个例子

- 显卡监测.lua：调用`nvidia-smi`获取并显示GPU使用率
- 赚钱.lua：实时显示当日打工挣了多少钱（创意来自macos上的某个插件，忘记名字了……）

# 开发计划

- [ ] 开发一个可视化的“测试”界面，方便lua脚本编写时测试
- [ ] 图标显示优化；以及考虑把图标和每个lua脚本放一起，而不是都放在images里
- [ ] 继续上传更多有用/有趣的插件
