import shuttle_middle, shuttle_backend

def glPushMatrix():
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_GL_PUSH_MATRIX,))

def glPopMatrix():
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_GL_POP_MATRIX,))

def glColor3f(r, g, b):
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_GL_COLOR_3F, r, g, b))

def glVertex3f(x, y, z):
	shuttle_middle._cmd_queue.append((shuttle_backend.CMD_GL_VERTEX_3F, x, y, z))
