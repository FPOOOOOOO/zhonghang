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
3、还要看一下报告里1.4G的还差哪些

## <span id = "10">0923</span>
1、发射功率到 15 ，走廊上测，拔掉天线连不上才行 \
2、如果衰减不下去，是有本体噪声的影响，要通过1排除这个影响 \
3、还要看一下报告里1.4G的还差哪些

## <span id = "11">0924</span>
1、发射功率到 15 ，走廊上测，拔掉天线连不上才行 \
2、如果衰减不下去，是有本体噪声的影响，要通过1排除这个影响 \
3、还要看一下报告里1.4G的还差哪些 —— 已经标红,在PPT里整理了\
4、增加了ETH透传部分，测试用——还需要确定mesh是否可以在无认证的情况下使用 \

## <span id = "12">0927</span>
1、以太网init mac time out —— 是不是GPIO0时钟的问题 \
2、以太网透传又失败了，服了—— 看看0505的版本如何，先保证2.4G板子没问题，然后再移植到1.4G，现在的1.4G STA ETH在2.4G上也很差（有可能是GPIO0的问题）\

## <span id = "13">0928</span>
1、开始寻找20M的代码 ！ 找到了，就是eth里的两个\
2、进行1.4G的改编 \
3、拔掉天线很好，安上天线很差 —— 不是以太网芯片的问题 \

## <span id = "14">0929</span>
1、自己板子2.4G先测一下mesh\
2、mesh root这里会有连续的child node connect和disconnet——没有加break，傻逼了\
3、mesh感觉都没有断过，丢包率很好，但是没有拉远，衰减同样衰减不下去。\
4、可以考虑测一下mesh的eth ping的情况怎么样\
5、在ETH kit上只改时钟 \
6、衰减测试上先用ETH PING一下 \
7、WiFI调制方式是否对相位敏感 \
8、本振的相位现在是指向性的 \
9、无人机组网、战争\
10、37.5米 一开始40 比18.75多了7db然后按衰减，衰减到-69不变了、
第二次测试什么都不按89、
第三次连不上了

## <span id = "15">0930</span>
1、本振相位改后，离得很近还是rssi很低，要看长包的丢包率\
2、看本振改后PING有没有截图上的惨淡。\
3、调整请求超时 —— ping包生存时间

## <span id = "16">1002</span>
1、由于RSSI不知道为什么2米降到了-60，所以要以PING包作为测试基准，将mesh加上ETH透传，再比较直连ETH透传。——1.4Gmesh（root）0929 \

## <span id = "17">1006</span>
1、1.4G mesh（root）0929 里的Multi Addr是自己也会收一份下来    \
2、esp.emac: no mem for receive buffer \
3、找到原因了，不能自己收一份，要单独除了Root以外做一个组播，取消往串口传送能减少延时。\
4、TODO mesh ETH透传测速、测衰减

## <span id = "18">1007</span>
1、1.4G的mesh长包丢包率很高、1.4G的直连 基本ping不通\
2、我们明天做直接实际的通信实验看下，一是串口gps数据通信， 二是spi数据通信。我带台笔记本过来\

## <span id = "18">1008</span>
1、QT 不是user的成员，先把QTUDP下面的moc_user.cpp删掉，clean-qmake-build，后续如果出现了LNK2019的问题，那就[点这里](https://blog.csdn.net/shinefriend/article/details/109405042?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-109405042-blog-108044482.pc_relevant_3mothn_strategy_and_data_recovery&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-109405042-blog-108044482.pc_relevant_3mothn_strategy_and_data_recovery&utm_relevant_index=1) ，现在可以在小WIN上开发了\
2、GPS模块的波特率是9600，数据格式如下  \
~~~
$GNRMC,000223.000,V,3905.845246,
N,11721.051131,E,0.000,0.000,,,E
,N*2B
$GNGGA,000223.000,3905.845246,N,11721.051131,E,0,00,127.0
00,-61.479,M,0,M,,*7F
$GNGLL,39
05.845246,N,11721.051131,E,00022
3.000,V,N*59
$GNGSA,A,1,,,,,,,,,,,,,127.000,127.000,127.000*2A

$GNGSA,A,1,,,,,,,,,,,,,127.000,
127.000,127.000*2A
~~~
\
3、明天测一下串口透传先