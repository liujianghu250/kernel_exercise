# kernel_exercise
学习linux内核时的练习编程题

## i.	 编写一个内核模块，向用户态暴露出一个Proc接口，输入"pid virtual_address"返回虚拟地址对应的物理页框号

在vir_to_pfn/modules目录下的vaddr_pfn.c。make之后insmod，再使用echo、cat对/proc/vaddr_pfn进行读写即可


## ii.	 编写一个用户态程序，输入"pid virtual_address"返回虚拟地址对应的物理页框号（通过编程题i中的内核模块实现）
在vir_to_pfn/user目录下。

vaddr_pfn.c是在编程题i实现的内核模块的基础上实现的。

vaddr_pfn_by_pagemap.c是通过对/proc/(pid)/pagemap进行读写实现的。

经测试，两者运行结果相同。

vir_to_pfn/test_proc目录下的test.c是为了测试而编写的程序，它会打印出一个有效的线性地址范围，可以通过ps命令得到test进程的pid后，再利用这个地址范围内的有效地址进行对程序进行验证。


## iii.	写一个内核模块遍历系统中的所有进程（线程），打印每个进程的虚拟内存和物理内存大小

在pmeminfo目录下，pmeminfo.c即是实现的内核模块。
