import shuttle_middle, shuttle_backend

def glColor3f(a, b, c):
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_glColor3f, a, b, c))

def glPopMatrix():
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_glPopMatrix,))

def glPushMatrix():
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_glPushMatrix,))

def glVertex3f(a, b, c):
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_glVertex3f, a, b, c))
