#include "spdlog/spdlog.h"

int main() 
{
    spdlog::info("欢迎使用spdlog！");
    spdlog::error("带参数的错误信息：{}", 1);
    
    spdlog::warn("数字的简单填充如：{:08d}", 12);
    spdlog::critical("支持整数：{0:d}；十六进制：{0:x}；八进制：{0:o}；二进制：{0:b}", 42);
    spdlog::info("支持浮点数：{:03.2f}", 1.23456);
    spdlog::info("位置参数：{1} {0}..", "也", "支持");
    spdlog::info("{:<30}", "左对齐");
    
    spdlog::set_level(spdlog::level::debug); // 设置全局日志级别为调试
    spdlog::debug("此消息应该显示出来..");    
    
    // 更改日志模式
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [线程 %t] %v");
    
    // 编译时日志级别
    // 注意这不会更改当前日志级别，它只会
    // 根据SPDLOG_ACTIVE_LEVEL在发布代码中移除调用
    SPDLOG_TRACE("带参数的跟踪消息：{}", 42);
    SPDLOG_DEBUG("调试消息");
}