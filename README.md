# DockerWebServer

主要代码逻辑：钟云鹏

UI：周豪哲



To Do:

1 添加无指令请求在非工作时间的顺延逻辑

2 添加防止无指令请求而死的逻辑

3 添加指定时间段可插队的逻辑

# 更新日志
20220901：
  - 添加无指令请求在非工作时间的顺延逻辑（未实装）
  - 添加工作时间优先执行无指令申请的逻辑（未实装）
  - 添加防止饥饿的逻辑
  - 修改了一系列BUG,这一系列BUG曾导致：某些用户可以看到其他用户的容器并使用,某些用户申请的可执行时间内某些特定用户可以使用GPU
  - 去除了历史申请队列当中的重复项