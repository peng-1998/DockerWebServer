# Http Enterpoint

## POST /api/auth/login
- headers
  - Content-Type: application/json

- body
    ```json
    {
        "account": "***",
        "password": "***"
    }
    ```
- Response
  - 200
    ```json
    {
        "access_token": "***"
    }
    ```
  - 401 [mean password error]
  - 404 [mean account not found]


## POST /api/auth/register
- headers
  - Content-Type: application/json
- body
    ```json
    {
        "account": "***",
        "password": "***",
        ...
    }
    ```

- Response
  - 401
    - {"message": "Token Not Found"}
    - {"message": "Invalid token"}
    - {"message": "User not include"}
    - {"message": "Token expired"}

## token verify
Response
  - 401 [mean token error]

## POST or GET /api/auth/logout
- headers
  - Authorization: [access_token]

- Response
  - 200
  - token verify error

## GET /api/auth/session
- headers
  - Authorization: [access_token]

- Response
  - 200 {"access_token": "new token"}
  - token verify error

## POST /api/user/set_profile
- headers
    - Authorization: [access_token]
    - Content-Type: application/json
- body
  ```json
  {
    "nickname or email or phone": "***",
    ...
  }
- Response
  - 200
  - token verify error

## PSET /api/user/set_photo
- headers
    - Authorization: [access_token]
    - Content-Type: multipart/form-data
    - file: [file]
    - with_file: "true" or "false"
- body
    binary file

- Response
  - 200
  - token verify error

## GET /api/user/info
- headers
  - Authorization: [access_token]

- Response
  - 200 {id:...,nickname: ..., email: ..., phone: ..., photo: ...}
  - token verify error


## GET /api/machines/info
- headers
  - Authorization: [access_token]

- Response
  - 200 [{id:..., ip:...,gpu:...,cpu:...,memory:...,disk:...,online:...},...]
  - token verify error

## PSET /api/task/request
- headers
  - Authorization: [access_token]
  - Content-Type: application/json
- body
    ```json
    {
        "machine_id": "...",
        "containername": "...",
        "command": "...",
        "duration": "duration",
        "gpu_count": 1 or "gpus":[1,2]
    }
    ```
- Response
  - 200
  - token verify error

## POST /api/task/cancel
- headers
  - Authorization: [access_token]
  - Content-Type: application/json
- body
    ```json
    {
        "task_id": 1564,
        "machine_id": "..."
    }
    ```
- Response
  - 200
  - token verify error

## GET /api/task/user/<account>
- headers
  - Authorization: [access_token]
- Response
  - 200 [{taskId:1564, machine_id:..., containername:..., command:..., duration:..., gpu_count:..., gpu_ids:...,start_time:...},...]
  - token verify error

## GET /api/task/machine/<machine_id>
- headers
  - Authorization: [access_token]
- Response
  - 200 [{taskId:1564, user_id:...., containername:..., command:..., duration:..., gpu_count:..., gpu_ids:...,start_time:...},...]

# WebSocket 

WebSocket packet format



```
{
    "type": "container" ,
    "data": {
        "opt": create 
        "image_id":...
        "port":...
    }
}
{
    "type": "auth"
    "data": {
        "token":...
    }
}
```