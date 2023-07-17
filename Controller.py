from database import BaseDB, InfoCache
from queue import Queue
from flask import current_app


def data_handler_gpus(info: dict, machine_id: int | str):
    """deal with the gpu info sent by the machine

    Args:
        info (dict): the gpu info sent by the machine. e.g. {'0': {'type': 'Nvidia RTX 3060Ti', 'memory_total': 10240, 'memory_used': 2048, 'utilization': 0.96 }, ...}
        machine_id (int | str): the id of the machine
    """
    current_app.config["gpus_cache"].update(machine_id, info)


def data_handler_image(data: dict, machine_id: int | str):
    if data["status"] == "failed":
        current_app.config["error_logger"](
            f'{machine_id} {data["opt"]} image:{data["image"]} failed, error info: {data["error"]}'
        )
        current_app.config["info_logger"](
            f'{machine_id} {data["opt"]} image:{data["image"]} failed'
        )
        return
    current_app.config["info_logger"](
        f'{machine_id} {data["opt"]} image:{data["image"]} success'
    )
    if data["opt"] == "pull":
        ...
    elif data["opt"] == "remove":
        ...


def data_handler_container(data: dict, machine_id: int | str):
    user_id = data["user_id"]
    massage_cache: InfoCache = current_app.config["massage_cache"]
    if not massage_cache.contains(user_id):
        massage_queue: Queue = Queue()
    else:
        massage_queue: Queue = massage_cache.get(user_id)
    massage_queue.put(
        {"type": "container", "opt": data["opt"], "status": data["status"]}
    )
    massage_cache.update(user_id, massage_queue)

    db: BaseDB = current_app.config["db"]
    current_app.config["info_logger"](
        f'{machine_id} {data["opt"]} container:{data["container"]} success'
    )
    if data["opt"] == "create":
        if data["status"] == "failed":
            current_app.config["error_logger"](
                f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]} create args: {json.dumps(data["original_data"])}'
            )
            current_app.config["info_logger"](
                f'{machine_id} {data["opt"]} container:{data["container"]} failed'
            )
            return
        current_app.config["info_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} success'
        )
        original_data = data["original_data"]
        showname = original_data["name"].split("_")[-1][1:]
        portlist = [
            [int(k.split("/")[0]), v] for k, v in original_data["ports"].items()
        ]
        image_id = db.get_image(
            search_key={"imagename": original_data["image"]}, return_key=["id"]
        )[0]["id"]
        db.insert_container(
            {
                "showname": showname,
                "containername": data["name"],
                "userid": user_id,
                "machineid": machine_id,
                "portlist": portlist,
                "image": image_id,
                "runing": False,
            }
        )
        return
    if data["status"] == "failed":
        current_app.config["error_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]}'
        )
        current_app.config["info_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} failed'
        )
        return
    if data["opt"] == "remove":
        db.delete_container(
            search_key={"containername": data["container"], "machineid": machine_id}
        )
    elif data["opt"] in ["start", "stop", "restart"]:
        db.update_container(
            search_key={"containername": data["container"], "machineid": machine_id},
            update_key={"running": "start" in data["opt"]},
        )


def data_handler_container(data: dict, machine_id: int | str):
    user_id = data["user_id"]
    massage_cache: InfoCache = current_app.config["massage_cache"]
    if massage_cache.contains(user_id):
        massage_queue: Queue = Queue()
    else:
        massage_queue: Queue = massage_cache.get(user_id)
    massage_queue.put(
        {"type": "container", "opt": data["opt"], "status": data["status"]}
    )
    massage_cache.update(user_id, massage_queue)

    db: BaseDB = current_app.config["db"]
    current_app.config["info_logger"](
        f'{machine_id} {data["opt"]} container:{data["container"]} success'
    )
    if data["opt"] == "create":
        if data["status"] == "failed":
            current_app.config["error_logger"](
                f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]} create args: {json.dumps(data["original_data"])}'
            )
            current_app.config["info_logger"](
                f'{machine_id} {data["opt"]} container:{data["container"]} failed'
            )
            return
        current_app.config["info_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} success'
        )
        original_data = data["original_data"]
        showname = original_data["name"].split("_")[-1][1:]
        portlist = [
            [int(k.split("/")[0]), v] for k, v in original_data["ports"].items()
        ]
        image_id = db.get_image(
            search_key={"imagename": original_data["image"]}, return_key=["id"]
        )[0]["id"]
        db.insert_container(
            {
                "showname": showname,
                "containername": data["name"],
                "userid": user_id,
                "machineid": machine_id,
                "portlist": portlist,
                "image": image_id,
                "runing": False,
            }
        )
        return
    if data["status"] == "failed":
        current_app.config["error_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} failed, error info: {data["error"]}'
        )
        current_app.config["info_logger"](
            f'{machine_id} {data["opt"]} container:{data["container"]} failed'
        )
        return
    if data["opt"] == "remove":
        db.delete_container(
            search_key={"containername": data["container"], "machineid": machine_id}
        )
    elif data["opt"] in ["start", "stop", "restart"]:
        db.update_container(
            search_key={"containername": data["container"], "machineid": machine_id},
            update_key={"running": "start" in data["opt"]},
        )


def data_handler_task(data: dict, machine_id: int | str):
    if data["status"] == "failed" and data["opt"] == "run":
        current_app.config["error_logger"](
            f'{machine_id} {data["opt"]} task failed, error info: {data["error"]}'
        )
        current_app.config["info_logger"](f'{machine_id} {data["opt"]} task failed')
        current_app.config["wq"].finish_task(machine_id, data["task_id"])
        return


# all functions get two parameters, the first is the data (json), the second is the machine_id
data_handler_funcs = {
    "gpus": data_handler_gpus,
    "image": data_handler_image,
    "container": data_handler_container,
    "task": data_handler_task,
}
# data: {'type': str, 'data': dict}
data_handler = lambda data, machine_id: data_handler_funcs[data["type"]](
    data["data"], machine_id
)
disconnect_handler = lambda machine_id: current_app.config["wq"].remove_machine(
    machine_id
)
