## 1. 项目说明

MuMeStar (Mutiple Menu Star for OLED) 是一个用于单色OLED屏幕的多级菜单，采用多叉树结构，底层逻辑自洽，移植简单，可无限扩展。相关代码全部开源，持续更新。

| 相关链接 |
| :------: |
| 源码下载地址(Github)：[MuMeStar (MutipleMenu_Star for OLED) ](https://github.com/13081032491/MuMeStar) |
| 源码下载地址(WriteBug)：[MuMeStar (MutipleMenu_Star for OLED) ](https://www.writebug.com/code/74f70a1e-1c04-11ef-a772-0242c0a81018)  |
|代码移植方法、详细扩展教程地址：[📚 MuMeStar (MutipleMenu_Star for OLED) 详解](https://www.writebug.com/article/2b4ea580-1bfe-11ef-a772-0242c0a81018) |

由于个人时间、精力和水平有限，项目难免存在不足，望读者不吝指正。

## 2. 目前成果
 
最新版菜单目录：
- Main (Parent)
	- Settings (Parent)
		- Fontsize: (Data): 自由调节菜单字号
		- Brightness (Data): 自由调节屏幕亮度
		- Color Mode (Data): 设置白天/黑夜模式
		- Refresh Rate (Data): 设置屏幕刷新率
		- Mirror Flip -Ho (Data): 水平镜像翻转
		- Mirror Flip -Ver (Data): 垂直镜像翻转
	- Hello (Parent)
		- Sayhello (Once): "Hello MuMeStar!"
		- Smile (Loop): 眼珠不断转动 
	- About (Once): 显示项目信息
	- Game (Parent)
		- Dinosaur(Loop): 小恐龙游戏 
	- Menu_5 (Parent)
	- Menu_6 (Parent)

## 3. 版本区别说明
下表说明了各版本间的区别，可根据需要选择合适版本：
 | 版本 | 说明| 适用范围|
 | :------: | :------: |:-: |
 | Version 1.0.X| 具有最基本的按键状态机和菜单框架，IIC，标准库 | 是手搓菜单的优良参考 |
 | Version 1.1.X | 具有较完整的菜单框架，IIC，HAL库 | 适合剩余内存极小的项目 |
 | Version 1.2.X | 具有完整的菜单框架，兼容SPI和IIC，HAL库 | 适合以菜单为核心框架的项目 |

## 4. 更新日志

| 日期 | 版本 | 概况 | 详细 | 
| :------: | :------: | :------: | :------: |
| 2024.6.1 | V1.2.0 | 兼容通信，优化代码 | 兼容SPI+IIC通信，仅需修改一行代码即可完成通信协议切换<br>优化显存处理 |
| 2024.5.23 | V1.1.1 | 修复问题，新增功能 | 优化了按键状态机的位置与按键总控的作用形式，将按键处理位置从中断回调移至程序主循环；新增 "Smile" 功能； Parent型菜单右侧显示"..."号。<br>修复了进入子菜单时滑动条显示异常的问题，修复了16字号下返回父菜单光标显示异常的问题。<br> 新增Chrome浏览器小恐龙游戏 |
| 2024.5.13 | V1.1.0 | 修复问题，完善功能，优化美观，规范代码 | 实现菜单选项数目（子菜单数）可超过屏幕最大显示数目；加入菜单属性，构建与之适配的按键状态机，菜单功能多样性得以实现；新增功能 "Brightness" 实现屏幕亮度自由调节；修复了 OLED 驱动函数 "显示非8倍数字号的字符（如12号，20号等）时会侵占下方像素点" 的常见问题 ；形成了较为完善的多级菜单。<br>新增 "Sayhello" 功能 ；修复了08字号下 previous 键功能异常的问题，修复了16字号下菜单与光标显示异常的问题；更改字号结构体，菜单字号大小可选。<br>新增 " Fontsize " 选项，菜单字号大小可自由选择，新增右侧滑动条； 修改IIC通信为DMA模式，提高屏幕帧率。<br> 修复了V2.2中DMA传输数据过快（开启下次DMA中断过快）时中断开启失败导致的数据缺失问题。<br> 修复了 OLED_WR_Byte() 函数形参datasize未使用的问题添加关键字，避免代码对其它文件产生影响；添加部分注释。 |
| 2024.5.7 | V1.0.0 | 建立底层 |提出菜单底层数据结构，建立菜单基本框架；实现菜单基本显示功能；实现按键 Choose, Enter 功能，按键无延迟。<br>优化菜单底层数据结构，重构菜单基本框架；实现 invalid operation 检测，约束未定义行为；实现按键 Choose, Enter, Return 功能；|
| 2024.5.4 | - | 项目准备 | 建立 OLEDSD (OLED Software Driver) |

