import asyncio

class AsyncDict:
    def __init__(self):
        self._data = {}
        self._lock = asyncio.Lock()

    async def __getitem__(self, key):
        async with self._lock:
            return self._data[key]

    async def __setitem__(self, key, value):
        async with self._lock:
            self._data[key] = value

    async def __delitem__(self, key):
        async with self._lock:
            del self._data[key]

    def __len__(self):
        return len(self._data)

    async def keys(self):
        async with self._lock:
            return list(self._data.keys())

    async def values(self):
        async with self._lock:
            return list(self._data.values())

    async def items(self):
        async with self._lock:
            return list(self._data.items())


class AsyncQueue(asyncio.Queue):
    pass

