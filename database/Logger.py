import time


class Logger:

    def __init__(self, filename):
        self.filename = filename

    def __call__(self, message: str):
        with open(self.filename, "a") as f:
            f.write(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()) + " " + message + "\n")
