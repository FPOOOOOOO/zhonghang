# 通用
- 上传到github仓库
~~~
git add -u
git status
git commit -m "0831"
git push origin ZHONGHANGmdf
~~~


# Debug日志


之前的在[语雀](https://www.yuque.com/u834735/nrtf4r/egagyw)

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
$GPGSV,3,1,10,5,30,47,,6,55,300,,8,61,115,,9,15,231,*4D
$GPGSV,3,2,10,12,19,186,,14,17,58,,18,66,172,,19,80,,*7E
$GPGSV,3,3,10,27,38,47,,28,12,290,*47
$GNRMC,210000.000,V,3905.845246,N,11721.051131,E,0.000,0.000,,,E,N*2B
$GNGGA,210000.000,3905.845246,N,11721.051131,E,0,00,127.000,-61.479,M,0,M,,*7F
$GNGLL,3905.845246,N,11721.051131,E,210000.000,V,N*59
$GNGSA,A,1,,,,,,,,,,,,,127.000,127.000,127.000*2A
$GNGSA,A,1,,,,,,,,,,,,,127.000,127.000,127.000*2A
~~~
\
3、明天测一下串口透传先 \

## <span id = "17">1009</span>
1、先供地,成了，能收到 \
2、2发1收:大部分-11，然后有几次变成-59；1发2收:就都是61\
3、2先不用，用1发3收，也都是-68——怀疑是1的发射链路出了问题。1收3发，-33，确实是1的发射链路出了问题\
4、2收3发 -83?; 2发3收: -67 \
5、撰写报告

## <span id = "18">1010</span>
1、换成长包，然后计数\
2、ARP包会占用esp资源，把esp以太网通信换成过滤ARP包，或者加一个eth网关\
3、以太网可能有些设置问题，先用直接通信数据测试速率和丢包看看= 写一个1.4G的iperf的测试\
4、确认一下mesh能不能用ETH栈。

## <span id = "19">1011</span>
1、1000包楼内测试18、25、50m+衰减没问题，就是串口打印有些慢，不知道为什么\
2、2.4G 会随着包长的变化，而导致吞吐量的变化，Mutilconsole 2.4G没问题，但是1.4G 无论是1460，1000，还是100的包长都是0.2M左右，不会变，而1460和1000都有如下的bug，找到下列bug的原因是关键：
~~~
mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:14, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:25
~~~
3、一直发是不行的，猜测是最底层有交换，要么用直连，要么再优化看看。\
4、不阻塞、2.4G半双工、直连测试 \

## <span id = "20">1012</span>
1、1000包长，楼外测    \
2、mesh 确认机制以及是否能拿掉，发送确认的时间能否长一些（不是mwifi write里的true false） \
3、间隔100ms的大包是什么（连发16次），还有一个喇叭型的 \
4、直连有没有ack、直连是否有交换\

## <span id = "21">1013</span>
1、
厕所采集的GPS 
~~~
$GNRMC,060846.000,A,3015.913072,N,12007.228755,E,0.786,23.543,270203,,E,A*05
$GNGGA,060846.000,3015.913072,N,12007.228755,E,1,04,3.417,28.970,M,0,M,,*53
$GNGLL,3015.913072,N,12007.228755,E,060846.000,A,A*49
$GNGSA,A,3,15,24,10,23,,,,,,,,,5.204,3.417,3.925*21
$GNGSA,A,3,,,,,,,,,,,,,5.204,3.417,3.925*23
$GPGSV,4,1,13,5,53,64,,6,29,271,,8,62,359,,1,63,212,,14,11,87,17,15,21,80,42,63,212,,14,11,87,17,15,21,80,42,18,40,187,*7C
$GPGSV,4,3,13,19,38,321,,22,1,37,,23,86,41,38,24,47,39,47*77
$GPGSV,4,4,13,28,58,288,*4E
$BDGSV,1,1,1,166,5,,21*5E
~~~
2、开题\
title:（面向飞行器的)地面检测无线组网单元系统研究 \
三个研究内容:(1)设计一套用于地面节点与机载节点通讯的硬件系统及其对应的软件开发，能够完成SPI、UART、ETH三路透传和1.4G WiFi 1主16从的组网，支持上位机上报消息和下发配置。\
(2)基于研究内容(1)中通讯系统进行所有功能上的元器件国产化替代，同时进行通讯指标的优化。\
(3)基于研究内容(1)(2)中的通讯系统进行新的组网形式，利用Mesh均衡网络负载和拓展性；同时主机-节点ETH透传，以及节点间透传上进行路由算法优化和功能调优；\
3、开关切换速度50ns，检波器6ns
4、2.4G也加一个WiFi放大器，做一个收发切换 \
5、把4、5块板子也测一下 \
6、GPS坐标转换到百度坐标转换 \
7、ack机制去掉/延长 \
8、1.4G暂时调成了链路增益0dB

## <span id = "22">1017</span>
1、原来有个度分秒有问题，[Here](http://old.openluat.com/GPS-Offset.html)有非常详细的转换和代码\

## <span id = "23">1018</span>
1、1.4G 开关深度改深了，还是不行 \
2、看一下2.4G加上收发切换以后的效果 \
3、mesh 确认机制以及是否能拿掉，发送确认的时间能否长一些（不是mwifi write里的true false） \
4、间隔100ms的大包是什么（连发16次），还有一个喇叭型的 \
5、WiFi边发边确认、可以单个确认或者批量确认\
6、同样的包，2.4G发送快(60us)，1.4G发送慢(200us) \
7、改了收发解耦\

## <span id = "24">1019</span>
1、准备放大线路上收发解耦——好像直连也不行，要试试mesh的\
2、都不行，推倒重来，怀疑是低噪放供电的问题（在改收发解耦的时候用的）\


## <span id = "25">1020</span>
1、左边的是发送通路，靠近天线这边的是滤波器，把它去掉，让收发走两个焊出来的天线\
2、目前和2.4G放大器结构一样，在收发通路上分别混频（这样容易滤波）——准备尝试总的一个混频，需要去公司找混频器\

## <span id = "26">1024</span>
1、进行2.4G的所有指标排查 \
2、这个可以在ping的时候打印时间戳（MACOS） WIN下PING分辨率0.2 MAC0.1
~~~
ping 192.168.4.88 -i 0.2 | while read pong; do echo "$(date): $pong"; done
~~~
3、SPI的顺序:15 13 12 14(ETH板子上)\
4、SPI透传在0707里，receiver是阻塞的\
5、如果要求1M是1000，那么就是8000000，如果1M是1024，那就是8388608\
6、2048任务栈下，SPItask里定义sendbuf168可以，169超标。改成4096可以。在2048下，把定义放到整个外面也可以\
7、一次发送1000Byte，delay10us，9000 000Hz，理论能够达到略小于9Mbit/s，实测: 8.28Mbit/s    \

## <span id = "27">1025</span>
1、新的ESP32芯片：
~~~
rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)

[12:04:3:724]Rx←?invalid header: 0xffffffff

[12:04:3:769]Rx←?invalid header: 0xffffffff   
~~~
2、Packet content transfer stopped (received 8 bytes) \
3、 4、5号板1.4Gmesh 4351 1000个包，20.09秒\
4、 直接控制发送接收电路（功放），栅压可能太低？\
5、 9177板子2.4G能通，但是信号很低，只有84; 800M 不通，N板CS0对地短路\
6、
~~~
Parent is disconnected, reason: 201[
~~~
[错误代码csdn](https://blog.csdn.net/Marchtwentytwo/article/details/118551075)  \   
[错误代码espressif](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/wifi.html#wi-fi-reason-code)\

7、功放一直开好连接（4-5），功放有选择的开关（3-5）不好连。\
8、（4-5）有bug
~~~
[16:46:2:199]Rx←?W (24395) mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:9, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:0
~~~

## <span id = "28">1026</span>
1、
~~~
现在1.4g收发可以稳定在22s，1000个包，这样差不多了，明天把以太网加上，如果ping没问题就差不多了，如何测速满足要求我们可以想个方案。后面测试就是以太网ok，spi ok，组网ok就好了。明天重点测以太网
~~~
2、5号：以太网上行√ 下行√（只有两次成功过？？）  4号 ：以太网上行√ 下行待测
~~~
ROOT：res = mwifi_root_write(Multiaddr, 1, &data_type, msg.packet, msg.length, true);
ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
NONROOT：
ret = mwifi_read(src_addr, &data_type, &buffer, &buffer_len, 100 / portTICK_RATE_MS);
res = mwifi_write(Rootaddr, &data_type, msg.packet, msg.length, true);
~~~
3、ETHkit 在0825两个上PING不会卡顿，到了0929就会卡顿？？人麻了，而且0825不会组播自己，0929会组播自己。\
4、根据3中，将0825直接加入1.4G。

## <span id = "29">1026</span>
1、1.4G 和2.4G 板子互换烧录的时候要改GPIO0的晶振和IP101的驱动！！！\
~~~
W (30970) mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:4, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:21
~~~

2、有16-26%的丢包率，20ms左右，但是前期有很多max_wnd和send flow control message failed or timeout。猜测和MAC地址设置有关，有3个TODO，一种是改MAC地址设置，一种是看混杂模式的具体原理，一种是学习David的，复制一份然后向上传一份（建议新开一个代码文件夹试）\
3、4、5号机分别与两个EThkit组成，然后发现4号机的以太网有点问题。\

## <span id = "30">1027</span>
1、5号机作为Nonroot加入的时候Root会报这个错     \
~~~
[10:22:47:860]Rx←?I (158688) mesh: [TXQ]<max:64>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:12), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (158689) mesh: [RXQ]<max:64 = cfg:64 + extra:0>self:0, <max:64 = cfg:64 + extr
~~~

## <span id = "31">1029</span>
1、5：没问题，有时候ping不通，ping通0%
9177R：1.4G有问题，以太网要先WiFi再插以太网才行，1.6%
4：先插以太网17%丢包，后插以太网0%丢包
3：先插以太网不通，后插以太网0%丢包
6：锁不住，以太网没问题，2.4G 0%
7：锁得住，以太网没问题，2.4G 0%
8：锁不住，以太网稍差，2.4G 4%
9：锁得住，以太网没问题，2.4G 0%

## <span id = "32">1031</span>
1、1.4G console要改一下，改成no router——因为没有1.4G的router\
~~~
5：没问题，有时候ping不通，ping通0%
9177R：1.4G有问题，以太网要先WiFi再插以太网才行，1.6%
4：先插以太网17%丢包，后插以太网0%丢包
3：先插以太网不通，后插以太网0%丢包
6：锁得住，以太网没问题，2.4G 0%
7：锁得住，以太网没问题，2.4G 0%
8：锁得住，以太网稍差，2.4G 4%
9：锁得住，以太网没问题，2.4G 0%
~~~
2、 79和45 差不多、68


## <span id = "33">1101</span>
1、esp.emac: no mem for receive buffer\
2、4、5号机，在```10261.4Geth2mesh(root)```里比``console test``里用jperf要慢好多——猜测是idf.py配置的问题\
3、Mwifi-Transmission-128(64)、240MHz(160)——tm的，改了以后不通了\

## <span id = "34">1102</span>
1、4号又烧了\
2、ETHkit————7号：128、240不行，以太网不通顺，改成128，160，就可以了？？

## <span id = "35">1103</span>
1、SR8201 21引脚没有上拉；12号引脚悬空了；\
2、 init phy failed、wrong chip ID：RST脚没有正确上拉；
~~~
[0;31mE (806) rtl8201: rtl8201_init(352): wrong chip ID[0m
[0;31mE (806) esp_eth: esp_eth_driver_install(223): init phy failed[0m
~~~
3、
~~~
[0;31mE (922) rtl8201: rtl8201_pwrctl(261): power up timeout[0m
[0;31mE (928) rtl8201: rtl8201_init(343): power control failed[0m
[0;31mE (934) esp_eth: esp_eth_driver_install(223): init phy failed[0m

[0;31mE (874) rtl8201: rtl8201_reset(167): reset timeout[0m
[0;31mE (874) rtl8201: rtl8201_init(346): reset failed[0m
~~~

## <span id = "36">1103</span>
1、21号引脚的2.49K也很关键\
2、以太网下行不行、


## <span id = "37">1106</span>
1、该换的都换了，准备重新铺铜\
2、以太网通讯前会有1010同步码，把1同步成1就是1，把1同步成0就是0（如果Tx两条线反了的话）\
3、8主 7从

## <span id = "38">1110</span>
1、找一下论文，找到了，仿真\
2、阿里云5G基站调试\
3、整体的变频是可以的，然后需要加一个
N-R 1.13
R-N 1.56

## <span id = "39">1113</span>
1、到实验室测下1.4G，我们测下外边变频到1.4g，然后用我们板上的1.4g收发切换加放大如何。板上的变频拿掉\
2、AP——2.4G功放收发切换——7M
AP——1.4G整体混频——1M
AP——2.4G功放收发切换——1.4G整体混频——连不上
AP——1.4G整体混频——板子切换电路——(1-1.5M左右)

6:开关切换灯不灭
8:0dB切换灯正常\
3、混频原理

## <span id = "40">1114</span>
1、到为何上电就download——EN少了一个100nF的RC延时电路(1.1uF、0.2uF)、IO0(不上拉、上拉20K、下接电容0.1uF)、3.3V上电快一点\

## <span id = "41">1116</span>
1、初始化没问题，但是上下行不通\
2、不知道为什么上下行有一次好了，但是上行误码率很高，下行对面能收到\

## <span id = "42">1117</span>
1、PC端改成10M是可以用的，改成100M就不行了，自协商也不行。
## <span id = "43">1118</span>
1、又不行了，只有上行，没有下行了，不知道改了什么(王老师说改了时钟，但是改回去了也不行？？)\

## <span id = "44">1119</span>
1、TXC上的电阻大小，影响输入到PHY的50M晶振的幅度，从而影响10M通信、100M通信，自适应\
2、具体来说就是22+5Ω 10M可以通信，1k自适应可以通信，122欧姆不行，1.12k不行，480可以，360不行，720\
3、芯片内部不说上拉和下拉就是M级的，如果说了弱上拉，100k左右，对地M级的上拉。

## <span id = "45">1124</span>
1、测试中航板子的ETH、UART、SPI\
2、是否发送新行会造成问题，发eee会造成重启:
~~~
[21:40:57:695]Rx←?55 41 52 54 3A 0D 0A CE D2 CA C7 D6 F7 BB FA 0D 00
[21:41:1:420]Rx←?55 41 52 54 3A 0D 0A CE D2 CA C7 D6 F7 BB 00
[21:41:16:281]Rx←?55 41 52 54 3A 0D 0A CE D2 CA C7 BB FA BB 00
[21:41:28:990]Rx←?55 41 52 54 3A 0D 0A CE D2 CA C7 BB FA BB FA 0D 00
~~~
3、新行把前面的头改成8试试看，eee的话，就不知道为什么了:
~~~
assert failed: block_locate_free heap_tlsf.c:447 (block_size(block) >= size)


Backtrace:0x4008189e:0x3ffbe0f00x40088b45:0x3ffbe110 0x4008f955:0x3ffbe130 0x4008d6c2:0x3ffbe250 0x4008e199:0x3ffbe270 0x40081a61:0x3ffbe290 0x400d8aaf:0x3ffbe2b0 0x400d95d1:0x3ffbe360 0x400d6a6
[21:57:41:538]Rx←?9:0x3ffbe3f0 0x4008c061:0x3ffbe420 


ELF file SHA256: 8358e5c9f441bed9
~~~

## <span id = "45">1125</span>
1、发送新行是字符串把最后一位强行变成了00，所以在接收的时候多增加一位，就不用改7个包头到8个了。\
2、eee好像是内存管理的问题，[这里](https://blog.csdn.net/gzxb1995/article/details/124504705)\
3、终端读取SPI数据时候一般作为slave，那么ZH的板子里IO34和IO2需要换网络位置\
4、同时这样不是IOMUX，所以速度达不到8M\
5、2号主机、1、3号主机（3号ETH有问题）\

## <span id = "46">1129</span>
1、进行了[计算机网络的学习](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)\


## <span id = "47">1130</span>
1、继续进行[计算机网络的学习](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)\
2、第五章没看、第六章只看了6.1DNS、第7章、第8章都没看

## <span id = "48">1201</span>
1、8201的TXC是O/PD，所以是下拉的。解决方案是我们这边\
2、2.4G能PING通是下面这组代码：
~~~
eth2mesh_0805
eth2meshroot_0811
~~~
3、ZH的是带ZH字样的代码\
4、继续进行[计算机网络的学习](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)，从第9章开始看\

## <span id = "49">1202</span>
1、IO0的上拉电阻改成1K，强上拉，10K-1.5K都不行，只有1K能上电启动还不影响以太网\
2、IO0上拉1K的基础上，改串联电阻。2K直接不通，1K没测，910Ω 20%丢包率。

## <span id = "50">1205</span>
1、6、7号机现在能上电启动，10K上拉，串联1K，WIN 7%、MAC 4.6%\
2、10K上拉，串联510欧姆，WIN 6% 、MAC 6%\
3、换电阻差距不大，只有通和不通的问题，准备对8201进行本地的PING。PHYRST20K，680nF\
~~~
10261.4Geth2mesh
10261.4Geth2meshroot
~~~
4、6号和8号，剩下的和7号。

## <span id = "51">1206</span>
1、明天我再测下变频，看看和老板子幅度相差多少，如果变频没有问题，理论下就应该通的。老板子之间连接没啥问题是吧\
2、节点之间的时间同步，通过802.11协议中的TSF进行、仿真采用的是瑞立信道模型、临时采集的数据存在PSRAM上    \

## <span id = "52">1207</span>
1、把发射功率变成10dB，没什么区别，新板子快的时候22-29ms，慢的时候45ms\
2、2.4GWiFi收发切换，一直使能、切换使能、PA一直使能、LNA一直使能都有影响，最好是分别使能\

## <span id = "53">1208</span>
1、组会\
2、最快的时间内交换点云数据\
3、旅晨阳 老师的学生：
沙漠  老师

## <span id = "54">1209</span>
1、要看wireless hart\

## <span id = "55">1213</span>
1、已经买了书，然后先写一下文档\
2、1213 1.4GMESH 1000Byte 10ms 10 dB 新板子A-新8 31.6ms RSSI -21\

## <span id = "56">1214</span>
1、新板子R几乎不通？RSSI -67\
2、1214 1.4GMESH 1000Byte 10ms 10 dB 新板子A-新8 18ms \
3、20dB基本连不上，15dB才连上。然后改开关切换到0dB，然后发现速度变慢了，再调回10dB\
4、发射dB和切换dB的差值决定了速度。20-12-10K，很慢，15-12 18ms，15-0-5.1K很慢，10-0也很慢\

## <span id = "57">1215</span>
1、一个是路上开关的隔离度差，一个是耦合器耦合过去的能量比较多，是一个对称的耦合器，来回的能量一遍一个是吸收电阻，一边是耦合电阻\

## <span id = "58">1216</span>
1、准备两套先给他们送过去\
2、我们的开关50ns，2.4G的开关10ns\

## <span id = "58">1216</span>
1、准备两套先给他们送过去\
2、我们的开关50ns，2.4G的开关10ns\

## <span id = "58">1217</span>
1、20dB-10K以太网是能通的，就是要等一会（12161.4GZH）\
2、从机SPI task有问题——两边都会出现 SPI Received（一上电就有） ——主机发UART的时候自己能收到（自己的从机的时候，ESP的从机不会有这个问题）——应该是\
3、之前10K是10dB切换，5K是0dB切换，现在10K是1dB切换。\
4、重新开始记录:
发送 A 10K-0dB、\
接收 B 12K-2dB、10K- -3-1dB左右、20K- 4-5dB、33K- 7-12dB（几乎连不上了）、27K - 5-10dB、24K - 4-9dB、15K - \


## <span id = "59">1219</span>
1、准备写上位机 和 GPS\

## <span id = "60">1225</span>
1、
老7号发，新B收，最快8-10ms
新A发，老6号收，最快20ms 
老7号发，老6号收，最快20ms
新A发，新B收，很慢，很难连上\

## <span id = "61">1226</span>
1、最快组1280和64160以后就会掉回到20ms\
2、使用iperf测试
2.54MBytes 9s
~~~
[13:38:42:363]Rx←?[0;32mI (39986) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  0.94 MBytes  2.51 Mbits/sec[0m

[13:38:45:587]Rx←?[0;32mI (43210) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  0.69 MBytes  1.85 Mbits/sec[0m

[13:38:48:598]Rx←?[0;32mI (46220) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  0.91 MBytes  2.42 Mbits/sec[0m

[13:38:51:607]Rx←?[0;32mI (49230) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.18 MBytes  0.49 Mbits/sec[0m
~~~
4.42MBytes 12s
~~~
[13:53:23:548]Rx←?[0;32mI (921150) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  1.05 MBytes  2.81 Mbits/sec[0m

[13:53:26:550]Rx←?[0;32mI (924152) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  1.19 MBytes  3.18 Mbits/sec[0m

[13:53:29:551]Rx←?[0;32mI (927153) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  1.19 MBytes  3.16 Mbits/sec[0m

[13:53:32:616]Rx←?[0;32mI (930216) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.99 MBytes  2.63 Mbits/sec[0m

[13:53:35:628]Rx←?[0;32mI (933230) [mwifi_test, 622]: [24:4c:ab:13:52:7c]  12-15 sec  0.16 MBytes  0.44 Mbits/sec[0m
~~~

6号发的表演，和7号差不多可以复刻了。
~~~
[16:43:49:760]Rx←?[0;32mI (24663) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  1.00 MBytes  2.68 Mbits/sec[0m

[16:43:52:762]Rx←?[0;32mI (27665) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  0.66 MBytes  1.75 Mbits/sec[0m

[16:43:55:763]Rx←?[0;32mI (30666) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  1.22 MBytes  3.27 Mbits/sec[0m

[16:43:58:772]Rx←?[0;32mI (33674) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.38 MBytes  1.01 Mbits/sec[0m
~~~

## <span id = "62">1227</span>
1、看看官方iperf在参数上有哪些需要改的:
240MHz
Mwifi_Transmission config

R-N jperf 2.7M iperf 5M
N-R jperf 1.7M
2、
高速检波器，变差，怀疑门限检波
3、
jperf 双向 1.4G速度有问题
TODO :
jperf 双向 2.4G速度看一下
jperf 提速给ZH提一套过去

4、jperf 1.4G 速度过高会报错
~~~

[18:26:20:942]Rx←?[0;33mW (114604) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
[0;31mE (114623) esp.emac: no m

[18:26:20:988]Rx←?em for receive buffer[0m
[0;31mE (114632) esp.emac: no mem for receive buffer[0m
[0;31mE (114634) esp.emac: no mem for receive buffer[0m
[0;31mE (114643) esp.emac: no mem for receive buffer[0m

[18:26:21:047]Rx←?[0;33mW (114705) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m

[18:26:21:141]Rx←?[0;33mW (114805) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m

[18:26:21:241]Rx←?[0;33mW (114905) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
[0;33mW (114905) [mwifi, 707]: <ESP_ERR_MESH_NO_MEMORY> Node failed to send packets, dest_addr: 24:4c:ab:20:e9:74, flag: 0x02, opt->type: 0x08, opt->len: 13, data->tos: 0, data: 0

[18:26:21:292]Rx←?x3ffde148, size: 1448[0m
[0;33mW (114918) [mwifi, 806]: <ESP_ERR_MESH_NO_MEMORY> Root node failed to send packets, dest_mac: 24:4c:ab:20:e9:74[0m
[0;33mW (114929) [mwifi, 1090]: <ESP_ERR_MESH_NO_MEMORY> mwifi_root_write, size: 1448[0m
[0;33mW (114937) [eth2mesh, 385]: <ESP_ERR_MESH_NO_MEMORY> mwifi_read[0m

[18:26:21:343]Rx←?[0;33mW (115005) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
~~~

## <span id = "63">1228</span>
1、
~~~
1216R1.4GZH
12161.4GZH
~~~
2.4G 部分
R—N jperf 3M(4M会呕吐) iperf  7.84M
N-R jperf 4M iperf  6.19M

## <span id = "63">1229</span>
1、
1.4G 部分

R——N
老板子——>新板子 4M
新板子<——老板子 0.4M

老板子<——新板子 
新板子——>老板子 M \
2、以太网部分改成SR8201F向ESP提供时钟，这样子ESP容易进入下载模式，但是100M的时候PING比较稳定（R40换成22欧姆）\

## <span id = "64">1230</span>
1、调试过程见纸张\
2、个人认为如果跑在单核上，还是只能对半，因为看到了mesh的 stack和wifi的不一样\

## <span id = "65">1231</span>
1、还能尝试的方向：
- 研究一下windows size
- try一下跑在不同的核上
- 这个网友的很多建议可以尝试 https://github.com/espressif/esp-mdf/issues/301

## <span id = "65">0103</span>
1、直连的AP和STA 差不多8-13M徘徊，没有0%过，总流量高计算出来的也高。然后AP ping STA时延高、STA ping AP 时延低（和github上的issue一样
2、?有点问题，flash没设置,但是ETH2AP官方例程也没设置\

## <span id = "66">0104</span>
1、task在不同的core上，好像效果会变差。task在不同的core相同优先级>不同优先级。task在同一个core上开了使用两个core会变差\
2、IRAM 已经都开启了\
3、iperf 跑双核没什么区别（本来就只有一个task）

## <span id = "67">0105</span>
1、测试一下Tx prior和RX prior的,好像有用又好像没用，没有很多可复现性\
2、xon_size还是有帮助的，具体参考纸上\

## <span id = "68">0110</span>
1、室外 1.4G 300米连不上、200米连不上、150米连不上、100米连上了，rssi-80。7R3N，7-3 0.02M，3-7 0.9M（15%丢包）。ping 6ms jperf时300-2000ms\
2、室内 1.4G 连得上，rssi-54，7R3N，3-7 1M没问题，ping 7ms，jperf时10ms \
3、室内 7R5N，同轴线RSSI -33，R-N 1M没问题，ping+jperf 10ms；N-R 只有0.6M，ping+jperf 200+ms 有send flow fail\
4、直天线，RSSI-27，R-N 1M没问题，ping+jperf 10+ms 有几个100+ms；N-R，0.3M、0.8M、1M都有，复现性比较差，ping+jperf 请求超时，没有send flow fail。加了一个10dB的衰减，从-27到-70了\
5、8号认证通不过、6号连不上、5号、10号、B都有一次或两次到过1M，然后其余是不稳定丢包，5号在稳定后还是挺稳定的。
6、10号和5号是老版型；10号同轴线，RSSI-36——ping：8ms，ping+Jperf 上行：1M 25+ms（好的时候ms不增加）下行：0.5M；5号同轴线，RSSI-27——ping：7ms，ping+Jperf 上行：1M 20+ms（2次），后面就变成0.7M（400+ms）下行：0.3M；5号加10dB衰减，RSSI-38——ping 8ms，ping+Jperf 上行：自从变成0.7M以后就一直没回去。总结：10号上行比5号稳定，3号上行最稳定；下行3号、10号、5号都有点问题。

## <span id = "69">0111</span>
7R3N
100米 2.4G rssi -83 1.4 连不上
90米 1.4G -81 0.55M 2.4G -77
60米 1.4G -79 1M 出现过一次 0.9M剩下 -2.4G -74
50米 1.4G -73 1M  2.4G -66

## <span id = "70">0131</span>
1、加一个IO让8201后复位
2、上位机的GPS分析

## <span id = "71">0207</span>
1、需要100米-200米以后然后走回来重启连接（设置一个重启器）
2、GPIO17 设置为Output模式以后，Download 0.7V，什么都不写正常工作1.9V，拉高以后3.3V，拉低以后1.9V。

## <span id = "72">0208</span>
1、要加一个ESP32_EN和8201 RST之间的二极管\
2、mesh一直重启以后\
~~~
I (639600) mesh: <MESH_NWK_LOOK_FOR_NETWORK>need_scan:0x2, need_scan_router:0x0, look_for_nwk_count:0
I (643513) mesh: [FIND][ch:0]AP:7, otherID:0, MAP:0, idle:0, candidate:0, root:0[00:00:00:00:00:00][FIXED-ROOT]
I (643513) mesh: [FIND:158]fail to find a network, channel:0, cfg<channel:13, router:, 00:00:00:00:00:00>

I (643521) mesh: <MESH_NWK_LOOK_FOR_NETWORK>need_scan:0x2, need_scan_router:0x0, look_for_nwk_count:0
~~~
3、10号和8号SR8201在RST的时候是IO0是3.3V，剩下两块在RST的时候IO0是0V\
4、想办法在8201时钟线路上拉高对于IO0管脚的电平\

## <span id = "73">0209</span>
1、室内+10dB衰减 RSSI -33 \
2、不加衰减 连不上（天线问题）
加3dB衰减 连上无法通讯（天线问题）
50m -52 ping 2000ms 
加10dB衰减（10dB衰减器和天线连接紧密）
1.8m -33 能自己连上 ping正常
34m -55 能自己连上
50m -64 能自己连上
两端3dB 
50m -50
25m -47 -59跳变
3、36M，6dB -51 —— -57 jperf 0.25M

## <span id = "73">0210</span>
1、
室外+6dB衰减
50 -50
100 -61
140 -71 -65 -66
去掉6dB衰减
230 没有6dB衰减 -62 
2、基本都能够自己连上，但是还有reason201的报错，也有b0 auth没通过，还有一堆WRNRX的报错

## <span id = "74">0213</span>
1、ZH乔工说这个项目，这个月要调试完\
2、测试5、6、10的速度\
3、
5R10N 几乎连不上，连上ping也很差，200ms+，室内2mRSSI-73\
5R6N 连得上，室内RSSI-68，ping在14ms左右，jperf速度:0.2M\
10R 6N  连得上 RSSI -78 ping 9ms jperf：几乎发不出去\

## <span id = "75">0214</span>
1、3R7N 36m 10dB rssi -45-52 ping 5ms jperf 3.7M  \
2、3R7N 50m 10dB rssi -56-64\
3、3R7N 36m 20dB -51 ping 15ms jperf 0.1M\
4、3R7N 36m 15dB -47-50 ping 15ms jperf 一开始1.8M 后来0.3M\
5、3R7N 36m 12dB -54 ping 15ms jperf 3.7M\

## <span id = "76">0215</span>
1、LO改到3.9G，天线1.43G，mesh频道改到12\
2、3R7N 36m 12dB -59 ping 100+ms jperf 0.13M (两个准天线)\
3、不准天线更好\
4、两块新老之间，新的效果 L> B > 8，老的3R7N目前都没问题（3M），8号目前连不上。新的LRBN，可以连上（2M），L和老的能到3，B和老的只能到2M。
5、天线S11反射指标 \

## <span id = "77">0217</span>
1、从机第一次遇到:Haven't to connect to a suitable AP now!\
2、主机遇到：[22:27:17:197]Rx←?Guru Meditation Error: Core  0 panic'ed (LoadStoreError). Exception was unhandled.
3、第二条是因为数组越界了，变量没有考虑清楚。

## <span id = "78">0218</span>
1、Qdebug()比printf快 \

## <span id = "79">0219</span>
1、SPI 加入没大问题，双边透传有丢包现象，单边SPI有num重复的现象，关掉一遍的SPI，另一边还是会持续接收一段时间然后才是0，看起来就是在队列里 \
2、有双边SPI的时候好像连不上ROOT（第二次又连上了）\
3、有双向SPI的时候，jperf只能到1.2M（单边SPI都有点断掉），把R-SPI断电，恢复到1.6M，还是低很多。关闭SPItask，将HB调低到1024以后,恢复到3M\
4、有时候从机连不上啊，咋回事（3号有一根线断了），新的天线也差不多3M\

## <span id = "80">0220</span>
1、改一下MWifi——Root里面的衰减，都到-90 \

## <span id = "81">0221</span>
1、王老师救命来了，先测了一下发送和接收 \
2、然后在看杂散、发送增益和发送功率的区别？功率增益和小信号增益导致老板子发射功率比新板子还高\
3、重新考虑整体混频，把 

## <span id = "82">0222</span>
1、救不动命了，现在需要板子上：提高混频前的增益、减少底部噪声、更换2.4G滤波器、更换电源电路、尝试整体混频（避免攻放增加了但是灵敏度减少了）；天线：加锡箔材料、加外壳地平面\
2、3-7 ：eth_example: WiFi send packet failed: 12309\
   L-7：eth_example: WiFi send packet failed: 12309，有2.4G的连上过\
   L-B:不行\
   3-B：不行
结论：在板子同样的情况下，mesh比WiFi更能连上，不知道为什么，可能是mesh又封了一层包？现在还是着力解决，mesh的部分。\
3、现在只有3RLN能互相到1M，然后提高灵敏度——这个后来也不行了，真是nnd复现性\
4、

## <span id = "83">0223</span>
1、I2C 用GPIO模拟的时候 SLK要给够——官网上好像也没有给够啊\
2、完全模拟了GPIO RFFC，不知道怎么不行 \

## <span id = "84">0224</span>
1、新板子速度也还可以
2、2071这个片子没有直接的输出，需要看泄露

## <span id = "85">0227</span>
1、Jperf 2+2=4M 暂时没问题\
2、iperf 16M看看有没有问题 / 直连看看有没有问题\

## <span id = "86">0228</span>
1、又做了一块新板子，8号，看一下性能\
2、为什么iperf和rssi这么相关\
看一下是不是收发函数影响到了，用2.4G看看是不是能收上来\
如果上面没有的话，那就是无线和无线之间功率不太一致。
还可以试一下直连的iperf


## <span id = "87">230301</span>
1、2.4G wifi 直连 iperf速度都只有10M了，完蛋了，看起来只有ESP——电脑才有更高的速率:确实，电脑上能到25M
——晚上用IPEX转SMA看一下两块ESP之间的速率：还没试，等上海回来试一下\
2、

## <span id = "88">230303</span>
1、2071，lodiv github错了，ENBL在最开始不用写但是接到了GPIO output上!\
2、datasheet 2.3 有勘误\

## <span id = "89">230304</span>
1、270M 单向2M，双向各1M可以。
2、室外1对2没有测，室内1对2的时候，两个从放在一起，只有对半1M。
3、不是满负荷，避让时间不会那么多。
4、2.4G直连iperf 一对多
5、2.4G直连Jperf 1对1,1对多
6、2.4Gmesh iperf 1对1，1对多

## <span id = "90">230306</span>
1、270M 单向2M，双向各1M可以。