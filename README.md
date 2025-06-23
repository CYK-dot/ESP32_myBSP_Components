# 项目简介
本项目是本人在开发ESP32过程中编写的一些组件，方便以后查阅。

# 注意事项
1. 在VSCode中，ESP插件只能从example中生成工程，直接创建工程无法编译
2. 可以进入"项目组件"，以组件的视图平铺代码文件，方便查看
3. 考虑到要重新编译，本项目不采用sdkconfig配置，而是直接在.h代码中配置
4. VSCode模板我已经写好了，可以用c_hdr和c_src唤出
5. idf.py create-component之前，需要先把目录转到components文件夹下，否则他会创建在根目录下
6. 在c_cpp_properties.json中我加入了"${workspaceFolder}/build/config"，这样就不会报错CONFIG_了