hostName = ""
serverPort = 9999

gpu_request_database_save_path = 'data/database/gpu_request_database.json'
docker_images_database_save_path = 'data/database/docker_images.json'
user_info_database_save_path = 'data/database/user_info.json'

# mail_user = "lxr_team_server"
# mail_pass = "URTTHSZKKHFXPBLD"
mail_user = "2021023231"
mail_pass = "2RddzpookTYwJGwn"

# 凌晨时间段 10pn-8am 如果没有运行指令则将GPU权限顺延
# 特殊情况，1.该用户是唯一的申请者，2.所有用户都没有设置指令：插入一个空等待指令
