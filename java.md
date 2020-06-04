```
// 只需初始化一次
javaInjectAgent()

// 签名
local object =java_getObjectHandleToAddress(0xAE71C020)
local calss = java_getObjectClass(object)
local signature = java_getClassSignature(calss)

print(signature)
// 查找该类的所有对象
local class = java_findClass('Lzombie/characters/BodyDamage/BodyPart')
local object = java_findAllObjectsFromClass(class)

```