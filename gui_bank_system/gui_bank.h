#pragma once

// 主系统调用此函数进入银行页面；用户点击返回后函数结束。
// 调用前：创建 1200×760 的 EasyX 窗口，开启批量绘图，设置银行数据工作目录。
// 本函数不创建或关闭窗口；accounts.txt、transactions.txt 从当前工作目录读写。
void runBankGui();
