(window.webpackJsonp=window.webpackJsonp||[]).push([[12],{423:function(e,n,t){e.exports=t.p+"assets/img/ce-tutorial-9.23bf801d.png"},424:function(e,n,t){e.exports=t.p+"assets/img/ce-tutorial-9-2.f39dd6e6.png"},468:function(e,n,t){"use strict";t.r(n);var r=t(54),a=Object(r.a)({},(function(){var e=this,n=e.$createElement,r=e._self._c||n;return r("ContentSlotsDistributor",{attrs:{"slot-key":e.$parent.slotKey}},[r("ul",[r("li",[r("a",{attrs:{href:"https://ganlvtech.github.io/2018/01/25/cheat-engine-tutorial/#step-9-shared-code-pw-31337157",target:"_blank",rel:"noopener noreferrer"}},[e._v("解答地址"),r("OutboundLink")],1)])]),e._v(" "),r("p",[r("img",{attrs:{src:t(423),alt:""}})]),e._v(" "),r("p",[e._v("通过辨别队伍的编号解决问题")]),e._v(" "),r("div",{staticClass:"language- extra-class"},[r("pre",{pre:!0,attrs:{class:"language-text"}},[r("code",[e._v('alloc(newmem,2048)\nlabel(returnhere)\nlabel(originalcode)\nlabel(exit)\nlabel(xxx) // 添加一个新标签\n\nnewmem: //this is allocated memory, you have read,write,execute access\n// 你的代码\ncmp [ebx+04+0c],01 // if [ebx+04+0c] == 01, then => xxx, else => originalcode\nje xxx\n\noriginalcode:\nmov [ebx+04],eax\n\nxxx:\nfldz \n\nexit:\njmp returnhere\n\n"Tutorial-i386.exe"+288D9:\njmp newmem\nreturnhere:\n')])])]),r("p",[e._v("还有种方法，这种方法更使用与大多数游戏，"),r("a",{attrs:{href:"https://www.youtube.com/watch?v=kpSRUJfaT1o&list=PLNffuWEygffbbT9Vz-Y1NXQxv2m6mrmHr&index=21",target:"_blank",rel:"noopener noreferrer"}},[e._v("参考视频"),r("OutboundLink")],1)]),e._v(" "),r("p",[r("img",{attrs:{src:t(424),alt:""}})]),e._v(" "),r("p",[e._v("通过分析结构找到不同的点，然后在进行区分")]),e._v(" "),r("div",{staticClass:"language- extra-class"},[r("pre",{pre:!0,attrs:{class:"language-text"}},[r("code",[e._v("[ENABLE]\n\naobscanmodule(INJECT,Tutorial-i386.exe,89 43 04 D9 EE) // should be unique\nalloc(newmem,$1000)\n\nlabel(code)\nlabel(return)\nlabel(xxx)\n\nnewmem:\n  cmp [ebx+10],1 // 如果等于1，说明不是敌人直接跳过，否则就执行秒杀\n  je xxx\n\ncode:\n  // mov [ebx+04],eax\n  mov [ebx+04],0\n  fldz \n  jmp return\n\nxxx:\n  fldz\n  jmp return\n\nINJECT:\n  jmp newmem\nreturn:\nregistersymbol(INJECT)\n\n[DISABLE]\nINJECT:\n  db 89 43 04 D9 EE\n\nunregistersymbol(INJECT)\ndealloc(newmem)\n")])])]),r("p",[e._v("关于一条指令可能会影响很多值的情况，通过找差异来判断当前处理的为那个值，这些差异可能存在内存偏移中，寄存器中，堆栈中，XMM0-XMM15，"),r("a",{attrs:{href:"https://www.youtube.com/watch?v=5fJFSOPGZyQ&list=PLNffuWEygffbbT9Vz-Y1NXQxv2m6mrmHr&index=44&t=0s",target:"_blank",rel:"noopener noreferrer"}},[e._v("建议观看此视频"),r("OutboundLink")],1)])])}),[],!1,null,null,null);n.default=a.exports}}]);