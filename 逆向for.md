```
PlantsVsZombies.exe+28E50 - lea eax,[ebx+eax+28] // 算数运算 eax = ebx + eax + 28，也就是 eax = (esi*50) + [edi+0000015C] + 28
PlantsVsZombies.exe+28E54 - call PlantsVsZombies.exe+B2FB0
PlantsVsZombies.exe+28E59 - mov eax,[edi+0000015C] // arr[0]的起始内存位置
PlantsVsZombies.exe+28E5F - inc esi  // 每次递增1
PlantsVsZombies.exe+28E60 - add ebx,50 // 每次+50，初始化因该是0
PlantsVsZombies.exe+28E63 - cmp esi,[eax+24]  // if esi < [eax+24], then 28E50
PlantsVsZombies.exe+28E66 - jl PlantsVsZombies.exe+28E50
```

28 属性的偏移,属性还有可能是一个结构体