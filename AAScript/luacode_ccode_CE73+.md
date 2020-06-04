`{$LUACODE}`和`{$CCODE}`：

它们的功能类似于`{$LUA}`块，因为它需要以`{$ASM}`结尾，但与`{$LUA}`不同的是，它确实在该位置汇编代码

LuaCode在CE的上下文中运行，因此您可以访问ce的所有lua代码，但CCode在目标进程中本机运行

它们可以使用以下参数进行寄存器到参数的转换：（parametername=register）

## ccode
```
// 定义一次的c函数
{$c}
int valuehelper(int x)
{
  if(x < 90)
   return 100;
  else return x -1;
}
{$asm}

newmem:
  mov eax, [rbx+000007F8]

// 嵌入c代码
{$ccode value=eax value2=ebx}
value=valuehelper(value);
{$asm}
```

## luacode
```
// 定义一次的lua函数
{$lua}
if syntaxcheck then return end

function valuehelper(x)
  if x &lt; 90 then
    return 100
  else
    return x - 1
  end
end
{$asm}

newmem:
  mov eax, [rbx+000007F8]

// 嵌入lua代码
{$luacode value=eax}
value=valuehelper(value)
{$asm}
```

## float
```
// xmm1.0F: xmm1寄存器第0个float
//{$ccode fvalue=xmm-1.0F}
//fvalue = 30.0F;
//{$asm}

// 或者取整个xmm1寄存器
{$ccode fvalue=xmm1}
fvalue.f0 = 50.0F;
{$asm}
```

## double
```
//xmm0.0D : 获取xmm0寄存器第0个double
//{$ccode dvalue=xmm0.0D}
//dvalue = 50.0;
//{$asm}

{$ccode dvalue=xmm0}
dvalue.d0 = 20.0;
{$asm}
```


See alse:
  - https://forum.cheatengine.org/viewtopic.php?p=5774456#5774456
  - [CE74_code_luacode_demo](https://github.com/januwA/game-reversed-study/blob/master/AAScript/CE74_code_luacode_demo.CT)