
:link_to_translation:`en:[English]`

==================
SPI 使用指南
==================



概述
==================


SPI (Serial Peripheral Interface）
----------------------------------

- SPI是一种高速、全双工、同步的通信总线，被广泛应用在ADC、Flash等与MCU的通信过程中.

- BK7238集成了一个SPI接口，可以在主模式或从模式下操作。SPI接口允许主模式下的时钟频率高达30 MHz，从模式下的频率高达20 MHz。
- 可配置的8位或16位数据宽度,支持4线和3线模式（无CSN引脚）
- 具有DMA功能的64-depth RX FIFO和64-depthTX FIFO。
- 接收数据可以在时钟信号的上升沿或下降沿被锁存。可以首先通过MSB或LSB来设置发送数据。


.. figure:: ../../_static/spi_connection.png
    :align: center
    :alt: SPI Connection
    :figclass: align-center

    SPI Connection


上图为 SPI 硬件连接，其中：
 
 - SCK：时钟信号, 提供传输数据的时钟，用来同步数据传输；
 - MOSI：(Master Output, Slave Input)，用于 master 向 slave 传输数据；
 - MISO：(Master Input, Slave Output)，用于 slave 向 master 传输数据；
 - CSN：片选信号，master 选择对应的 slave 进行传输数据，低电平有效


SPI模式
------------------

SPI 有四种通讯模式，他们的主要依靠 SPI 时钟的极性和相位来区别。

+----------+------+------+--------------+--------------+
| SPI 模式 | CPOL | CPHA | SCK 初始电平 |  采样时钟沿  |
+==========+======+======+==============+==============+
|    0     |  0   |   0  |    低电平    | 第一个时钟沿 |
+----------+------+------+--------------+--------------+
|    1     |  0   |   1  |    低电平    | 第二个时钟沿 |
+----------+------+------+--------------+--------------+
|    2     |  1   |   0  |    高电平    | 第一个时钟沿 |
+----------+------+------+--------------+--------------+
|    3     |  1   |   1  |    高电平    | 第二个时钟沿 |
+----------+------+------+--------------+--------------+

我们常用的是模式0和模式3，因为它们都是在上升沿采样数据，不用去在乎时钟的初始电平是什么，只要在上升沿采集数据就行。
bk7238 SPI 支持 SPI 的四种模式.

SPI时序
------------------

.. figure:: ../../_static/spi_timing.png
    :align: center
    :alt: SPI Timing
    :figclass: align-center

    SPI Timing


上图为 master 传输 0x56(0b0101 0110) 数据给 slave 时序(CPOL = 1, CPHA = 1，MSB first)
 
 - CS 拉低，选中对应的 slave
 - 在每个 SCK 时钟周期，MOSI 输出对应的电平，先输出数据的高 bit
 - slave 会在每个时钟周期的上升沿读取 MOSI 上的电平




