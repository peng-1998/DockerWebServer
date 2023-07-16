import subprocess
import psutil


def get_cpu_info() -> dict:
    cputype            = subprocess.run('cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c', shell=True, stdout=subprocess.PIPE, text=True).stdout
    cputype            = cputype.strip().split(' ')
    cputype            = ' '.join(cputype[1:])
    logical_cpu_count  = psutil.cpu_count(logical=True)
    physical_cpu_count = psutil.cpu_count(logical=False)
    return {'type': cputype, 'logical_cpu_count': logical_cpu_count, 'physical_cpu_count': physical_cpu_count}


def get_memory_info() -> float:
    memory = psutil.virtual_memory()
    return {'total': round(memory.total / (1024.0 * 1024.0 * 1024.0), 2), 'free': round(memory.free / (1024.0 * 1024.0 * 1024.0), 2)}


def get_disk_info() -> dict:
    disk = psutil.disk_usage('/')
    return {'total': round(disk.total / (1024.0 * 1024.0 * 1024.0), 2), 'free': round(disk.free / (1024.0 * 1024.0 * 1024.0), 2)}