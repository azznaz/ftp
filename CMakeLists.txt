cmake_minimum_required (VERSION 3.24.1)

# 项目信息
project (ftp)

# 指定生成目标
add_executable(cserver Cmd_Handle.cpp server.cpp Ftp_Server.cpp )

target_link_libraries(cserver ACE)