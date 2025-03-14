
:link_to_translation:`en:[English]`

BLE APIs
================

----------------------------
BLE 简介
----------------------------
BK7238 是BLE 5.2 协议栈，同时支持master和slave 模式。

----------------------------
BLE API 类别
----------------------------

BLE API 可分为：

 - BLE common interface
 - BLE scan interface
 - BLE ADV interface
 - BLE connect interface

----------------------------------------
编程建议
----------------------------------------

.. important::
   - 使用BLE API 的一些特别注意项:
	   1. 不要在 BLE 事件回调中做太多工作，需将事件通过信号量等方式抛到您自己的应用程序任务处理.
	   2. BK7238 BLE API都是异步执行的,应用程序调用 BLE API ，需要获得 对应的BLE 事件通知。


----------------------------------------
API Reference
----------------------------------------
.. include:: ../../_build/inc/ble_api_5_x.inc



