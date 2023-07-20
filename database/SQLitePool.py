import threading
from queue import Queue
import sqlite3
import time

try:
    from _queue import Empty
except ImportError:
    class Empty(Exception):
        'Exception raised by Queue.get(block=0)/get_nowait().'
        pass

class SQLResult(object):
    def __init__(self,task,connection_timeout:int=5) -> None:
        self.task = task
        self.timeout = False
        self.result = None
        self.done = False
        self.start_time = time.time()
        self.connection_timeout = connection_timeout

    def get(self):
        '查询结果为[...],CRUD操作失败均返回False,包括超时'
        while not self.done:
            if time.time() - self.start_time > self.connection_timeout:
                print(f'Error: Connection {self.task} timeout.')
                self.timeout = True
                return False
        return self.result

class SQLitePool(object):
    """
    创建策略：创建了几个连接对象，以便使用时能从连接池中获取。
    连接管理策略：当客户请求数据库连接时，首先查看连接池中是否有空闲连接，如果存在空闲连接，则将连接分配给客户使用；如果没有空闲连接，则查看当前所开的连接数是否已经达到最大连接数，如果没达到就重新创建一个连接给请求的客户；如果达到就按设定的最大等待时间进行等待，如果超出最大等待时间，则抛出异常给客户。
当客户释放数据库连接时，先判断该连接的引用次数是否超过了规定值，如果超过就从连接池中删除该连接，否则保留为其他客户服务。
    关闭策略：当应用程序退出时，关闭连接池中所有的连接，释放连接池相关的资源，
    """
    def __init__(self,db_fp:str,min_connections:int=5,max_connections:int=64,temp_thread_repeat_times:int=10,connection_timeout:int=5,reconnect_retries:int=3) -> None:
        self.db_fp = db_fp
        self.max_connections = max_connections
        self.min_connections = min_connections
        self.temp_thread_repeat_times = temp_thread_repeat_times
        self.connection_timeout = connection_timeout
        self.reconnect_retries = reconnect_retries
        # 初始化连接池
        self.task_queue = Queue()
        self.num_workers = min_connections
        self.pools = []
        self.isruning = [False]*min_connections 
        def _create_task_handler(idx):
            conn = sqlite3.connect(db_fp)
            cursor = conn.cursor()
            while True:
                self.isruning[idx] = False
                result = self.task_queue.get(block=True,timeout=None)
                self.isruning[idx] = True
                if result.timeout:
                    self.task_queue.task_done()
                    result.done = True
                    continue
                try:
                    cursor.execute(result.task)
                    result.result = cursor.fetchall()
                    conn.commit()
                except Exception as e:
                    print(e)
                    result.result = False
                result.done = True
                self.task_queue.task_done()
        for _ in range(min_connections):
            thread = threading.Thread(target=_create_task_handler,args=(_,))
            thread.start()
            self.pools.append(thread)
        assert self.num_workers == min_connections
        print(f'activated thread:{threading.active_count()}')
         
            
    
    def _create_temp_task_handler(self):
        conn = sqlite3.connect(self.db_fp)
        cursor = conn.cursor()
        n = 0
        while True:

            if n == self.temp_thread_repeat_times:
                break
            result = self.task_queue.get(block=True,timeout=None)
            if result.timeout:
                self.task_queue.task_done()
                result.done = True
                continue
            try:
                cursor.execute(result.task)
                result.result = cursor.fetchall()
                conn.commit()
            except Exception as e:
                print(e)
                result.result = False
            result.done = True
            self.task_queue.task_done()
            n+=1
        conn.close()
        self.num_workers-=1


    def put(self,task:str):
        future_result = SQLResult(task,connection_timeout=self.connection_timeout)
        self.task_queue.put(future_result)
        if self.num_workers < self.max_connections and all(self.isruning):
            thread = threading.Thread(target=self._create_temp_task_handler,args=())
            thread.start()
            self.num_workers+=1
            assert self.num_workers <= self.max_connections and self.num_workers >= self.min_connections
        return future_result
    

    def close(self):
        self.task_queue.join()
        try:
            for thread in self.pools:
                thread.kill()
        except Exception:
            print("Close Error: Failed to close the thread.")