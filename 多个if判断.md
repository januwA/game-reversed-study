## 正向代码
```
int main()
{
	bool setHP = true;
	bool setMP = false;
	bool setGold = true;

	if (setHP)
	{
		std::cout << "设置hp" << std::endl;
	}

	if (setMP)
	{
		std::cout << "设置mp" << std::endl;
	}

	if (setGold)
	{
		std::cout << "设置gold" << std::endl;
	}


	std::cout << "next code...." << std::endl;
}
```


## AA脚本片段
```
[ENABLE]
assert(address,bytes)
alloc(newmem,$1000)

label(code)
label(return)

label(pPlayer)
label(pFeaturesStatus)

newmem:
  pushf
  push ecx

  mov [pPlayer],eax

  mov cl,byte ptr [pFeaturesStatus]
  test cl,cl
  je @f
  mov ecx,[eax+88]
  mov [eax+84],ecx

@@:
  mov cl,byte ptr [pFeaturesStatus+1]
  test cl,cl
  je @f
  mov [eax+C0],#100

@@:
  mov cl,byte ptr [pFeaturesStatus+2]
  test cl,cl
  je code
  mov [eax+B8],#99999

code:
  pop ecx
  popf
  mov eax,[eax+00000084]
  jmp return

align 10 CC
pPlayer:
  dd 0
  align 4
pFeaturesStatus:
  db 1 // 锁血?
  db 1 // 满备用HP?
  db 1 // 99999金币?
  align 4

address:
  jmp newmem
  nop
return:
registerSymbol(pPlayer pFeaturesStatus)

[DISABLE]
address:
  db bytes
unregisterSymbol(pPlayer pFeaturesStatus)
dealloc(newmem)
```