# 一、项目简介
本项目是本人在开发ESP32过程中编写的一些组件和开发心得，方便以后查阅。

# 二、注意事项
1. 在VSCode中，ESP插件只能从example中生成工程，直接创建工程无法编译
2. 可以进入"项目组件"，以组件的视图平铺代码文件，方便查看
3. 考虑到要重新编译，本项目不采用sdkconfig配置，而是直接在.h代码中配置
4. VSCode模板我已经写好了，可以用c_hdr和c_src唤出
5. idf.py create-component之前，需要先把目录转到components文件夹下，否则他会创建在根目录下
6. 在c_cpp_properties.json中我加入了"${workspaceFolder}/build/config"，这样就不会报错CONFIG_了
7. wifi配置中，SSID还有ssid_len，别忘了写，不然wifi不会被探测到。
8. wifi速率是自适应的，如果需要手动调整，可以调用esp_wifi_config_80211_tx函数配置
9. ESP System Settings中有日志输出的配置，如果保留在UART0，就会使得ESP32可以看到日志，ESP32C3不行
10. wifi底层有容错设计，如果因信号质量而断连，应用层是无法得知的，反而是重新建立连接后，才能通过wifi事件后知后觉。
11. 例如对ESP-NOW应用，可以维护一个定时器，在回调函数中复位该定时器，进而得知是否断连，比较简单，无需封装成模块。
12. mavlink重新生成代码后，要在mavlink_helpers.h中加上#include "方言/mavlink.h"，不能#include "mavlink.h"，或是在任意的文件中先#nclude "方言/mavlink.h"再#include "mavlink_helpers.h"。
13. mavlink的其他功能也和freertos一样，必须先#include最大的mavlink.h，然后才能include其他文件，否则会报错
14. 资源管理经验+1：不要靠感觉去思考什么时候释放资源，如果释放早了，就会crash。忘记释放了，就会漏内存。正确的做法是，提前思考资源的申请和释放在什么地方，而不是等到写代码了再去思考这里是不是要释放资源。

# 三、功能选型
## 1. 通用无线通信
1. 远距离、低延迟、少量数据：802.11LR+ESP-NOW
    要注意，2.4GHz本来就不适合这个场景，最强的802.11LR只能做到中等延迟。
    BLE虽然低延迟但做不到远距离，因为射频功率在那摆着。
    真要低延迟+远距离，433MHz最合适。

2. 近距离、需要低延迟：蓝牙
    这个场景是2.4GHz的舒适区之一，降低速率实现低功耗和低延迟。
    中量数据的低延迟，比如音频、串口数据，用经典蓝牙。
    少量数据的低延迟，比如传感器数据，可以进一步选择低功耗蓝牙。

3. 近距离、需要大量数据：WiFi
    这个场景是2.4GHz的舒适区之一，把速率拉满但牺牲了功耗和延迟。

## 2. 持久存储
1. 读写，少量数据，类似变量：NVS
2. 读写，大量数据，类似文件：SPIFFS
3. 只读：IDF内嵌二进制文件

# 四、学习笔记

## 1. 常用的持久性存储
NVS存储数据的方式是，在Flash中开一个分区，专门用于存储哈希键值对，使用者通过命名空间+键来唯一地确定一块数据。
SPIFFS也会在Flash中新开一个分区，但这个分区上运行的是文件系统而不是键值对。
> NVS更像是数据库，而SPIFFS是文件系统。二者管理数据的逻辑是不同的。

## 2. 蓝牙基本概念
wifi之上一般只有TCP/IP一种协议栈，而蓝牙有两种协议栈：经典蓝牙、低功耗蓝牙。
TCP/IP协议栈的框架是OSI，而两种蓝牙协议栈遵循的不是OSI，而是蓝牙四层模型。

<table border="1" style="margin:auto; text-align:center;">
  <tr>
    <th>层</th>
  </tr>
  <tr>
    <td>Profile（应用层）</td>
  </tr>
  <tr>
    <td>Host 层</td>
  </tr>
  <tr>
    <td>HCI 层</td>
  </tr>
  <tr>
    <td>Controller 层</td>
  </tr>
</table>

对于BLE而言，四层模型中包含如下的协议(不全)
1. 蓝牙controller负责进行物理通信，由PHY以及操作PHY的链路(LL)构成。
2. HCI则对controller的通信进行抽象，屏蔽物理层的差异，比如只运行HCI，暴露UART接口，Host就能通过UART接入蓝牙。
3. Host层是蓝牙的核心，描述了蓝牙的核心功能，开发者需要关注这一层的机制，因为他直接被应用层调用。

<table border="1" style="margin:auto; text-align:center;">
  <tr>
    <th>层</th>
    <th>典型协议/模块</th>
  </tr>
  <tr>
    <td>Profile（应用层）</td>
    <td>HID（键鼠）、SPP（串口）、HSP（耳机）、OTP（文件）</td>
  </tr>
  <tr>
    <td>Host 层</td>
    <td>GAP（发现/连接）、GATT（属性读写）、SMP（配对加密）</td>
  </tr>
  <tr>
    <td>HCI 层</td>
    <td>命令包、数据包、事件包、同步包</td>
  </tr>
  <tr>
    <td>Controller 层</td>
    <td>PHY（射频）、LL（链路层）</td>
  </tr>
</table>

> 要注意HCI和Profile的区别，如果UART提供的是HCI，那么上位机可以有全面的蓝牙支持。
> 而如果UART提供的是SPP，那么对上位机而言，蓝牙只是一种串口透传工具而已，底层细节全屏蔽。

TCP/IP协议栈有着不同的部署，比如LwIP就是其中的一种。
而对于BLE，一般有两种部署，BlueDroid、NimBLE，前者体积较大功能较全，后者体积和开销较小。
乐鑫在NimBLE之上提供了自己的Profile，例如Blufi、Mesh等。

# 五、项目组件
## 1. easy_nvs
> 将NVS键值对操作封装成函数，实现数据断电后的可持续性保存
> 对NVS操作进行了标准化，通过返回值判断操作是否成功，同时也会输出日志到控制台
> NVS不适合频繁操作，easy_nvs中提供的每个函数都视为单独的事务，时间开销大
> 如果需要高效率操作，请继续使用IDF的NVS API
> 对于大型文件，请使用spiffs

1. 经过测试，easy_nvs只能填入结构体，不能填入字符串，即使在o0下字符串仍然无法正常更新
2. 经过测试，烧录只会修改代码分区，不会修改NVS分区，因此之前的东西是会保留的，需要注意

## 2. easy_wifi
> 封装了WiFi操作，以及基于wifi操作的前端，方便调试

> 文件结构为两层结构,四个组件单独使用各自的.h文件，共享prv_ewifi.h文件。
> 函数式交互接口: wifi_basic仅实现wifi联网，wifi_advance则提供联网之外的其他功能。
> 前端式交互接口：wifi_interface_xx封装函数式交互接口，提供无需编程的前端，方便调试。

- ewifi_basic
1. 初始化：提供完整的三步wifi参数配置，[外设参数+指定射频参数+指定协议参数]。
2. 事件回调：提供所有wifi事件的回调注册。
3. 配置读取与保存。
- ewifi_advance
1. 扫描周边的AP热点
2. 得知当前AP的RSSI
3. 使用CSI实现频段扫描(暂不支持,todo)
- ewifi_if_cli
1. 提供wifi_basic中所有接口的命令行前端
2. 提供wifi_advance中所有接口的命令行前端
- ewifi_if_web
1. 提供wifi_basic中所有接口的web前端