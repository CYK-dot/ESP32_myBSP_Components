# 项目简介
本项目是本人在开发ESP32过程中编写的一些组件，方便以后查阅。

# 注意事项
1. 在VSCode中，ESP插件只能从example中生成工程，直接创建工程无法编译
2. 可以进入"项目组件"，以组件的视图平铺代码文件，方便查看
3. 考虑到要重新编译，本项目不采用sdkconfig配置，而是直接在.h代码中配置
4. VSCode模板我已经写好了，可以用c_hdr和c_src唤出
5. idf.py create-component之前，需要先把目录转到components文件夹下，否则他会创建在根目录下
6. 在c_cpp_properties.json中我加入了"${workspaceFolder}/build/config"，这样就不会报错CONFIG_了
7. wifi配置中，SSID还有ssid_len，别忘了写，不然wifi不会被探测到

# 项目组件

## easy_nvs
> 将NVS键值对操作封装成函数，实现数据断电后的可持续性保存
> 对NVS操作进行了标准化，通过返回值判断操作是否成功，同时也会输出日志到控制台
> NVS不适合频繁操作，easy_nvs中提供的每个函数都视为单独的事务，时间开销大
> 如果需要高效率操作，请继续使用IDF的NVS API
> 对于大型文件，请使用spiffs

1. 经过测试，easy_nvs只能填入结构体，不能填入字符串，即使在o0下字符串仍然无法正常更新
2. 经过测试，烧录只会修改代码分区，不会修改NVS分区，因此之前的东西是会保留的，需要注意

## easy_wifi
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