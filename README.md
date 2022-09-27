# Í¨ÓÃ
- ÉÏ´«µ½github²Ö¿â
~~~
git add -u
git status
git commit -m "0831"
git push origin ZHONGHANGmdf
~~~


# DebugÈÕÖ¾


## <span id = "1">0906</span>

1¡¢Qt½ÚµãÍØÆËÊÇ·ñÒÔÊµ¼ÊµÄ½ÚµãÊýÁ¿À´ \
2¡¢¹ã²¥ºÍµã¶Ôµã·¢ËÍµÄµØÖ·ÔÚÄÄ¶ùÉèÖÃ£¬Í¨¹ýÉÏÎ»»úÔÚ´úÂëÀïÉèÖÃ£¬»¹ÊÇÔÚETHÉÏÖ±½Ó¶Ô×Åip·¢\
3¡¢Èí¼þ Reboot Ö÷´ÓµÈ²ÎÊý\
4¡¢¼ÓÈëSPI \
5¡¢µ÷ÕûmeshÍøÂç²ãÊý \
6¡¢QtÓÐ¸ö½ø³ÌÒ»Ö±ÔÚÊÕUDP

## <span id = "2">0913</span>

1¡¢Qt½ÚµãÍØÆËÊÇ·ñÒÔÊµ¼ÊµÄ½ÚµãÊýÁ¿À´ \
2¡¢¹ã²¥ºÍµã¶Ôµã·¢ËÍµÄµØÖ·ÔÚÄÄ¶ùÉèÖÃ£¬Í¨¹ýÉÏÎ»»úÔÚ´úÂëÀïÉèÖÃ£¬»¹ÊÇÔÚETHÉÏÖ±½Ó¶Ô×Åip·¢ ¡ª¡ª ÓÐÂ·ÓÉÑ¡Ôñ\
3¡¢Èí¼þ Reboot Ö÷´ÓµÈ²ÎÊý\
4¡¢¼ÓÈëSPI \
5¡¢µ÷ÕûmeshÍøÂç²ãÊý \
6¡¢QtÓÐ¸ö½ø³ÌÒ»Ö±ÔÚÊÕUDP \
7¡¢~~Qt Ñ¡ÔñÍø¿Ú~~
8¡¢²âÊÔ1.4G 4351
9¡¢×«Ð´±¨¸æ

## <span id = "3">0915</span>

```
 [15:27:30:577]Rx¡û?I (9081) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (9091) wifi:state: init -> auth (b0)
I (9101) wifi:state: auth -> assoc (0)

[15:27:30:643]Rx¡û?I (9151) wifi:state: assoc -> run (10)

[15:27:34:616]Rx¡û?I (13121) wifi:state: run -> init (fc0)
I (13121) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (13131) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (13131) ADF4351: retry to connect to the AP[0m
[0;32mI (13131) 
[15:27:34:672]Rx¡û?ADF4351: connect to the AP fail[0m

[15:27:36:673]Rx¡û?[0;32mI (15181) ADF4351: retry to connect to the AP[0m
[0;32mI (15181) ADF4351: connect to the AP fail[0m

[15:27:36:724]Rx¡û?I (15231) wifi:new:<1,1>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
I (15231) wifi:state: init -> auth (b0)
I (15241) wifi:state: auth -> assoc (0)

[15:27:36:768]Rx¡û?I (15271) wifi:state: assoc -> run (10)

[15:27:40:757]Rx¡û?I (19261) wifi:state: run -> init (fc0)
I (19261) wifi:new:<1,0>, old:<1,1>, ap:<255,255>, sta:<1,1>, prof:1
I (19271) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,1>, prof:1
[0;32mI (19271) ADF4351: connect to the AP fail[0m
[0;32mI (19271) ADF4
[15:27:40:809]Rx¡û?351: Failed to connect to SSID:fpoap, password:jjjkkk05[0m

```

1¡¢¸Ä»·Â·ÂË²¨Æ÷ \
2¡¢Ìø¹ýËÄ´ÎÎÕÊÖ½×¶Î¡ª¡ª¿´Ò»ÏÂËÄ´ÎÎÕÊÖ½×¶ÎµÄ°ü³¤\
3¡¢´®¿Ú¸ÉÈÅµ½ËøÏà»·

## <span id = "4">0916</span>
1¡¢°ü³¤Ôö¼Ó ¡ª¡ª È·Êµ°ü³¤Ôö¼ÓÁËµ¼ÖÂ¶ª°üºÜÑÏÖØ£¬µ«ÊÇÎóÂëÂÊ»¹ÐÐ£¬¾ÍÊÇ¶ª°ü\
2¡¢Â¥Àï²â¾à ¡ª¡ª Ê§°ÜÁË£¬²âÊÔÊý¾Ý¼ûPPT


## <span id = "5">0917</span>
1¡¢°ü³¤Ôö¼Ó¡ª¡ªÔÚÔöÒæºÃµÄÇé¿öÏÂ£¬»¹ÊÇÍ¨²»¹ý \
- °ÑWiFi·¢ÉäÔöÒæ¼õÐ¡£¬10dB£¬15dB:
2¡¢Â¥Àï²â¾à¡ª¡ªÍê³ÉÁË£¬ÌìÏßµÄÔöÒæºÍÆ¥ÅäÎÊÌâ

## <span id = "6">0919</span>
1¡¢Ìø¹ýÈÏÖ¤½×¶Î²âËÙ¡ª¡ª ÏÈÓÃ×Ô´øµÄiperf²âÊÔ£¬ºÍ2.4G½øÐÐ±È½Ï\
2¡¢ÓÃÁËÐÂÌìÏß£¬¸Äµ½20dB \
3¡¢saving new calibration data because of checksum failure, mode(0) ¡¢\
4¡¢¡û?I (16388) wifi:bcn_timout,ap_probe_send_start \
5¡¢Ã»ÓÐ½øÐÐDHCPµÄ·Ö·¢

## <span id = "7">0920</span>
1¡¢»»ÁËÌìÏßÒÔºó£¬½øÐÐÁË¾àÀëµÄ²âÊÔ\

## <span id = "8">0921</span>
1¡¢ÂèµÄÊý¾Ý¶ªÁË£¬½øÐÐÒ»¸öÍ¼µÄ»æÖÆ ¡ª¡ªlinux¿ÉÒÔÓÃred

## <span id = "9">0922</span>
1¡¢·¢Éä¹¦ÂÊµ½ 15 £¬×ßÀÈÉÏ²â£¬°ÎµôÌìÏßÁ¬²»ÉÏ²ÅÐÐ \
2¡¢Èç¹ûË¥¼õ²»ÏÂÈ¥£¬ÊÇÓÐ±¾ÌåÔëÉùµÄÓ°Ïì£¬ÒªÍ¨¹ý1ÅÅ³ýÕâ¸öÓ°Ïì \
3¡¢»¹Òª¿´Ò»ÏÂ±¨¸æÀï1.4GµÄ»¹²îÄÄÐ©

## <span id = "10">0923</span>
1¡¢·¢Éä¹¦ÂÊµ½ 15 £¬×ßÀÈÉÏ²â£¬°ÎµôÌìÏßÁ¬²»ÉÏ²ÅÐÐ \
2¡¢Èç¹ûË¥¼õ²»ÏÂÈ¥£¬ÊÇÓÐ±¾ÌåÔëÉùµÄÓ°Ïì£¬ÒªÍ¨¹ý1ÅÅ³ýÕâ¸öÓ°Ïì \
3¡¢»¹Òª¿´Ò»ÏÂ±¨¸æÀï1.4GµÄ»¹²îÄÄÐ©

## <span id = "11">0924</span>
1¡¢·¢Éä¹¦ÂÊµ½ 15 £¬×ßÀÈÉÏ²â£¬°ÎµôÌìÏßÁ¬²»ÉÏ²ÅÐÐ \
2¡¢Èç¹ûË¥¼õ²»ÏÂÈ¥£¬ÊÇÓÐ±¾ÌåÔëÉùµÄÓ°Ïì£¬ÒªÍ¨¹ý1ÅÅ³ýÕâ¸öÓ°Ïì \
3¡¢»¹Òª¿´Ò»ÏÂ±¨¸æÀï1.4GµÄ»¹²îÄÄÐ© ¡ª¡ª ÒÑ¾­±êºì,ÔÚPPTÀïÕûÀíÁË\
4¡¢Ôö¼ÓÁËETHÍ¸´«²¿·Ö£¬²âÊÔÓÃ¡ª¡ª»¹ÐèÒªÈ·¶¨meshÊÇ·ñ¿ÉÒÔÔÚÎÞÈÏÖ¤µÄÇé¿öÏÂÊ¹ÓÃ \

## <span id = "12">0927</span>
1¡¢ÒÔÌ«Íøinit mac time out ¡ª¡ª ÊÇ²»ÊÇGPIO0Ê±ÖÓµÄÎÊÌâ \
2¡¢ÒÔÌ«ÍøÍ¸´«ÓÖÊ§°ÜÁË£¬·þÁË¡ª¡ª ¿´¿´0505µÄ°æ±¾ÈçºÎ£¬ÏÈ±£Ö¤2.4G°å×ÓÃ»ÎÊÌâ£¬È»ºóÔÙÒÆÖ²µ½1.4G£¬ÏÖÔÚµÄ1.4G STA ETHÔÚ2.4GÉÏÒ²ºÜ²î£¨ÓÐ¿ÉÄÜÊÇGPIO0µÄÎÊÌâ£©\
3¡¢