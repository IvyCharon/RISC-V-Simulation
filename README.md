# RISC-V-Simulation

### 类及hpp介绍
+ `func_and_tools.hpp`: `myspa`中放了所有需要的枚举类型、常量、功能函数（得到立即数和十六进制转十进制）。
+ `memory.hpp`: `Memory_simu`类作为内存，把读入的指令放置在`mem[]`中，`initialize()`时把所有指令读入。
+ `program.hpp`: 在类`program`中，`Reg[]`指代寄存器。

### 五级流水：
+ 当遇到data_hazard和部分control_hazard时暂停流水。
+ 结构体`ID_Reg`,`EXE_Reg`,`MEM_Reg`,`WB_Reg`分别用于暂存从`IF()`到`ID()`，从`ID()`到`EXE()`，从`EXE()`到`MEM()`，从`MEM()`到`WB()`的数据。
+ 需要暂停流水时，在`IF()`时设置`dReg.now_code`为-1，在`ID()`时若`dReg.now_code`为-1，则把`eReg.type`设为`NOType`，在接下来的函数中，若其类型为`NOType`，则把它传给下一个函数的结构体中的type也设为`NOType`，并return。这样流水就被暂停了一个回合。

### `mem()`三周期模拟
+ `cycle`用于在`mem()`时用三周期模拟，当某条指令需要内存操作时，设置`cycle`为3，接下来的循环中，`IF()`时若`cycle`不为0，就将`cycle`减1并暂停流水。

### 分支预测
+ 使用两位饱和计数器，结构体`branch_pre_cache`用于缓存。
+ 在`IF()`时判断上一条指令是否是B开头的指令，再根据缓存结构体的`jud()`预测是否跳转，在这个过程中，用结构体中的`pos_c`暂存该指令的位置，pc不改变。
+ 在`EXE()`中跳转与不跳转的情况下分别对缓存结构体调用`push()`函数。若预测成功则继续流水，若预测不成功则对`dReg`重新取值，然后流水可以继续。
+ 特别的，若读到的指令是结束指令，则不论是否预测成功，都当成不成功，直接把`pos_c`设为pc不可能变为的值，把`dReg.now_code`设为-1。
+ 调用函数`success_rate()`可输出成功率，最高能达到85%，最低只有17%，平均在60%左右。

### forwarding
+ 在`ID()`时判断这一轮是否用到上一轮要修改的寄存器，如果用到，就把值从上一轮`EXE()`时算出来的`mReg.data`直接拿来。
