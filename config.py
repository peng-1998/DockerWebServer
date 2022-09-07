
hostName = ""
serverPort = 9999

gpu_request_database_save_path = 'data/database/gpu_request_database.json'
docker_images_database_save_path = 'data/database/docker_images.json'
user_info_database_save_path = 'data/database/user_info.json'


mail_login_user = "2021023231@m.scnu.edu.cn"
mail_password = "2RddzpookTYwJGwn"
mail_sender = mail_login_user
from_str = 'GPU服务器管理员'
server_host = "smtp.exmail.qq.com"
server_port = 465
use_ssl = True
max_resend_time = 5

administrator_passwd = "2021023231"

# 后台线程检查时间间隔(s)
check_interval = 20

# 调度策略
scheduling_policy = "WNCFNWCF" # FIFO:first in first out, WNCFNWCF:worktime no command first not worktime command first
## WNCFNWCF 参数
worktime_start = 8 # 工作时间开始时间 0 到 23 且 worktime_start < worktime_end
worktime_end = 22
max_dalay_time = 2


# 容器创建参数
host_port_start = 10000
host_port_end = 21000
shared_memory_size = "4g"
