# 通用
- 上传到github仓库
~~~
git add -u
git status
git commit -m "0831"
git push origin ZHONGHANGmdf
~~~


# Debug日志


## <span id = "1">0906</span>

1、Qt节点拓扑是否以实际的节点数量来 \
2、广播和点对点发送的地址在哪儿设置，通过上位机在代码里设置，还是在ETH上直接对着ip发\
3、软件 Reboot 主从等参数\
4、加入SPI \
5、调整mesh网络层数 \
6、Qt有个进程一直在收UDP

## <span id = "2">0913</span>

1、Qt节点拓扑是否以实际的节点数量来 \
2、广播和点对点发送的地址在哪儿设置，通过上位机在代码里设置，还是在ETH上直接对着ip发 —— 有路由选择\
3、软件 Reboot 主从等参数\
4、加入SPI \
5、调整mesh网络层数 \
6、Qt有个进程一直在收UDP \
7、~~Qt 选择网口~~
8、测试1.4G 4351
9、撰写报告

## <span id = "3">0915</span>

```
 [15:27:30:577]Rx←?I (9081) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (9091) wifi:state: init -> auth (b0)
I (9101) wifi:state: auth -> assoc (0)

[15:27:30:643]Rx←?I (9151) wifi:state: assoc -> run (10)

[15:27:34:616]Rx←?I (13121) wifi:state: run -> init (fc0)
I (13121) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (13131) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (13131) ADF4351: retry to connect to the AP[0m
[0;32mI (13131) 
[15:27:34:672]Rx←?ADF4351: connect to the AP fail[0m

[15:27:36:673]Rx←?[0;32mI (15181) ADF4351: retry to connect to the AP[0m
[0;32mI (15181) ADF4351: connect to the AP fail[0m

[15:27:36:724]Rx←?I (15231) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (15231) wifi:state: init -> auth (b0)
I (15241) wifi:state: auth -> assoc (0)

[15:27:36:768]Rx←?I (15271) wifi:state: assoc -> run (10)

[15:27:40:757]Rx←?I (19261) wifi:state: run -> init (fc0)
I (19261) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (19271) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (19271) ADF4351: connect to the AP fail[0m
[0;32mI (19271) ADF4
[15:27:40:809]Rx←?351: Failed to connect to SSID:fpoap, password:jjjkkk05[0m

```

1、改环路滤波器 \
2、跳过四次握手阶段——看一下四次握手阶段的包长\
3、串口干扰到锁相环

## <span id = "4">0916</span>
1、包长增加 —— 确实包长增加了导致丢包很严重，但是误码率还行，就是丢包\
2、楼里测距 —— 失败了，测试数据见PPT


## <span id = "5">0917</span>
1、包长增加——在增益好的情况下，还是通不过 \
- 把WiFi发射增益减小，10dB，15dB:
2、楼里测距——完成了，天线的增益和匹配问题

## <span id = "6">0919</span>
1、跳过认证阶段测速—— 先用自带的iperf测试，和2.4G进行比较\
2、用了新天线，改到20dB \
3、saving new calibration data because of checksum failure, mode(0) 、\
4、←?I (16388) wifi:bcn_timout,ap_probe_send_start \
5、没有进行DHCP的分发

## <span id = "7">0920</span>
1、换了天线以后，进行了距离的测试\

## <span id = "8">0921</span>
1、妈的数据丢了，进行一个图的绘制 ——linux可以用red

## <span id = "9">0922</span>
1、发射功率到 15 ，走廊上测，拔掉天线连不上才行 \
2、如果衰减不下去，是有本体噪声的影响，要通过1排除这个影响 \
3、
