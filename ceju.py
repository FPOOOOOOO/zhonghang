# %%
import re # 导入正则包
import os

path = "/Users/fpo/Desktop/0920ceju.txt"  # 文件夹目录
allresult=[]

f = open(path, "r", encoding='utf-8')  # 设置文件对象
data = f.readlines()  # 直接将文件中按行读到list里，效果与方法2一样

#print(data)
pattern = re.compile(r'number (.*?). =:') 
result = pattern.findall(str(data)) # 模式匹配方法，找到所有模式为pattern的字段 
for i in range(len(result)):
    result[i]=int(result[i])
    if result[i]==760:
        begin=i
    if result[i]==860:
        end=i
##print(len(result))

print(result)

print('总丢包率:',(1490-len(result))/1490)
print('300米丢包率:',(100-(end-begin))/100)

pattern2 = re.compile(r'RSSI = (.*?) ')
result2 = pattern2.findall(str(data))
for i in range(len(result2)):
    result2[i]=int(result2[i])
print(result2)
##print(len(result2))
#allresult = allresult + result + result2 # 两个模式为去除来的列表合并
#print(allresult)
f.close() # 关闭数据流

# %%
#import numpy as np
import matplotlib.pyplot as plt
#from matplotlib.dates import DateFormatter,HourLocator
# plt.rcParams["font.sans-serif"]=["SimHei"]
# plt.rcParams["axes.unicode_minus"]=False

x=result
y=result2
#plt.plot(x, y)


plt.scatter(x,y,s=1)


plt.show()

# fig=plt.figure()
# ax=fig.add_axes((0.0,0.0,1.0,1.0))
# ax.plot(x_date,y_date,'->',ms=8,mfc='#FF9900')
# ax.set_xlabel('时间')
# ax.set_ylabel('RSSI')
# # date_fmt=DateFormatter('%H:%M')
# # ax.xaxis.set_major_formatter(date_fmt)
# # ax.xaxis.set_major_locator(HourLocator(interval=2))
# # ax.tick_params(direction='in',length=6,width=2,labelsize=12)
# # ax.xaxis.set_tick_params(labelrotation=45)
# plt.title("0921")
# plt.show()

# %%

file = open('/Users/fpo/Desktop/0922ceju.txt', 'w', encoding='utf-8')
file.write(str(result+result2)) # 将文件写入目标文件夹
file.close() # 关闭数据流
# %%
