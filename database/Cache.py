from readerwriterlock import rwlock


class InfoCache:
    def __init__(self):
        self.cache = {}
        self.lock = rwlock.RWLockFair()

    def update(self, key: int | str, info: dict):
        with self.lock.gen_wlock():
            self.cache[key] = info

    def get(self, key: int | str):
        with self.lock.gen_rlock():
            return self.cache[key]

    def contains(self, key: int | str):
        with self.lock.gen_rlock():
            return key in self.cache
