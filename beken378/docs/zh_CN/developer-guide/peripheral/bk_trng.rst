
:link_to_translation:`en:[English]`

TRNG 使用指南
=====================


概述
-----------------

- TRNG 真随机数发生器，集成在 Beken 芯片内部，通过随机噪声产生真随机数，不依赖于其他模块。
- 用于创建加密操作所需的密钥、初始化向量和随机数。

TRNG 使用方法
------------------

 - 获取随机数，调用 bk_rand() 获取。

