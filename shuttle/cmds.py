import shuttle_middle
import shuttle_backend as sb

GL_COLOR_BUFFER_BIT = sb.GL_COLOR_BUFFER_BIT
GL_DEPTH_BUFFER_BIT = sb.GL_DEPTH_BUFFER_BIT
GL_DEPTH_TEST = sb.GL_DEPTH_TEST
GL_LEQUAL = sb.GL_LEQUAL
GL_MODELVIEW = sb.GL_MODELVIEW
GL_NICEST = sb.GL_NICEST
GL_PERSPECTIVE_CORRECTION_HINT = sb.GL_PERSPECTIVE_CORRECTION_HINT
GL_PROJECTION = sb.GL_PROJECTION
GL_QUADS = sb.GL_QUADS
GL_SMOOTH = sb.GL_SMOOTH
GL_TRIANGLES = sb.GL_TRIANGLES

def _c(cmdseq):
	shuttle_middle._cmd_queue.append(cmdseq)

def glBegin(a):
	_c((sb.CMD_glBegin,))

def glClearColor(a, b, c, d):
	_c((sb.CMD_glClearColor, a, b, c, d,))

def glClearDepth(a):
	_c((sb.CMD_glClearDepth, a,))

def glClear(a):
	_c((sb.CMD_glClear, a,))

def glColor3f(a, b, c):
	_c((sb.CMD_glColor3f, a, b, c,))

def glDepthFunc(a):
	_c((sb.CMD_glDepthFunc, a,))

def glEnable(a):
	_c((sb.CMD_glEnable, a,))

def glHint(a, b):
	_c((sb.CMD_glHint, a, b,))

def glLoadIdentity():
	_c((sb.CMD_glLoadIdentity,))

def glMatrixMode(a):
	_c((sb.CMD_glMatrixMode, a,))

def glPopMatrix():
	_c((sb.CMD_glPopMatrix,))

def glPushMatrix():
	_c((sb.CMD_glPushMatrix,))

def glRotatef(a, b, c, d):
	_c((sb.CMD_glRotatef, a, b, c, d,))

def glScalef(a, b, c, d):
	_c((sb.CMD_glScalef, a, b, c, d,))

def glShadeModel(a):
	_c((sb.CMD_glShadeModel, a,))

def glTranslatef(a, b, c, d):
	_c((sb.CMD_glScalef, a, b, c, d,))

def glVertex3f(a, b, c):
	_c((sb.CMD_glVertex3f, a, b, c,))

def glViewport(a, b, c, d):
	_c((sb.CMD_glVertex3f, a, b, c, d,))

def gluPerspective(a, b, c, d):
	_c((sb.CMD_gluPerspective, a, b, c, d,))
