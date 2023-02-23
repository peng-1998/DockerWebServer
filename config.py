
hostName = ""
serverPort = 9999

user_data_path = '/data/' # 用户数据存放路径


# 邮箱登陆信息
mail_login_user = "xxxx@aa.com"  # 邮箱账号
mail_password = "****************" # 邮箱密码
mail_sender = mail_login_user
from_str = 'GPU服务器管理员' # 邮件发送者名称,即发送的邮件显示的发送者
server_host = "smtp.exmail.qq.com" # 邮箱服务器地址，smtp.exmail.qq.com是QQ企业邮箱的服务器地址
server_port = 465   # 邮箱服务器端口号
use_ssl = True # 是否使用SSL
max_resend_time = 5 

administrator_passwd = "********" # 管理员密码，用于添加镜像，等管理操作

# 后台线程检查时间间隔(s)
check_interval = 20

# 调度策略
scheduling_policy = "WNCFNWCF" # FIFO:first in first out, WNCFNWCF:worktime no command first not worktime command first
## WNCFNWCF 参数
worktime_start = 8 # 工作时间开始时间 0 到 23 且 worktime_start < worktime_end
worktime_end = 22   
max_dalay_time = 2 # 最大延迟次数


# 容器创建参数
host_port_start = 10000 # 容器端口范围
host_port_end = 21000   # 容器端口范围
shared_memory_size = "4g" # 容器共享内存大小

max_container_num = 3 # 每个用户的最大容器数量
container_flags = ['a','b','c'] # 容器名后缀,创建的容器名为用户名_后缀