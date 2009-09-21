import shuttle_backend

_cmd_queue = []

def init():
	shuttle_backend.init()

def flush():
	global _cmd_queue
	shuttle_backend.flush(_cmd_queue)
	_cmd_queue = []
