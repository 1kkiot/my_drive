                                           Si24R1简单通信示例程序

1. NOACK 通信模式 

发送端程序: TxTest_NOACK
接收端程序：RxTest_NOACK

2. ACK通信模式 
(代码已修改：与收发开机顺序无关）

发送端程序: TxTest_ACK
接收端程序：RxTest_ACK

3.多对一通信模式
配置方案： 多对一通信

4.Si24R1 与NRF24LU1OTP芯片通信例子----NOACK
Si24R1为发送端，NRF24LU1OTP为接收端
源代码目录：Si24R1_NRF24LU1OTP
                              -----TxTest_NOACK：Si24R1发送源代码
                              -----RxTest_NOACK: NRF24LU1OTP 接收源代码

5.ACK通信模式---跳频通信（四个通道）
发送端程序: TxTest_ACK_FP
接收端程序：RxTest_ACK_FP



6.Si24R1 与NRF24LU1OTP芯片通信例子----ACK WITH 5BYTE PAYLOAD
Si24R1为发送端，NRF24LU1OTP为接收端: 2Mbps, 5Byte PAYOLAD with 5Byte ACK Payload
源代码目录：Si24R1_NRF24LU1OTP_ACK
                              -----TxTest_ACK_2M：Si24R1发送源代码
                              -----RxTest_ACK_2M: NRF24LU1OTP 接收源代码




