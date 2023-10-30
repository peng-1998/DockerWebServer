# 数据库
数据库分为四张表：用户、镜像、容器、机器
## 用户
字段包含：id(主键，递增)，账号(account)，昵称(nickname,可选)，密码(password,实际上是密码的哈希值，字符串)，邮箱（可选），手机（可选）
## 镜像
字段包含：id(主键，递增)，镜像名(含tag)，镜像描述，显示名称，创建参数（字典）
## 容器
字段包含：id(主键，递增)，容器名，显示名称，机器id，镜像id，端口，用户id，状态（bool）
## 机器
字段包含：id(主键，递增，由GPU服务器给出)，ip，gpu，硬盘情况，内存情况，cpu情况，状态（bool）,url



# 镜像管理
主服务器负责构建容器并充当镜像仓库，镜像仓库使用docker registry搭建
在主服务器上运行：
```
docker run -d -p [port]:5000 --restart=always --name registry -v /path/to/save:/var/lib/registry registry
```
在主服务器上构建镜像：
```
docker build -t 127.0.0.1:[port]/[镜像名]:[tag] .
```
在主服务器上推送镜像：
```
docker push 127.0.0.1:[port]/[镜像名]:[tag]
```
在主服务器上删除镜像以减少空间占用,除非主服务器也是GPU服务器那就不要删除：
```
docker rmi 127.0.0.1:[port]/[镜像名]:[tag]
```

# docker 需要的功能
## 1. 创建容器
### 1.1 创建参数
## 2. 删除/停止/启动容器
## 3. 容器打包成镜像
## 4. 镜像推送到镜像仓库
## 5. 镜像从镜像仓库拉取
## 6. 构建镜像（主控制节点）
## 7. 删除镜像
## 8. 查询镜像/容器

# 镜像
web服务器建立一个镜像仓库
1. 公共镜像，由管理员在管理界面构建，并应该在构建好后推送到镜像仓库，再由其他服务器拉取（懒拉取？）
2. 客户自定义镜像，只能由已有容器打包成镜像，然后推送到镜像仓库，再由其他服务器拉取并且是使用时才拉取
3. 删除镜像，对于公共镜像，只能由管理员在管理界面删除，对于客户自定义镜像，由客户在管理界面删除
   删除前需要检查是否有依赖镜像和容器，如果有则提示客户删除依赖镜像和容器后再删除本镜像
   因此，数据库当中应该记录镜像的依赖关系
4. 镜像仓库的地址可能为： xxx.xxx.xxx.xxx:port
   而镜像名应该是user/image:tag
   公共镜像的镜像名应该是public/image:tag
   其他服务器的docker应该配置好镜像仓库的地址，这样才能拉取镜像
5. 自定义镜像的创建参数应当继承基础镜像的创建参数


# web api 接口规范
## 变量名
变量名使用下划线分割，例如：user_id
| 变量名 | 含义 | 类型 | 说明 |
|: ----: |: ----: |: ----: |: ----: |
| user_id | 用户id | int | 主键，递增 |
| image_id | 镜像id | int | 主键，递增 |
| container_id | 容器id | int | 主键，递增 |
| machine_id | 机器id | str | 主键，由GPU服务器给出 |
| account | 用户账号 | str | |
| nickname | 用户昵称 | str | 可选 |
| password | 用户密码 | str | 实际上是密码的哈希值 |
| email | 用户邮箱 | str | 可选 |
| phone | 用户手机 | str | 可选 |
| photo | 用户头像 | str | 可选 |
| new_password | 新密码 | str | 实际上是密码的哈希值 |


## web接口
### 登陆
#### request
/api/auth/login POST 
body(json):
```
{
    "account": "xxx",
    "password": "xxx" // 密码的哈希值
}
```
#### response
StatusCode: 200 | 401 (Wrong Password) | 404 (no such user)
200 body(json):
```
{"access_token": "x.x.x"}
```

### 注册
#### request
/api/auth/register POST
body(json):
```
{
    "account": "xxx",
    "password": "xxx" // 密码的哈希值
}
```
#### response
StatusCode: 200 | 409 (user already exists)

### 获取用户信息
#### request
/api/user/get_user/<account> GET
#### response
StatusCode: 200 
body(json):
```
{
    "user_id": 1,
    "account": "xxx",
    "nickname": "xxx",
    "email": "xxx",
    "phone": "xxx",
    "photo": "xxx"
}
```

### 修改用户信息
#### request
/api/user/set_profile POST
body(json):
```
{
   field: key in {"nickname", "email", "phone"}
   value: new value
}
```
#### response
StatusCode: 200


### 修改密码
#### request
/api/user/set_password POST
body(json):
```
{
   "user_id": x,
   "new_password": "xxx",
}
```
#### response
StatusCode: 200

### 修改头像
#### request
/api/user/set_photo/custom POST
header:
```
"user_id": x,
"account": "xxx",
"content-type": "multipart/form-data"; boundary=----xxx(随机字符串)
```
body(binary):
```
----xxx \n
Content-Disposition: form-data; name="photo"; filename="account.png" \n 
Content-Type: image/jpeg \n
photo file data
----xxx \n
```
#### response
StatusCode: 200

#### request
/api/user/set_photo/default POST
body(json):
```
{
   "user_id": x,
   "photo": "abc.png",
}
```
#### response
StatusCode: 200
### 获取所有GPU服务器信息
#### request
/api/machine/info GET
#### response
StatusCode: 200
body(json):
```
[
   {
      machine_id: "xxx",
      ip: "xxx",
      gpu: {0:{"type":"RTX 3060","memory":10240}},
      cpu: {0:{"type":"Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz","cores":8}},
      disk: {"total":1024,"free":512},
      memory: {"total":1024,"free":512},
      online: true
   },
   {
      ...
   }
]
```



## websocket接口