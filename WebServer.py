import os

import yaml
from flask import g
from create_app import create_app
from Controller import data_handler, disconnect_handler

import communication
import database.SqliteDB
import dispatch.SchedulingStrategy as SS

from communication import BaseServer, DockerController
from database import BaseDB, InfoCache
from dispatch import WaitQueue


os.chdir(os.path.dirname(__file__))

app = create_app()


def connect_handler(info: dict, ip: str) -> dict:
    """deal with the first data sent by the machine

    Args:
        info (dict): the info sent by the machine. e.g.
            {'machine_id': 1,
                'gpus': {0: {'type': 'NVIDIA GeForce RTX 3060', 'memory_total': 12288.0, 'memory_used': 3187.25, 'utilization': 45}},
                'cpu': {'type': ' Intel(R) Core(TM) i5-10400F CPU @ 2.90GHz', 'logical_cpu_count': 12, 'physical_cpu_count': 6},
                'memory': {'total': 62.71, 'free': 30.26},
                'disk': {'total': 195.8, 'free': 112.51},
                'url': 'www.sdasds.com'}
        ip (str): the ip of the machine

    Returns:
        dict: the info return to Messenger
    """
    db: BaseDB = app.config["db"]
    machine_id = info["machine_id"]
    del info["machine_id"]
    gpus = info["gpus"]
    machines_list = db.get_machine(search_key={"id": machine_id})
    if len(machines_list) == 0:
        db.insert_machine(machine={"id": machine_id, "ip": ip, "machine_info": info})
    else:
        db.update_machine(
            search_key={"id": machine_id}, update_key={"machine_info": info, "ip": ip}
        )
    app.config["wq"].new_machine(machine_id, {i: True for i in range(len(gpus))})
    containers = info["containers"]
    for container in containers:
        db.update_container(
            search_key={"containername": container["name"], "machineid": machine_id},
            update_key={"running": container["running"]},
        )
    return {"machine_id": machine_id}


def run_finish_mail(task: dict):
    return "", ""


def run_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = app.config["messenger"]
    task["opt"] = "run"
    messenger.send({"type": "task", "data": task}, machine_id)
    if hasattr(app.config, "mail"):
        user_id = task["user_id"]
        user = app.config["db"].get_user(search_key={"id": user_id}, return_key=[""])[0]
        if user["email"] != "":
            app.config["mail"].append(
                user["email"], user["nickname"], *run_finish_mail(task)
            )


def finish_handler(machine_id: int | str, task: dict) -> None:
    messenger: BaseServer = app.config["messenger"]
    task["opt"] = "finish"
    messenger.send({"type": "task", "data": task}, machine_id)
    if hasattr(app.config, "mail"):
        user_id = task["user_id"]
        user = app.config["db"].get_user(search_key={"id": user_id}, return_key=[""])[0]
        if user["email"] != "":
            app.config["mail"].append(
                user["email"], user["nickname"], *run_finish_mail(task)
            )


# 初始化
with app.app_context():
    with open("configs/WebServerConfig.yaml") as f:
        configs = yaml.load(f, Loader=yaml.FullLoader)
    app.config["configs"] = configs
    if configs["Components"]["Logger"]["enable"]:
        from database import Logger

        app.config["info_logger"] = Logger(**configs["Components"]["Logger"]["args"])
    else:
        app.config["info_logger"] = print
    if configs["Components"]["ErrorLogger"]["enable"]:
        from database import Logger

        app.config["error_logger"] = Logger(
            **configs["Components"]["ErrorLogger"]["args"]
        )
    else:
        app.config["error_logger"] = print

    app.config["repository"] = configs["Docker"]["repository"]
    DB_Class = getattr(database, configs["Database"]["Class"])
    app.config["db"]: BaseDB = DB_Class(db_path=configs["Database"]["db_path"])
    Scheduler_Class = getattr(SS, configs["Dispatch"]["Class"])
    app.config["wq"] = WaitQueue(
        Scheduler_Class(**configs["Dispatch"]["args"]),
        run_handler,
        app.config["info_logger"],
    )
    app.config["wq"].start()
    Messenger_Class = getattr(
        communication, configs["Components"]["WebMessenger"]["Class"]
    )
    app.config["messenger"]: BaseServer = Messenger_Class(
        **configs["Components"]["WebMessenger"]["args"],
        data_handler=data_handler,
        connect_handler=connect_handler,
        disconnect_handler=disconnect_handler,
    )
    app.config["messenger"].start()  # start the messenger thread
    app.config["max_task_id"] = 0
    app.config["gpus_cache"] = InfoCache()
    app.config["massage_cache"] = InfoCache()
    app.config["docker"] = DockerController()
    if configs["Components"]["Mail"]["enable"]:
        import communication.MailBox as MB

        Mail_Class = getattr(MB, configs["Components"]["Mail"]["Class"])
        app.config["mail"] = Mail_Class(
            **configs["Components"]["Mail"]["args"], logger=app.config["info_logger"]
        )



if __name__ == "__main__":
    app.run(host="0.0.0.0", port=10000, debug=False, threaded=True)
    # 目前有个bug,如果以debug模式运行,会导致套接字被定义两次,导致第二次无法绑定端口
