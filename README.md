# ͨ��
- �ϴ���github�ֿ�
~~~
git add -u
git status
git commit -m "0831"
git push origin ZHONGHANGmdf
~~~


# Debug��־


֮ǰ����[��ȸ](https://www.yuque.com/u834735/nrtf4r/egagyw)

## <span id = "1">0906</span>

1��Qt�ڵ������Ƿ���ʵ�ʵĽڵ������� \
2���㲥�͵�Ե㷢�͵ĵ�ַ���Ķ����ã�ͨ����λ���ڴ��������ã�������ETH��ֱ�Ӷ���ip��\
3����� Reboot ���ӵȲ���\
4������SPI \
5������mesh������� \
6��Qt�и�����һֱ����UDP

## <span id = "2">0913</span>

1��Qt�ڵ������Ƿ���ʵ�ʵĽڵ������� \
2���㲥�͵�Ե㷢�͵ĵ�ַ���Ķ����ã�ͨ����λ���ڴ��������ã�������ETH��ֱ�Ӷ���ip�� ���� ��·��ѡ��\
3����� Reboot ���ӵȲ���\
4������SPI \
5������mesh������� \ 
6��Qt�и�����һֱ����UDP \
7��~~Qt ѡ������~~
8������1.4G 4351
9��׫д����

## <span id = "3">0915</span>

```
 [15:27:30:577]Rx��?I (9081) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (9091) wifi:state: init -> auth (b0)
I (9101) wifi:state: auth -> assoc (0)

[15:27:30:643]Rx��?I (9151) wifi:state: assoc -> run (10)

[15:27:34:616]Rx��?I (13121) wifi:state: run -> init (fc0)
I (13121) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (13131) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (13131) ADF4351: retry to connect to the AP[0m
[0;32mI (13131) 
[15:27:34:672]Rx��?ADF4351: connect to the AP fail[0m

[15:27:36:673]Rx��?[0;32mI (15181) ADF4351: retry to connect to the AP[0m
[0;32mI (15181) ADF4351: connect to the AP fail[0m

[15:27:36:724]Rx��?I (15231) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (15231) wifi:state: init -> auth (b0)
I (15241) wifi:state: auth -> assoc (0)

[15:27:36:768]Rx��?I (15271) wifi:state: assoc -> run (10)

[15:27:40:757]Rx��?I (19261) wifi:state: run -> init (fc0)
I (19261) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (19271) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (19271) ADF4351: connect to the AP fail[0m
[0;32mI (19271) ADF4
[15:27:40:809]Rx��?351: Failed to connect to SSID:fpoap, password:jjjkkk05[0m

```

1���Ļ�·�˲��� \
2�������Ĵ����ֽ׶Ρ�����һ���Ĵ����ֽ׶εİ���\
3�����ڸ��ŵ����໷

## <span id = "4">0916</span>
1���������� ���� ȷʵ���������˵��¶��������أ����������ʻ��У����Ƕ���\
2��¥���� ���� ʧ���ˣ��������ݼ�PPT


## <span id = "5">0917</span>
1���������ӡ���������õ�����£�����ͨ���� \
- ��WiFi���������С��10dB��15dB:
2��¥���ࡪ������ˣ����ߵ������ƥ������

## <span id = "6">0919</span>
1��������֤�׶β��١��� �����Դ���iperf���ԣ���2.4G���бȽ�\
2�����������ߣ��ĵ�20dB \
3��saving new calibration data because of checksum failure, mode(0) ��\
4����?I (16388) wifi:bcn_timout,ap_probe_send_start \
5��û�н���DHCP�ķַ�

## <span id = "7">0920</span>
1�����������Ժ󣬽����˾���Ĳ���\

## <span id = "8">0921</span>
1��������ݶ��ˣ�����һ��ͼ�Ļ��� ����linux������red

## <span id = "9">0922</span>
1�����书�ʵ� 15 �������ϲ⣬�ε����������ϲ��� \
2�����˥������ȥ�����б���������Ӱ�죬Ҫͨ��1�ų����Ӱ�� \
3����Ҫ��һ�±�����1.4G�Ļ�����Щ

## <span id = "10">0923</span>
1�����书�ʵ� 15 �������ϲ⣬�ε����������ϲ��� \
2�����˥������ȥ�����б���������Ӱ�죬Ҫͨ��1�ų����Ӱ�� \
3����Ҫ��һ�±�����1.4G�Ļ�����Щ

## <span id = "11">0924</span>
1�����书�ʵ� 15 �������ϲ⣬�ε����������ϲ��� \
2�����˥������ȥ�����б���������Ӱ�죬Ҫͨ��1�ų����Ӱ�� \
3����Ҫ��һ�±�����1.4G�Ļ�����Щ ���� �Ѿ����,��PPT��������\
4��������ETH͸�����֣������á�������Ҫȷ��mesh�Ƿ����������֤�������ʹ�� \

## <span id = "12">0927</span>
1����̫��init mac time out ���� �ǲ���GPIO0ʱ�ӵ����� \
2����̫��͸����ʧ���ˣ����ˡ��� ����0505�İ汾��Σ��ȱ�֤2.4G����û���⣬Ȼ������ֲ��1.4G�����ڵ�1.4G STA ETH��2.4G��Ҳ�ܲ�п�����GPIO0�����⣩\

## <span id = "13">0928</span>
1����ʼѰ��20M�Ĵ��� �� �ҵ��ˣ�����eth�������\
2������1.4G�ĸı� \
3���ε����ߺܺã��������ߺܲ� ���� ������̫��оƬ������ \

## <span id = "14">0929</span>
1���Լ�����2.4G�Ȳ�һ��mesh\
2��mesh root�������������child node connect��disconnet����û�м�break��ɵ����\
3��mesh�о���û�жϹ��������ʺܺã�����û����Զ��˥��ͬ��˥������ȥ��\
4�����Կ��ǲ�һ��mesh��eth ping�������ô��\
5����ETH kit��ֻ��ʱ�� \
6��˥������������ETH PINGһ�� \
7��WiFI���Ʒ�ʽ�Ƿ����λ���� \
8���������λ������ָ���Ե� \
9�����˻�������ս��\
10��37.5�� һ��ʼ40 ��18.75����7dbȻ��˥����˥����-69�����ˡ�
�ڶ��β���ʲô������89��
��������������

## <span id = "15">0930</span>
1��������λ�ĺ���úܽ�����rssi�ܵͣ�Ҫ�������Ķ�����\
2��������ĺ�PING��û�н�ͼ�ϵĲҵ���\
3����������ʱ ���� ping������ʱ��

## <span id = "16">1002</span>
1������RSSI��֪��Ϊʲô2�׽�����-60������Ҫ��PING����Ϊ���Ի�׼����mesh����ETH͸�����ٱȽ�ֱ��ETH͸��������1.4Gmesh��root��0929 \

## <span id = "17">1006</span>
1��1.4G mesh��root��0929 ���Multi Addr���Լ�Ҳ����һ������    \
2��esp.emac: no mem for receive buffer \
3���ҵ�ԭ���ˣ������Լ���һ�ݣ�Ҫ��������Root������һ���鲥��ȡ�������ڴ����ܼ�����ʱ��\
4��TODO mesh ETH͸�����١���˥��

## <span id = "18">1007</span>
1��1.4G��mesh���������ʺܸߡ�1.4G��ֱ�� ����ping��ͨ\
2������������ֱ��ʵ�ʵ�ͨ��ʵ�鿴�£�һ�Ǵ���gps����ͨ�ţ� ����spi����ͨ�š��Ҵ�̨�ʼǱ�����\

## <span id = "18">1008</span>
1��QT ����user�ĳ�Ա���Ȱ�QTUDP�����moc_user.cppɾ����clean-qmake-build���������������LNK2019�����⣬�Ǿ�[������](https://blog.csdn.net/shinefriend/article/details/109405042?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-109405042-blog-108044482.pc_relevant_3mothn_strategy_and_data_recovery&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-109405042-blog-108044482.pc_relevant_3mothn_strategy_and_data_recovery&utm_relevant_index=1) �����ڿ�����СWIN�Ͽ�����\
2��GPSģ��Ĳ�������9600�����ݸ�ʽ����  \
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
3�������һ�´���͸���� \

## <span id = "17">1009</span>
1���ȹ���,���ˣ����յ� \
2��2��1��:�󲿷�-11��Ȼ���м��α��-59��1��2��:�Ͷ���61\
3��2�Ȳ��ã���1��3�գ�Ҳ����-68����������1�ķ�����·�������⡣1��3����-33��ȷʵ��1�ķ�����·��������\
4��2��3�� -83?; 2��3��: -67 \
5��׫д����

## <span id = "18">1010</span>
1�����ɳ�����Ȼ�����\
2��ARP����ռ��esp��Դ����esp��̫��ͨ�Ż��ɹ���ARP�������߼�һ��eth����\
3����̫��������Щ�������⣬����ֱ��ͨ�����ݲ������ʺͶ�������= дһ��1.4G��iperf�Ĳ���\
4��ȷ��һ��mesh�ܲ�����ETHջ��

## <span id = "19">1011</span>
1��1000��¥�ڲ���18��25��50m+˥��û���⣬���Ǵ��ڴ�ӡ��Щ������֪��Ϊʲô\
2��2.4G �����Ű����ı仯���������������ı仯��Mutilconsole 2.4Gû���⣬����1.4G ������1460��1000������100�İ�������0.2M���ң�����䣬��1460��1000�������µ�bug���ҵ�����bug��ԭ���ǹؼ���
~~~
mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:14, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:25
~~~
3��һֱ���ǲ��еģ��²�����ײ��н�����Ҫô��ֱ����Ҫô���Ż�������\
4����������2.4G��˫����ֱ������ \

## <span id = "20">1012</span>
1��1000������¥���    \
2��mesh ȷ�ϻ����Լ��Ƿ����õ�������ȷ�ϵ�ʱ���ܷ�һЩ������mwifi write���true false�� \
3�����100ms�Ĵ����ʲô������16�Σ�������һ�������͵� \
4��ֱ����û��ack��ֱ���Ƿ��н���\

## <span id = "21">1013</span>
1��
�����ɼ���GPS 
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
2������\
title:�������������)����������������Ԫϵͳ�о� \
�����о�����:(1)���һ�����ڵ���ڵ�����ؽڵ�ͨѶ��Ӳ��ϵͳ�����Ӧ������������ܹ����SPI��UART��ETH��·͸����1.4G WiFi 1��16�ӵ�������֧����λ���ϱ���Ϣ���·����á�\
(2)�����о�����(1)��ͨѶϵͳ�������й����ϵ�Ԫ���������������ͬʱ����ͨѶָ����Ż���\
(3)�����о�����(1)(2)�е�ͨѶϵͳ�����µ�������ʽ������Mesh�������縺�غ���չ�ԣ�ͬʱ����-�ڵ�ETH͸�����Լ��ڵ��͸���Ͻ���·���㷨�Ż��͹��ܵ��ţ�\
3�������л��ٶ�50ns���첨��6ns
4��2.4GҲ��һ��WiFi�Ŵ�������һ���շ��л� \
5����4��5�����Ҳ��һ�� \
6��GPS����ת�����ٶ�����ת�� \
7��ack����ȥ��/�ӳ� \
8��1.4G��ʱ��������·����0dB

## <span id = "22">1017</span>
1��ԭ���и��ȷ��������⣬[Here](http://old.openluat.com/GPS-Offset.html)�зǳ���ϸ��ת���ʹ���\

## <span id = "23">1018</span>
1��1.4G ������ȸ����ˣ����ǲ��� \
2����һ��2.4G�����շ��л��Ժ��Ч�� \
3��mesh ȷ�ϻ����Լ��Ƿ����õ�������ȷ�ϵ�ʱ���ܷ�һЩ������mwifi write���true false�� \
4�����100ms�Ĵ����ʲô������16�Σ�������һ�������͵� \
5��WiFi�߷���ȷ�ϡ����Ե���ȷ�ϻ�������ȷ��\
6��ͬ���İ���2.4G���Ϳ�(60us)��1.4G������(200us) \
7�������շ�����\

## <span id = "24">1019</span>
1��׼���Ŵ���·���շ����������ֱ��Ҳ���У�Ҫ����mesh��\
2�������У��Ƶ������������ǵ���Ź�������⣨�ڸ��շ������ʱ���õģ�\


## <span id = "25">1020</span>
1����ߵ��Ƿ���ͨ·������������ߵ����˲���������ȥ�������շ�������������������\
2��Ŀǰ��2.4G�Ŵ����ṹһ�������շ�ͨ·�Ϸֱ��Ƶ�����������˲�������׼�������ܵ�һ����Ƶ����Ҫȥ��˾�һ�Ƶ��\

## <span id = "26">1024</span>
1������2.4G������ָ���Ų� \
2�����������ping��ʱ���ӡʱ�����MACOS�� WIN��PING�ֱ���0.2 MAC0.1
~~~
ping 192.168.4.88 -i 0.2 | while read pong; do echo "$(date): $pong"; done
~~~
3��SPI��˳��:15 13 12 14(ETH������)\
4��SPI͸����0707�receiver��������\
5�����Ҫ��1M��1000����ô����8000000�����1M��1024���Ǿ���8388608\
6��2048����ջ�£�SPItask�ﶨ��sendbuf168���ԣ�169���ꡣ�ĳ�4096���ԡ���2048�£��Ѷ���ŵ���������Ҳ����\
7��һ�η���1000Byte��delay10us��9000 000Hz�������ܹ��ﵽ��С��9Mbit/s��ʵ��: 8.28Mbit/s    \

## <span id = "27">1025</span>
1���µ�ESP32оƬ��
~~~
rst:0x10 (RTCWDT_RTC_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)

[12:04:3:724]Rx��?invalid header: 0xffffffff

[12:04:3:769]Rx��?invalid header: 0xffffffff   
~~~
2��Packet content transfer stopped (received 8 bytes) \
3�� 4��5�Ű�1.4Gmesh 4351 1000������20.09��\
4�� ֱ�ӿ��Ʒ��ͽ��յ�·�����ţ���դѹ����̫�ͣ�\
5�� 9177����2.4G��ͨ�������źźܵͣ�ֻ��84; 800M ��ͨ��N��CS0�Եض�·\
6��
~~~
Parent is disconnected, reason: 201[
~~~
[�������csdn](https://blog.csdn.net/Marchtwentytwo/article/details/118551075)  \   
[�������espressif](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/wifi.html#wi-fi-reason-code)\

7������һֱ�������ӣ�4-5����������ѡ��Ŀ��أ�3-5����������\
8����4-5����bug
~~~
[16:46:2:199]Rx��?W (24395) mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:9, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:0
~~~

## <span id = "28">1026</span>
1��
~~~
����1.4g�շ������ȶ���22s��1000��������������ˣ��������̫�����ϣ����pingû����Ͳ���ˣ���β�������Ҫ�����ǿ������������������Ծ�����̫��ok��spi ok������ok�ͺ��ˡ������ص����̫��
~~~
2��5�ţ���̫�����С� ���С̣�ֻ�����γɹ���������  4�� ����̫�����С� ���д���
~~~
ROOT��res = mwifi_root_write(Multiaddr, 1, &data_type, msg.packet, msg.length, true);
ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
NONROOT��
ret = mwifi_read(src_addr, &data_type, &buffer, &buffer_len, 100 / portTICK_RATE_MS);
res = mwifi_write(Rootaddr, &data_type, msg.packet, msg.length, true);
~~~
3��ETHkit ��0825������PING���Ῠ�٣�����0929�ͻῨ�٣��������ˣ�����0825�����鲥�Լ���0929���鲥�Լ���\
4������3�У���0825ֱ�Ӽ���1.4G��

## <span id = "29">1026</span>
1��1.4G ��2.4G ���ӻ�����¼��ʱ��Ҫ��GPIO0�ľ����IP101������������\
~~~
W (30970) mesh: [mesh_schedule.c,3130] [WND-RX]max_wnd:4, 1200 ms timeout, seqno:0, xseqno:1, no_wnd_count:0, timeout_count:21
~~~

2����16-26%�Ķ����ʣ�20ms���ң�����ǰ���кܶ�max_wnd��send flow control message failed or timeout���²��MAC��ַ�����йأ���3��TODO��һ���Ǹ�MAC��ַ���ã�һ���ǿ�����ģʽ�ľ���ԭ��һ����ѧϰDavid�ģ�����һ��Ȼ�����ϴ�һ�ݣ������¿�һ�������ļ����ԣ�\
3��4��5�Ż��ֱ�������EThkit��ɣ�Ȼ����4�Ż�����̫���е����⡣\

## <span id = "30">1027</span>
1��5�Ż���ΪNonroot�����ʱ��Root�ᱨ�����     \
~~~
[10:22:47:860]Rx��?I (158688) mesh: [TXQ]<max:64>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:12), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (158689) mesh: [RXQ]<max:64 = cfg:64 + extra:0>self:0, <max:64 = cfg:64 + extr
~~~

## <span id = "31">1029</span>
1��5��û���⣬��ʱ��ping��ͨ��pingͨ0%
9177R��1.4G�����⣬��̫��Ҫ��WiFi�ٲ���̫�����У�1.6%
4���Ȳ���̫��17%�����������̫��0%����
3���Ȳ���̫����ͨ�������̫��0%����
6������ס����̫��û���⣬2.4G 0%
7������ס����̫��û���⣬2.4G 0%
8������ס����̫���Բ2.4G 4%
9������ס����̫��û���⣬2.4G 0%

## <span id = "32">1031</span>
1��1.4G consoleҪ��һ�£��ĳ�no router������Ϊû��1.4G��router\
~~~
5��û���⣬��ʱ��ping��ͨ��pingͨ0%
9177R��1.4G�����⣬��̫��Ҫ��WiFi�ٲ���̫�����У�1.6%
4���Ȳ���̫��17%�����������̫��0%����
3���Ȳ���̫����ͨ�������̫��0%����
6������ס����̫��û���⣬2.4G 0%
7������ס����̫��û���⣬2.4G 0%
8������ס����̫���Բ2.4G 4%
9������ס����̫��û���⣬2.4G 0%
~~~
2�� 79��45 ��ࡢ68


## <span id = "33">1101</span>
1��esp.emac: no mem for receive buffer\
2��4��5�Ż�����```10261.4Geth2mesh(root)```���``console test``����jperfҪ���öࡪ���²���idf.py���õ�����\
3��Mwifi-Transmission-128(64)��240MHz(160)����tm�ģ������Ժ�ͨ��\

## <span id = "34">1102</span>
1��4��������\
2��ETHkit��������7�ţ�128��240���У���̫����ͨ˳���ĳ�128��160���Ϳ����ˣ���

## <span id = "35">1103</span>
1��SR8201 21����û��������12�����������ˣ�\
2�� init phy failed��wrong chip ID��RST��û����ȷ������
~~~
[0;31mE (806) rtl8201: rtl8201_init(352): wrong chip ID[0m
[0;31mE (806) esp_eth: esp_eth_driver_install(223): init phy failed[0m
~~~
3��
~~~
[0;31mE (922) rtl8201: rtl8201_pwrctl(261): power up timeout[0m
[0;31mE (928) rtl8201: rtl8201_init(343): power control failed[0m
[0;31mE (934) esp_eth: esp_eth_driver_install(223): init phy failed[0m

[0;31mE (874) rtl8201: rtl8201_reset(167): reset timeout[0m
[0;31mE (874) rtl8201: rtl8201_init(346): reset failed[0m
~~~

## <span id = "36">1103</span>
1��21�����ŵ�2.49KҲ�ܹؼ�\
2����̫�����в��С�


## <span id = "37">1106</span>
1���û��Ķ����ˣ�׼��������ͭ\
2����̫��ͨѶǰ����1010ͬ���룬��1ͬ����1����1����1ͬ����0����0�����Tx�����߷��˵Ļ���\
3��8�� 7��

## <span id = "38">1110</span>
1����һ�����ģ��ҵ��ˣ�����\
2��������5G��վ����\
3������ı�Ƶ�ǿ��Եģ�Ȼ����Ҫ��һ��
N-R 1.13
R-N 1.56

## <span id = "39">1113</span>
1����ʵ���Ҳ���1.4G�����ǲ�����߱�Ƶ��1.4g��Ȼ�������ǰ��ϵ�1.4g�շ��л��ӷŴ���Ρ����ϵı�Ƶ�õ�\
2��AP����2.4G�����շ��л�����7M
AP����1.4G�����Ƶ����1M
AP����2.4G�����շ��л�����1.4G�����Ƶ����������
AP����1.4G�����Ƶ���������л���·����(1-1.5M����)

6:�����л��Ʋ���
8:0dB�л�������\
3����Ƶԭ��

## <span id = "40">1114</span>
1����Ϊ���ϵ��download����EN����һ��100nF��RC��ʱ��·(1.1uF��0.2uF)��IO0(������������20K���½ӵ���0.1uF)��3.3V�ϵ��һ��\

## <span id = "41">1116</span>
1����ʼ��û���⣬���������в�ͨ\
2����֪��Ϊʲô��������һ�κ��ˣ��������������ʺܸߣ����ж������յ�\

## <span id = "42">1117</span>
1��PC�˸ĳ�10M�ǿ����õģ��ĳ�100M�Ͳ����ˣ���Э��Ҳ���С�
## <span id = "43">1118</span>
1���ֲ����ˣ�ֻ�����У�û�������ˣ���֪������ʲô(����ʦ˵����ʱ�ӣ����ǸĻ�ȥ��Ҳ���У���)\

## <span id = "44">1119</span>
1��TXC�ϵĵ����С��Ӱ�����뵽PHY��50M����ķ��ȣ��Ӷ�Ӱ��10Mͨ�š�100Mͨ�ţ�����Ӧ\
2��������˵����22+5�� 10M����ͨ�ţ�1k����Ӧ����ͨ�ţ�122ŷķ���У�1.12k���У�480���ԣ�360���У�720\
3��оƬ�ڲ���˵��������������M���ģ����˵����������100k���ң��Ե�M����������

## <span id = "45">1124</span>
1�������к����ӵ�ETH��UART��SPI\
2���Ƿ������л�������⣬��eee���������:
~~~
[21:40:57:695]Rx��?55 41 52 54 3A 0D 0A CE D2 CA C7 D6 F7 BB FA 0D 00
[21:41:1:420]Rx��?55 41 52 54 3A 0D 0A CE D2 CA C7 D6 F7 BB 00
[21:41:16:281]Rx��?55 41 52 54 3A 0D 0A CE D2 CA C7 BB FA BB 00
[21:41:28:990]Rx��?55 41 52 54 3A 0D 0A CE D2 CA C7 BB FA BB FA 0D 00
~~~
3�����а�ǰ���ͷ�ĳ�8���Կ���eee�Ļ����Ͳ�֪��Ϊʲô��:
~~~
assert failed: block_locate_free heap_tlsf.c:447 (block_size(block) >= size)


Backtrace:0x4008189e:0x3ffbe0f00x40088b45:0x3ffbe110 0x4008f955:0x3ffbe130 0x4008d6c2:0x3ffbe250 0x4008e199:0x3ffbe270 0x40081a61:0x3ffbe290 0x400d8aaf:0x3ffbe2b0 0x400d95d1:0x3ffbe360 0x400d6a6
[21:57:41:538]Rx��?9:0x3ffbe3f0 0x4008c061:0x3ffbe420 


ELF file SHA256: 8358e5c9f441bed9
~~~

## <span id = "45">1125</span>
1�������������ַ��������һλǿ�б����00�������ڽ��յ�ʱ�������һλ���Ͳ��ø�7����ͷ��8���ˡ�\
2��eee�������ڴ��������⣬[����](https://blog.csdn.net/gzxb1995/article/details/124504705)\
3���ն˶�ȡSPI����ʱ��һ����Ϊslave����ôZH�İ�����IO34��IO2��Ҫ������λ��\
4��ͬʱ��������IOMUX�������ٶȴﲻ��8M\
5��2��������1��3��������3��ETH�����⣩\

## <span id = "46">1129</span>
1��������[����������ѧϰ](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)\


## <span id = "47">1130</span>
1����������[����������ѧϰ](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)\
2��������û����������ֻ����6.1DNS����7�¡���8�¶�û��

## <span id = "48">1201</span>
1��8201��TXC��O/PD�������������ġ�����������������\
2��2.4G��PINGͨ������������룺
~~~
eth2mesh_0805
eth2meshroot_0811
~~~
3��ZH���Ǵ�ZH�����Ĵ���\
4����������[����������ѧϰ](https://www.yuque.com/u834735/nrtf4r/nu54m8gr9cwexzhb)���ӵ�9�¿�ʼ��\

## <span id = "49">1202</span>
1��IO0����������ĳ�1K��ǿ������10K-1.5K�����У�ֻ��1K���ϵ���������Ӱ����̫��\
2��IO0����1K�Ļ����ϣ��Ĵ������衣2Kֱ�Ӳ�ͨ��1Kû�⣬910�� 20%�����ʡ�

## <span id = "50">1205</span>
1��6��7�Ż��������ϵ�������10K����������1K��WIN 7%��MAC 4.6%\
2��10K����������510ŷķ��WIN 6% ��MAC 6%\
3���������಻��ֻ��ͨ�Ͳ�ͨ�����⣬׼����8201���б��ص�PING��PHYRST20K��680nF\
~~~
10261.4Geth2mesh
10261.4Geth2meshroot
~~~
4��6�ź�8�ţ�ʣ�µĺ�7�š�

## <span id = "51">1206</span>
1���������ٲ��±�Ƶ���������ϰ��ӷ��������٣������Ƶû�����⣬�����¾�Ӧ��ͨ�ġ��ϰ���֮������ûɶ�����ǰ�\
2���ڵ�֮���ʱ��ͬ����ͨ��802.11Э���е�TSF���С�������õ��������ŵ�ģ�͡���ʱ�ɼ������ݴ���PSRAM��    \

## <span id = "52">1207</span>
1���ѷ��书�ʱ��10dB��ûʲô�����°��ӿ��ʱ��22-29ms������ʱ��45ms\
2��2.4GWiFi�շ��л���һֱʹ�ܡ��л�ʹ�ܡ�PAһֱʹ�ܡ�LNAһֱʹ�ܶ���Ӱ�죬����Ƿֱ�ʹ��\

## <span id = "53">1208</span>
1�����\
2������ʱ���ڽ�����������\
3���ó��� ��ʦ��ѧ����
ɳĮ  ��ʦ

## <span id = "54">1209</span>
1��Ҫ��wireless hart\

## <span id = "55">1213</span>
1���Ѿ������飬Ȼ����дһ���ĵ�\
2��1213 1.4GMESH 1000Byte 10ms 10 dB �°���A-��8 31.6ms RSSI -21\

## <span id = "56">1214</span>
1���°���R������ͨ��RSSI -67\
2��1214 1.4GMESH 1000Byte 10ms 10 dB �°���A-��8 18ms \
3��20dB���������ϣ�15dB�����ϡ�Ȼ��Ŀ����л���0dB��Ȼ�����ٶȱ����ˣ��ٵ���10dB\
4������dB���л�dB�Ĳ�ֵ�������ٶȡ�20-12-10K��������15-12 18ms��15-0-5.1K������10-0Ҳ����\

## <span id = "57">1215</span>
1��һ����·�Ͽ��صĸ���Ȳһ�����������Ϲ�ȥ�������Ƚ϶࣬��һ���ԳƵ�����������ص�����һ��һ�������յ��裬һ������ϵ���\

## <span id = "58">1216</span>
1��׼�������ȸ������͹�ȥ\
2�����ǵĿ���50ns��2.4G�Ŀ���10ns\

## <span id = "58">1216</span>
1��׼�������ȸ������͹�ȥ\
2�����ǵĿ���50ns��2.4G�Ŀ���10ns\

## <span id = "58">1217</span>
1��20dB-10K��̫������ͨ�ģ�����Ҫ��һ�ᣨ12161.4GZH��\
2���ӻ�SPI task�����⡪�����߶������ SPI Received��һ�ϵ���У� ����������UART��ʱ���Լ����յ����Լ��Ĵӻ���ʱ��ESP�Ĵӻ�������������⣩����Ӧ����\
3��֮ǰ10K��10dB�л���5K��0dB�л�������10K��1dB�л���\
4�����¿�ʼ��¼:
���� A 10K-0dB��\
���� B 12K-2dB��10K- -3-1dB���ҡ�20K- 4-5dB��33K- 7-12dB�������������ˣ���27K - 5-10dB��24K - 4-9dB��15K - \


## <span id = "59">1219</span>
1��׼��д��λ�� �� GPS\

## <span id = "60">1225</span>
1��
��7�ŷ�����B�գ����8-10ms
��A������6���գ����20ms 
��7�ŷ�����6���գ����20ms
��A������B�գ���������������\

## <span id = "61">1226</span>
1�������1280��64160�Ժ�ͻ���ص�20ms\
2��ʹ��iperf����
2.54MBytes 9s
~~~
[13:38:42:363]Rx��?[0;32mI (39986) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  0.94 MBytes  2.51 Mbits/sec[0m

[13:38:45:587]Rx��?[0;32mI (43210) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  0.69 MBytes  1.85 Mbits/sec[0m

[13:38:48:598]Rx��?[0;32mI (46220) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  0.91 MBytes  2.42 Mbits/sec[0m

[13:38:51:607]Rx��?[0;32mI (49230) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.18 MBytes  0.49 Mbits/sec[0m
~~~
4.42MBytes 12s
~~~
[13:53:23:548]Rx��?[0;32mI (921150) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  1.05 MBytes  2.81 Mbits/sec[0m

[13:53:26:550]Rx��?[0;32mI (924152) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  1.19 MBytes  3.18 Mbits/sec[0m

[13:53:29:551]Rx��?[0;32mI (927153) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  1.19 MBytes  3.16 Mbits/sec[0m

[13:53:32:616]Rx��?[0;32mI (930216) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.99 MBytes  2.63 Mbits/sec[0m

[13:53:35:628]Rx��?[0;32mI (933230) [mwifi_test, 622]: [24:4c:ab:13:52:7c]  12-15 sec  0.16 MBytes  0.44 Mbits/sec[0m
~~~

6�ŷ��ı��ݣ���7�Ų����Ը����ˡ�
~~~
[16:43:49:760]Rx��?[0;32mI (24663) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   0- 3 sec  1.00 MBytes  2.68 Mbits/sec[0m

[16:43:52:762]Rx��?[0;32mI (27665) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   3- 6 sec  0.66 MBytes  1.75 Mbits/sec[0m

[16:43:55:763]Rx��?[0;32mI (30666) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   6- 9 sec  1.22 MBytes  3.27 Mbits/sec[0m

[16:43:58:772]Rx��?[0;32mI (33674) [mwifi_test, 622]: [24:4c:ab:13:52:7c]   9-12 sec  0.38 MBytes  1.01 Mbits/sec[0m
~~~

## <span id = "62">1227</span>
1�������ٷ�iperf�ڲ���������Щ��Ҫ�ĵ�:
240MHz
Mwifi_Transmission config

R-N jperf 2.7M iperf 5M
N-R jperf 1.7M
2��
���ټ첨�������������޼첨
3��
jperf ˫�� 1.4G�ٶ�������
TODO :
jperf ˫�� 2.4G�ٶȿ�һ��
jperf ���ٸ�ZH��һ�׹�ȥ

4��jperf 1.4G �ٶȹ��߻ᱨ��
~~~

[18:26:20:942]Rx��?[0;33mW (114604) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
[0;31mE (114623) esp.emac: no m

[18:26:20:988]Rx��?em for receive buffer[0m
[0;31mE (114632) esp.emac: no mem for receive buffer[0m
[0;31mE (114634) esp.emac: no mem for receive buffer[0m
[0;31mE (114643) esp.emac: no mem for receive buffer[0m

[18:26:21:047]Rx��?[0;33mW (114705) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m

[18:26:21:141]Rx��?[0;33mW (114805) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m

[18:26:21:241]Rx��?[0;33mW (114905) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
[0;33mW (114905) [mwifi, 707]: <ESP_ERR_MESH_NO_MEMORY> Node failed to send packets, dest_addr: 24:4c:ab:20:e9:74, flag: 0x02, opt->type: 0x08, opt->len: 13, data->tos: 0, data: 0

[18:26:21:292]Rx��?x3ffde148, size: 1448[0m
[0;33mW (114918) [mwifi, 806]: <ESP_ERR_MESH_NO_MEMORY> Root node failed to send packets, dest_mac: 24:4c:ab:20:e9:74[0m
[0;33mW (114929) [mwifi, 1090]: <ESP_ERR_MESH_NO_MEMORY> mwifi_root_write, size: 1448[0m
[0;33mW (114937) [eth2mesh, 385]: <ESP_ERR_MESH_NO_MEMORY> mwifi_read[0m

[18:26:21:343]Rx��?[0;33mW (115005) [mwifi, 697]: <ESP_ERR_MESH_NO_MEMORY> esp_mesh_send[0m
~~~

## <span id = "63">1228</span>
1��
~~~
1216R1.4GZH
12161.4GZH
~~~
2.4G ����
R��N jperf 3M(4M��Ż��) iperf  7.84M
N-R jperf 4M iperf  6.19M

## <span id = "63">1229</span>
1��
1.4G ����

R����N
�ϰ��ӡ���>�°��� 4M
�°���<�����ϰ��� 0.4M

�ϰ���<�����°��� 
�°��ӡ���>�ϰ��� M \
2����̫�����ָĳ�SR8201F��ESP�ṩʱ�ӣ�������ESP���׽�������ģʽ������100M��ʱ��PING�Ƚ��ȶ���R40����22ŷķ��\

## <span id = "64">1230</span>
1�����Թ��̼�ֽ��\
2��������Ϊ������ڵ����ϣ�����ֻ�ܶ԰룬��Ϊ������mesh�� stack��wifi�Ĳ�һ��\

## <span id = "65">1231</span>
1�����ܳ��Եķ���
- �о�һ��windows size
- tryһ�����ڲ�ͬ�ĺ���
- ������ѵĺܶཨ����Գ��� https://github.com/espressif/esp-mdf/issues/301

## <span id = "65">0103</span>
1��ֱ����AP��STA ���8-13M�ǻ���û��0%�����������߼��������Ҳ�ߡ�Ȼ��AP ping STAʱ�Ӹߡ�STA ping AP ʱ�ӵͣ���github�ϵ�issueһ��
2��?�е����⣬flashû����,����ETH2AP�ٷ�����Ҳû����\

## <span id = "66">0104</span>
1��task�ڲ�ͬ��core�ϣ�����Ч�����task�ڲ�ͬ��core��ͬ���ȼ�>��ͬ���ȼ���task��ͬһ��core�Ͽ���ʹ������core����\
2��IRAM �Ѿ���������\
3��iperf ��˫��ûʲô���𣨱�����ֻ��һ��task��

## <span id = "67">0105</span>
1������һ��Tx prior��RX prior��,���������ֺ���û�ã�û�кܶ�ɸ�����\
2��xon_size�����а����ģ�����ο�ֽ��\

## <span id = "68">0110</span>
1������ 1.4G 300�������ϡ�200�������ϡ�150�������ϡ�100�������ˣ�rssi-80��7R3N��7-3 0.02M��3-7 0.9M��15%��������ping 6ms jperfʱ300-2000ms\
2������ 1.4G �����ϣ�rssi-54��7R3N��3-7 1Mû���⣬ping 7ms��jperfʱ10ms \
3������ 7R5N��ͬ����RSSI -33��R-N 1Mû���⣬ping+jperf 10ms��N-R ֻ��0.6M��ping+jperf 200+ms ��send flow fail\
4��ֱ���ߣ�RSSI-27��R-N 1Mû���⣬ping+jperf 10+ms �м���100+ms��N-R��0.3M��0.8M��1M���У������ԱȽϲping+jperf ����ʱ��û��send flow fail������һ��10dB��˥������-27��-70��\
5��8����֤ͨ������6�������ϡ�5�š�10�š�B����һ�λ����ε���1M��Ȼ�������ǲ��ȶ�������5�����ȶ�����ͦ�ȶ��ġ�
6��10�ź�5�����ϰ��ͣ�10��ͬ���ߣ�RSSI-36����ping��8ms��ping+Jperf ���У�1M 25+ms���õ�ʱ��ms�����ӣ����У�0.5M��5��ͬ���ߣ�RSSI-27����ping��7ms��ping+Jperf ���У�1M 20+ms��2�Σ�������ͱ��0.7M��400+ms�����У�0.3M��5�ż�10dB˥����RSSI-38����ping 8ms��ping+Jperf ���У��Դӱ��0.7M�Ժ��һֱû��ȥ���ܽ᣺10�����б�5���ȶ���3���������ȶ�������3�š�10�š�5�Ŷ��е����⡣

## <span id = "69">0111</span>
7R3N
100�� 2.4G rssi -83 1.4 ������
90�� 1.4G -81 0.55M 2.4G -77
60�� 1.4G -79 1M ���ֹ�һ�� 0.9Mʣ�� -2.4G -74
50�� 1.4G -73 1M  2.4G -66

## <span id = "70">0131</span>
1����һ��IO��8201��λ
2����λ����GPS����

## <span id = "71">0207</span>
1����Ҫ100��-200���Ժ�Ȼ���߻����������ӣ�����һ����������
2��GPIO17 ����ΪOutputģʽ�Ժ�Download 0.7V��ʲô����д��������1.9V�������Ժ�3.3V�������Ժ�1.9V��

## <span id = "72">0208</span>
1��Ҫ��һ��ESP32_EN��8201 RST֮��Ķ�����\
2��meshһֱ�����Ժ�\
~~~
I (639600) mesh: <MESH_NWK_LOOK_FOR_NETWORK>need_scan:0x2, need_scan_router:0x0, look_for_nwk_count:0
I (643513) mesh: [FIND][ch:0]AP:7, otherID:0, MAP:0, idle:0, candidate:0, root:0[00:00:00:00:00:00][FIXED-ROOT]
I (643513) mesh: [FIND:158]fail to find a network, channel:0, cfg<channel:13, router:, 00:00:00:00:00:00>

I (643521) mesh: <MESH_NWK_LOOK_FOR_NETWORK>need_scan:0x2, need_scan_router:0x0, look_for_nwk_count:0
~~~
3��10�ź�8��SR8201��RST��ʱ����IO0��3.3V��ʣ��������RST��ʱ��IO0��0V\
4����취��8201ʱ����·�����߶���IO0�ܽŵĵ�ƽ\

## <span id = "73">0209</span>
1������+10dB˥�� RSSI -33 \
2������˥�� �����ϣ��������⣩
��3dB˥�� �����޷�ͨѶ���������⣩
50m -52 ping 2000ms 
��10dB˥����10dB˥�������������ӽ��ܣ�
1.8m -33 ���Լ����� ping����
34m -55 ���Լ�����
50m -64 ���Լ�����
����3dB 
50m -50
25m -47 -59����
3��36M��6dB -51 ���� -57 jperf 0.25M

## <span id = "73">0210</span>
1��
����+6dB˥��
50 -50
100 -61
140 -71 -65 -66
ȥ��6dB˥��
230 û��6dB˥�� -62 
2���������ܹ��Լ����ϣ����ǻ���reason201�ı���Ҳ��b0 authûͨ��������һ��WRNRX�ı���

## <span id = "74">0213</span>
1��ZH�ǹ�˵�����Ŀ�������Ҫ������\
2������5��6��10���ٶ�\
3��
5R10N ���������ϣ�����pingҲ�ܲ200ms+������2mRSSI-73\
5R6N �����ϣ�����RSSI-68��ping��14ms���ң�jperf�ٶ�:0.2M\
10R 6N  ������ RSSI -78 ping 9ms jperf������������ȥ\

## <span id = "75">0214</span>
1��3R7N 36m 10dB rssi -45-52 ping 5ms jperf 3.7M  \
2��3R7N 50m 10dB rssi -56-64\
3��3R7N 36m 20dB -51 ping 15ms jperf 0.1M\
4��3R7N 36m 15dB -47-50 ping 15ms jperf һ��ʼ1.8M ����0.3M\
5��3R7N 36m 12dB -54 ping 15ms jperf 3.7M\

## <span id = "76">0215</span>
1��LO�ĵ�3.9G������1.43G��meshƵ���ĵ�12\
2��3R7N 36m 12dB -59 ping 100+ms jperf 0.13M (����׼����)\
3����׼���߸���\
4����������֮�䣬�µ�Ч�� L> B > 8���ϵ�3R7NĿǰ��û���⣨3M����8��Ŀǰ�����ϡ��µ�LRBN���������ϣ�2M����L���ϵ��ܵ�3��B���ϵ�ֻ�ܵ�2M��
5������S11����ָ�� \

## <span id = "77">0217</span>
1���ӻ���һ������:Haven't to connect to a suitable AP now!\
2������������[22:27:17:197]Rx��?Guru Meditation Error: Core  0 panic'ed (LoadStoreError). Exception was unhandled.
3���ڶ�������Ϊ����Խ���ˣ�����û�п��������

## <span id = "78">0218</span>
1��Qdebug()��printf�� \

## <span id = "79">0219</span>
1��SPI ����û�����⣬˫��͸���ж������󣬵���SPI��num�ظ������󣬹ص�һ���SPI����һ�߻��ǻ��������һ��ʱ��Ȼ�����0�������������ڶ����� \
2����˫��SPI��ʱ�����������ROOT���ڶ����������ˣ�\
3����˫��SPI��ʱ��jperfֻ�ܵ�1.2M������SPI���е�ϵ�������R-SPI�ϵ磬�ָ���1.6M�����ǵͺܶࡣ�ر�SPItask����HB���͵�1024�Ժ�,�ָ���3M\
4����ʱ��ӻ������ϰ���զ���£�3����һ���߶��ˣ����µ�����Ҳ���3M\

## <span id = "80">0220</span>
1����һ��MWifi����Root�����˥��������-90 \

## <span id = "81">0221</span>
1������ʦ�������ˣ��Ȳ���һ�·��ͺͽ��� \
2��Ȼ���ڿ���ɢ����������ͷ��͹��ʵ����𣿹��������С�ź����浼���ϰ��ӷ��书�ʱ��°��ӻ���\
3�����¿��������Ƶ���� 

## <span id = "82">0222</span>
1���Ȳ������ˣ�������Ҫ�����ϣ���߻�Ƶǰ�����桢���ٵײ�����������2.4G�˲�����������Դ��·�����������Ƶ�����⹥�������˵��������ȼ����ˣ������ߣ����������ϡ�����ǵ�ƽ��\
2��3-7 ��eth_example: WiFi send packet failed: 12309\
   L-7��eth_example: WiFi send packet failed: 12309����2.4G�����Ϲ�\
   L-B:����\
   3-B������
���ۣ��ڰ���ͬ��������£�mesh��WiFi�������ϣ���֪��Ϊʲô��������mesh�ַ���һ��������ڻ������������mesh�Ĳ��֡�\
3������ֻ��3RLN�ܻ��ൽ1M��Ȼ����������ȡ����������Ҳ�����ˣ�����nnd������\
4��

## <span id = "83">0223</span>
1��I2C ��GPIOģ���ʱ�� SLKҪ�������������Ϻ���Ҳû�и�����\
2����ȫģ����GPIO RFFC����֪����ô���� \

## <span id = "84">0224</span>
1���°����ٶ�Ҳ������
2��2071���Ƭ��û��ֱ�ӵ��������Ҫ��й¶

## <span id = "85">0227</span>
1��Jperf 2+2=4M ��ʱû����\
2��iperf 16M������û������ / ֱ��������û������\

## <span id = "86">0228</span>
1��������һ���°��ӣ�8�ţ���һ������\
2��Ϊʲôiperf��rssi��ô���\
��һ���ǲ����շ�����Ӱ�쵽�ˣ�1.4G iperf 1�Զࣩ����2.4G�����ǲ�����������\
�������û�еĻ����Ǿ������ߺ�����֮�书�ʲ�̫һ�¡�
��������һ��ֱ����iperf


## <span id = "87">230301</span>
1��2.4G wifi ֱ�� iperf�ٶȶ�ֻ��10M�ˣ��군�ˣ�������ֻ��ESP�������Բ��и��ߵ�����:ȷʵ���������ܵ�25M
����������IPEXתSMA��һ������ESP֮������ʣ���û�ԣ����Ϻ�������һ��\
2��

## <span id = "88">230303</span>
1��2071��lodiv github���ˣ�ENBL���ʼ����д���ǽӵ���GPIO output��!\
2��datasheet 2.3 �п���\

## <span id = "89">230304</span>
1��270M ����2M��˫���1M���ԡ�
2������1��2û�в⣬����1��2��ʱ�������ӷ���һ��ֻ�ж԰�1M��
3�����������ɣ�����ʱ�䲻����ô�ࡣ
4��2.4Gֱ��iperf һ�Զ�
5��2.4Gֱ��Jperf 1��1,1�Զ�
6��2.4Gmesh iperf 1��1��1�Զ�

## <span id = "90">230307</span>
1��2.4Gֱ��iperf 1�Զ� ֻ����MAC��server���Թ��˲���,���ٶȱ�ռ����;
2��1.4Gmesh iperf 1��1 5M; 1�Զ� 4Mû��ȥ;
3��2.4Gֱ��iperf 1��1:\
�ڵ���AP 192.168.4.1 server
MAC  192.168.4.2 30M
3��  192.168.4.3 20M
10�� 192.168.4.4 12M

�ڵ���AP 192.168.4.1 server
MAC  192.168.4.4 30M
3��  192.168.4.3 20M
10�� 192.168.4.2 15.8M

�ڵ���AP 192.168.4.1 server
3��  192.168.4.3 20M
10�� 192.168.4.2 17.95M

3����AP 192.168.4.1 server
�ڵ�  192.168.4.2 20M
10�� 192.168.4.3 12.97M

10����AP 192.168.4.1 server
�ڵ�  192.168.4.2 12.5M
3�� 192.168.4.3 16.34M

5����AP 192.168.4.1 server
�ڵ�  192.168.4.2 15.8M
3�� 192.168.4.3 13.84M

�ڵ�AP 192.168.4.1 server
5�� 192.168.4.2 16.7M
3�� 192.168.4.3 17.54M
/----- ���϶��Ǻڵ����м� -----/

5��AP 192.168.4.1 server
3�� 192.168.4.2 13.21M
�ڵ� 192.168.4.3 9.9M

������˵������... ��ȫ�����

�ڵ�AP 192.168.4.1 server
3�� 192.168.4.2 12.84M
5�� 192.168.4.3 11.68M

## <span id = "91">230309</span>
1��2.4G mesh iperf 1��2 �ܹ��ϵ�14M������Ҳ���Ǻ��ȶ��������Բ���ǿ�����󲿷���10M\
2����λ��������ӻ�������RSSI��ID����ͨ��hjypackup
3��SPI

## <span id = "92">230317</span>
1��ESP�ˣ�����д��flash ����
2��PC�ˣ�����ҳ��֮��չʾ

## <span id = "93">230319</span>
1�����µ㰡���µ㣡��\
2���ó��� ��ʦ��ѧ����ɳĮ  ��ʦ \
3��Ҫ��wireless hart\
4��
�ֲ�ʽϵͳ һ��������
paxosЭ�顢raft��
�������������֣��ֺ�ָ�

## <span id = "93">230321</span>
1����̬�ִ���������!�������nnd��
2��ȫ�Ӿ�+���ܷ�����
3���ٶȸ�/�л��ٶȿ죨ʱ�����У�
4��ͨ������������Ԥ���Ƿ��л�
5���ο��ֻ��Ļ�վ�л��������� MCS ���� BTS 
6��raft

## <span id = "94">230328</span>
1��5 7 8���Ϻ�������
2����Ҫ�޸ĵ���λ����
router��Ϣ��
ESP32��ȱ�ٲ���ʵʱӦ�ú������Ĳ��֡�

## <span id = "95">230329</span>
1��5 7 8���Ϻ�������
2����Ҫ�޸ĵ���λ����
router��Ϣ��
3��10R7N���ȶ���7R3N�ȶ�
4���ṹ���ڴ�û�н�������

## <span id = "96">230330</span>
1��GPSҪ��RMC���ǲ���ۺ��Ժ��

## <span id = "97">230331</span>
1���ϰ� Brownout�����˿��أ�brownout�ø�����
2�����໷û�����Ǿ�������
3����LO��12V 0.24A
4��
Guru Meditation Error: Core  0 panic'ed (LoadProhibited). Exception was unhandled.

Core  0 register dump:
PC      : 0x400f62b6  PS      : 0x00060c30  A0      : 0x800d712e  A1      : 0x3ffafb30  
A2      : 0x3ffb3ea8  A3      : 0x3ffb3ef5  A4      : 0x00000000  A5      : 0x3ffbdc54  
A6      : 0x0101800a  A7      : 0x00000002  A8      : 0x00000000  A9      : 0x3ffafb00  
A10     : 0x00000001  A11     : 0x00000000  A12     : 0x3ffbe6b0  A13     : 0x00000000  
A14     : 0x00000000  A15     : 0x00000001  SAR     : 0x0000001f  EXCCAUSE: 0
[11:53:0:728]Rx��?x0000001c  
EXCVADDR: 0x00000048  LBEG    : 0x4000c2e0  LEND    : 0x4000c2f6  LCOUNT  : 0x00000000  


Backtrace:0x400f62b3:0x3ffafb300x400d712b:0x3ffafb60 0x400d61bb:0x3ffafb90 0x401441a7:0x3ffafc90 0x4008b4b9:0x3ffafcb0 

ʧ��ע�ʹ���
5��������̫���Ͳ���

## <span id = "98">230401</span>
1��8V����̫�����ԣ�


## <span id = "99">230402</span>
1���ϰ� 7�ţ�-40��ʱ��1.5M��+6dB˥�� -60��ʱ��3M

## <span id = "100">230404</span>
1�����Խ�������\
2���ܲ���һ�������̫����\

## <span id = "101">230406</span>
1��׼����һ��һ�϶�

## <span id = "102">230412</span>
1��N12 init phy failed, wrong chip ID�����׽�download������������
2��N13 ��download�������������,��̫��û����
3������û��Ӧ

## <span id = "103">230508</span>
1��Ҫ����һ�¶�����ӵ�������N2��N3reset downloadģʽ��N4��΢��һ�㣬һ��downloadһ����������\
2��

## <span id = "104">230511</span>
1���ߵ��ˣ�����һ�����ӺͲ���\

## <span id = "105">230515</span>
1���ȿ���ÿ������ܲ��ܺ�1��ROOT������\
2��
2����̫�����С�3����̫��ʱ��ʱ��

## <span id = "106">230516</span>
1��2����̫��0%��3����̫��85%��4��60%��5���ϵ�û������6���ϵ�û������7��95%��8��100%��9��100%��11��100% \
2��6���ӽڵ����ӣ�2 3 4 7 11 9 \
3�����μ���˳��9 3 2 4 11 ���� 7û�м���
4�����泣פ�ģ�3 4 9
5����2.4G�����߿�һ��

## <span id = "107">230516</span>
1��2.4G��8����ȫû�������˵\
```
I (15482) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (15482) wifi:station: b0:b2:1c:8f:91:e8 join, AID=1, bgn, 40D
[0;32mI (15484) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (15486) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 2[0m
[0;32mI (15498) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (15504) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (15509) [eth2mesh, 811]: Child is connected on ap interface[0m
I (26955) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (26955) wifi:station: b0:b2:1c:8f:93:d8 join, AID=2, bgn, 40D
[0;32mI (26957) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (26959) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 3[0m
[0;32mI (26972) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (26977) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (26982) [eth2mesh, 811]: Child is connected on ap interface[0m
I (38013) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (38013) wifi:station: b0:b2:1c:8f:92:d8 join, AID=3, bgn, 40D
[0;32mI (38014) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (38015) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 4[0m
[0;32mI (38029) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (38035) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (38040) [eth2mesh, 811]: Child is connected on ap interface[0m
I (105192) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (105192) wifi:station: b0:b2:1c:8f:92:f8 join, AID=4, bgn, 40D
I (105194) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (105199) wifi:station: b0:b2:1c:8f:93:14 join, AID=5, bgn, 40D
[0;32mI (105206) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (105208) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 5[0m
[0;32mI (105221) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (105226) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (105228) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 6[0m
[0;32mI (105242) [eth2mesh, 811]: Child is connected on ap interface[0m
[0;32mI (105249) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (105254) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (105260) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (105265) [eth2mesh, 811]: Child is connected on ap interface[0m
I (130685) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (130685) wifi:station: b0:b2:1c:8f:93:c4 join, AID=6, bgn, 40D
[0;32mI (130687) [eth2mesh, 786]: event_loop_cb, event: 25[0m
I (130691) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
I (130698) wifi:station: b0:b2:1c:8f:93:50 join, AID=7, bgn, 40D
[0;32mI (130689) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 7[0m
[0;32mI (130714) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (130719) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (130721) [mwifi, 243]: Routing table is changed by adding newly joined children add_num: 1, total_num: 8[0m
[0;32mI (130735) [eth2mesh, 811]: Child is connected on ap interface[0m
[0;32mI (130742) [eth2mesh, 786]: event_loop_cb, event: 25[0m
[0;32mI (130747) [eth2mesh, 786]: event_loop_cb, event: 5[0m
[0;32mI (130753) [eth2mesh, 786]: event_loop_cb, event: 3[0m
[0;32mI (130758) [eth2mesh, 811]: Child is connected on ap interface[0m
I (193800) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (193800) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (195010) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (195011) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (196198) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (196199) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (197400) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (197401) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (198618) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (198619) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (199806) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (199807) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
I (200996) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (200997) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:1, <max:128 = cfg:128 + extra:0>tods:0
W (201187) wifi:inactive timer: now=bcd6512 last_rx_time=9fdbde2 diff=76b1, aid[4]b0:b2:1c:8f:92:f8 leave
I (201188) wifi:station: b0:b2:1c:8f:92:f8 leave, AID = 4, bss_flags is 134243, bss:0x3ffdc324
I (201194) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
W (201200) wifi:inactive timer: now=bcd98b4 last_rx_time=9fe214a diff=76a5, aid[5]b0:b2:1c:8f:93:14 leave
I (201209) wifi:station: b0:b2:1c:8f:93:14 leave, AID = 5, bss_flags is 134243, bss:0x3ffde8a0
I (201218) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
[0;32mI (201225) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 7[0m
[0;32mI (201225) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (201237) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 6[0m
[0;32mI (201250) [eth2mesh, 817]: Child is disconnected on ap interface[0m
[0;32mI (201257) [eth2mesh, 786]: event_loop_cb, event: 6[0m
[0;32mI (201262) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (201268) [eth2mesh, 817]: Child is disconnected on ap interface[0m
[0;32mI (201274) [eth2mesh, 786]: event_loop_cb, event: 6[0m
[0;33mW (201282) [mwifi, 707]: <ESP_ERR_MESH_DISCONNECTED> Node failed to send packets, dest_addr: b0:b2:1c:8f:92:f8, flag: 0x02, opt->type: 0x08, opt->len: 13, data->tos: 0, data: 0x3ffddc48, size: 66[0m
[0;33mW (201298) [mwifi, 806]: <ESP_ERR_MESH_DISCONNECTED> Root node failed to send packets, dest_mac: b0:b2:1c:8f:92:f8[0m
[0;33mW (201309) [mwifi, 1090]: <ESP_ERR_MESH_DISCONNECTED> mwifi_root_write, size: 66[0m
[0;33mW (201317) [eth2mesh, 501]: <ESP_ERR_MESH_DISCONNECTED> mwifi_read[0m
W (202680) wifi:inactive timer: now=be42c5a last_rx_time=a1477ef diff=76b5, aid[6]b0:b2:1c:8f:93:c4 leave
I (202680) wifi:station: b0:b2:1c:8f:93:c4 leave, AID = 6, bss_flags is 134243, bss:0x3ffdeba4
I (202687) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
W (202693) wifi:inactive timer: now=be4601a last_rx_time=a14d998 diff=76a9, aid[7]b0:b2:1c:8f:93:50 leave
I (202702) wifi:station: b0:b2:1c:8f:93:50 leave, AID = 7, bss_flags is 134243, bss:0x3ffdeea8
I (202711) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
[0;32mI (202717) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (202718) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 5[0m
[0;32mI (202732) [eth2mesh, 817]: Child is disconnected on ap interface[0m
[0;32mI (202734) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 4[0m
[0;32mI (202749) [eth2mesh, 786]: event_loop_cb, event: 6[0m
[0;32mI (202755) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (202760) [eth2mesh, 817]: Child is disconnected on ap interface[0m
[0;32mI (202767) [eth2mesh, 786]: event_loop_cb, event: 6[0m
I (212751) mesh: [scan]new scanning time:600ms, beacon interval:300ms
I (220559) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (220559) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
I (221803) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (221803) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
W (222008) wifi:inactive timer: now=d0b18ad last_rx_time=b2caab5 diff=7a7a, aid[3]b0:b2:1c:8f:92:d8 leave
I (222008) wifi:station: b0:b2:1c:8f:92:d8 leave, AID = 3, bss_flags is 134243, bss:0x3ffaee50
I (222015) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
[0;32mI (222022) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (222023) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 3[0m
[0;32mI (222037) [eth2mesh, 817]: Child is disconnected on ap interface[0m
I (222038) mesh: [scan]new scanning time:300ms, beacon interval:100ms
[0;32mI (222044) [eth2mesh, 786]: event_loop_cb, event: 6[0m
I (222989) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (222989) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
I (224155) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (224155) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
I (225382) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (225383) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
I (226686) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (226687) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
I (227756) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (227756) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
W (228952) wifi:inactive timer: now=d750bbe last_rx_time=b9694b6 diff=7a7c, aid[2]b0:b2:1c:8f:93:d8 leave
I (228952) wifi:station: b0:b2:1c:8f:93:d8 leave, AID = 2, bss_flags is 134243, bss:0x3ffafb68
I (228958) wifi:new:<12,2>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
[0;32mI (228965) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (228966) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 2[0m
[0;32mI (228980) [eth2mesh, 817]: Child is disconnected on ap interface[0m
[0;32mI (228987) [eth2mesh, 786]: event_loop_cb, event: 6[0m
I (228993) mesh: [TXQ]<max:128>up(0, be:0), down(0, be:0), mgmt:0, xon(req:0, rsp:16), bcast:0, wnd(0, parent:00:00:00:00:00:00)
I (229004) mesh: [RXQ]<max:128 = cfg:128 + extra:0>self:0, <max:128 = cfg:128 + extra:0>tods:0
[0;33mW (229013) [mwifi, 707]: <ESP_ERR_MESH_DISCONNECTED> Node failed to send packets, dest_addr: b0:b2:1c:8f:93:d8, flag: 0x02, opt->type: 0x08, opt->len: 13, data->tos: 0, data: 0x3ffde38c, size: 66[0m
[0;33mW (229030) [mwifi, 806]: <ESP_ERR_MESH_DISCONNECTED> Root node failed to send packets, dest_mac: b0:b2:1c:8f:93:d8[0m
[0;33mW (229041) [mwifi, 1090]: <ESP_ERR_MESH_DISCONNECTED> mwifi_root_write, size: 66[0m
[0;33mW (229049) [eth2mesh, 501]: <ESP_ERR_MESH_DISCONNECTED> mwifi_read[0m
I (238981) mesh: [scan]new scanning time:600ms, beacon interval:300ms
[0;32mI (251003) eth2mesh: Ethernet Link Down[0m
W (289479) wifi:inactive timer: now=11109eb5 last_rx_time=f344bfb diff=79f0, aid[1]b0:b2:1c:8f:91:e8 leave
I (289479) wifi:station: b0:b2:1c:8f:91:e8 leave, AID = 1, bss_flags is 134243, bss:0x3ffaf458
I (289486) wifi:new:<12,0>, old:<12,2>, ap:<12,2>, sta:<0,0>, prof:12
[0;32mI (289493) [eth2mesh, 786]: event_loop_cb, event: 4[0m
[0;32mI (289493) [mwifi, 252]: Routing table is changed by removing leave children remove_num: 1, total_num: 1[0m
[0;32mI (289508) [eth2mesh, 817]: Child is disconnected on ap interface[0m
I (289510) mesh: [scan]new scanning time:300ms, beacon interval:100ms
[0;32mI (289515) [eth2mesh, 786]: event_loop_cb, event: 6[0m
I (299523) mesh: [scan]new scanning time:600ms, beacon interval:300ms

```


## <span id = "108">230524</span>
1��2����̫��0%��3����̫��85%��4��60%��5���ϵ�û������6���ϵ�û������7��95%��8��100%��9��100%��11��100% \
2�����м���û�õĽڵ�
1����û����
2�� ��̫���������⣬�Ѿ��޸���2.4G���ԣ�1.4G�������У����������ԡ�
3�� 2.4G���ԣ�1.4Gһ��pin��ͨ
4�� 1.4G �β�һ�¿���
5�� 2.4G���ԣ�J30J�β�����ϵ磬1.4G������
6�� 2.4G���ԣ�1.4G������
7�� 2.4Gһ��Ҳpin��ͬ
8�� û����
9�� ����
10�� 2.4Gû���⣬1.4G������
11�� 1.4Gû����
12�� N12 init phy failed, wrong chip ID�����׽�download������������
13�� ���������⣬û����
14�� ���������⣬û����
15�� 1.4G����
16�� 2.4Gû���⣬1.4G������
3��
1��4��8��9��11��15������

2��3��5��6��10��16��2.4G���ԣ�1.4G����
7����̫��ż��ͨż����ͨ��������ʱ��
12��wrong chip ID������8201оƬ
13������download������һֱdownload
14�����������⣬û�����ݣ�downloadҲû��


## <span id = "109">230531</span>
1��SPI���˵����Ժ���Ҳ���Ǻܺ�ʹ������nnd\

## <span id = "110">230601</span>
1��SPI����Non ���� Root �� 2.4G \
2��SPI����Root ���� Non �� 2.4G \
3������SPI - SPI�Ľ��\
4���ε�SPI���MOSI�����յ�������������������յ���������ˣ�Root�����·�������£���Nonroot�����·�Ҳ��һ���� ���� ���߶����·��Ͳ��У�nnd\

## <span id = "111">230604</span>
1��2��5��6��7��12��16 �Ǻõģ�������\
2��