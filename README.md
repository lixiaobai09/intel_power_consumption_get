# intel_power_consumption_get

#### Description
a program to get the power consumption for intel  
you can read Chinese blog: https://blog.csdn.net/Xiaobai__Lee/article/details/100729269 for more details

**notice**: The MSR_PKG_ENERGY_STATUS register counts the all cores of a package (i.e. a physical CPU on a socket), so do not add redundant cores in a package to count.

#### Software Architecture
no

#### Installation

1. Your cpu should be intel. If it's AMD, you can read the manual pdf for AMD and code a program about it.
2. Before run this program, you need to add the module cpuid and msr with commands:  
    sudo modprobe cpuid  
    sudo modprobe msr
3. You need to get root provillege to run this program.
