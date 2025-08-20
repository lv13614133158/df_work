1、文件树		
|
|---build
|
|---output|---debug	
|		  |---release
|		
|---lib(常用lib快捷方式）	
|
|---include(输出接口头文件)	
|
|---third_part
|		openssl	
|			 include
|			 lib
|			 src
|			 readme
|		websocket
|			 include
|			 lib	
|		curl
|			 include
|			 lib
|       pcap
|            include
|                  pcap
|            lib
|       spdlog
|            include
|            src
|            cmake
|
|             
|---tier1_part		
|		quecte	
|			include
|			lib
|			src
|			Qualcomm
|		neusoft	             
|
|---function		
|		common	
|			src
|			include
|		networkManger	
|			src
|			include
|		networkMonitor
|			src
|			include	
|		fileMonitor
|			src
|			include
|		resourceMonitor	
|			src
|			include
|		sysconfigMonitor
|			src
|			include	
|		processMonitor
|			src
|			include	
|		wbesocket_api
|			src
|			include
|		filemonitor
|			src
|			include
|		configParse
|			src
|			include	
|
|---readme	文件树、编译方式、产出获取、日志log	
|			
|---config目录		
|		
|---srcipt	
|		
|---cmake文件	
|	
		
					
		
		
						
		
		

2、注意
2.1移远封装模块放到 sysconfigMonitor模块下了，同时networkMonitor也使用了。



3、版本，说明
v0.0.1 分布式版本   --东风日产

v1.0.0 集成版本

v1.1.0 cJson 注册协商成功

v1.1.1 配置文件先获取本地，后对比

v1.1.2 拉取配置基本完整，完整的防火墙规则

v1.1.4 防火墙去iptables字样，websocket通信成功

v1.1.5 添加rpc成功

v1.2.0 更改日志格式，添加各个模块功能

V1.2.0 功能全部更新完成

V2.0.0 - 1.2.1 功能更新，添加版本功能

V2.0.2 - 1.2.2 networkmanager功能优化，改变工具链  --武汉东风M18

V1.2.5  适配武汉东风   

V1.2.6  event添加版本，流量监测修改

V1.2.7  改变工具链 -- 5G
        用户登陆加补丁

V1.2.12 封版
	添加脚本

V1.2.14 网卡改为配置表-配置

V1.2.15 东风封版，
	配置文件版本号，
        DNS事件修改，
 	进程上报方式修改，
	添加东风双向认证，联友SDK，
        编译时增加rpath，从相对路径中去找动态库，不受设备环境影响，
	修改设备信息的配置文件，
	流量采集间隔时间配置修改，
	优化日志打印。

V1.2.16 回归主线
        添加离线版

V1.2.17 改进网络模块，删除防火墙部分  --2022/12/23
        添加网络入侵开关由云端配置
	添加可配置变更网络攻击阈值
	添加可配置记录网络攻击阈值

V1.2.18 添加系统运行时间，IDPS运行时间 --2023/01/12
	心跳上报运行时间

V1.2.19 添加靶场系统版本 --2023/02/07

V1.2.20 修改DNS功能




	
		
