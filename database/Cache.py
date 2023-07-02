from readerwriterlock import rwlock


class InfoCache:

    def __init__(self):
        self.cache = {}
        self.lock = rwlock.RWLockFair()

    def update(self, machine_id: int | str, info: dict):
        with self.lock.gen_wlock():
            self.cache[machine_id] = info

    def get(self, machine_id: int | str):
        with self.lock.gen_rlock():
            return self.cache[machine_id]