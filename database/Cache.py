from readerwriterlock import rwlock


class InfoCache:
    def __init__(self):
        self.cache = {}
        self.lock = rwlock.RWLockFair()

    def update(self, info: dict):
        with self.lock.gen_wlock():
            self.cache[info['machine_id']] = info
        
    def get(self, machine_id: str):
        with self.lock.gen_rlock():
            return self.cache.get(machine_id, None)

