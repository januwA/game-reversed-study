## 修改ce ui
- https://www.youtube.com/watch?v=eH_t06-fWT8
- https://www.cheatengine.org/forum/viewtopic.php?t=608754&sid=2147e5c8753c7db4443da259771abffe
- https://forum.cheatengine.org/viewtopic.php?t=570055
- https://forum.cheatengine.org/viewtopic.php?t=558871

## 查看指定ui模块
```
ctrl = wincontrol_getControl(MainForm,1)
ctrl.Visible = not ctrl.Visible

-- or
local c = MainForm.Panel9
c.visible = not c.visible
```

## 设置图片
- https://wiki.cheatengine.org/index.php?title=Help_File:Script_engine
- 
```
local c = MainForm.Panel6
--c.visible = not c.visible

local img = createImage(c)
-- img.setPicture('C:\\Users\\ajanuw\\Pictures\\ce.jpg')
img.loadImageFromFile('C:\\Users\\ajanuw\\Pictures\\ce.jpg')
img.Align = alClient
img.Top = 0
img.Left = 0
img.Visible = true
img.height = 300
img.width = 300
```
```
local c = MainForm.Panel10

local img = createImage(c)
img.loadImageFromFile('C:\\Users\\ajanuw\\Pictures\\ce3.jpg')
img.Top = 50
img.Left = 0
img.Visible = true
img.Height = 140
img.Width = 100
```