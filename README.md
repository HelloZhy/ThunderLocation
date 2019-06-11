# ThunderLocation
利用广义互相关实现雷声定位。(目前只完成仿真测试，并未通过实际环境的检测)

 * `Program`文件夹为定位程序。
 * `SimuProgram`文件夹为雷声模拟程序。

##编译

```
cd ./Program/ && make
cd ./SimuProgram/ && make
```

##运行顺序
1. 运行`cd ./SimuProgram/ && ./simu ./xxxx_yy/`，其中xxxx_yy为仿真文件夹下的模拟信号文件夹。
2. 运行`cd ./Program/ && ./program`。
