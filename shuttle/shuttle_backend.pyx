cdef extern from "SDL.h":
	void SDL_Quit()
	void SDL_GL_SwapBuffers()
	void SDL_GL_SetAttribute(int, int)
	void SDL_SetVideoMode(int, int, int, int)

cdef extern from "gl.h":
	void glBegin(int)
	void glClearColor(float, float, float, float)
	void glClearDepth(float)
	void glClear(int)
	void glColor3f(float, float, float)
	void glDepthFunc(int)
	void glEnable(int)
	void glHint(int, int)
	void glLoadIdentity()
	void glMatrixMode(int)
	void glPopMatrix()
	void glPushMatrix()
	void glRotatef(float, float, float, float)
	void glScalef(float, float, float)
	void glShadeModel(int)
	void glTranslatef(float, float, float)
	void glVertex3f(float, float, float)
	void glViewport(int, int, int, int)

cdef extern from "glu.h":
	void gluPerspective(float, float, float, float)

(
	CMD_glBegin,
	CMD_glClearColor,
	CMD_glClearDepth,
	CMD_glClear,
	CMD_glColor3f,
	CMD_glDepthFunc,
	CMD_glEnable,
	CMD_glHint,
	CMD_glLoadIdentity,
	CMD_glMatrixMode,
	CMD_glPopMatrix,
	CMD_glPushMatrix,
	CMD_glRotatef,
	CMD_glScalef,
	CMD_glShadeModel,
	CMD_glTranslatef,
	CMD_glVertex3f,
	CMD_glViewport,
	
	CMD_gluPerspective,
) = range(19)

def _impl_glBegin(int a): glBegin(a)
def _impl_glClearColor(float a, float b, float c, float d): glClearColor(a, b, c, d)
def _impl_glClearDepth(float a): glClearDepth(a)
def _impl_glClear(int a): glClear(a)
def _impl_glColor3f(float a, float b, float c): glColor3f(a, b, c)
def _impl_glDepthFunc(int a): glDepthFunc(a)
def _impl_glEnable(int a): glEnable(a)
def _impl_glHint(int a, int b): glHint(a, b)
def _impl_glLoadIdentity(): glLoadIdentity()
def _impl_glMatrixMode(int a): glMatrixMode(a)
def _impl_glPopMatrix(): glPopMatrix()
def _impl_glPushMatrix(): glPushMatrix()
def _impl_glRotatef(float a, float b, float c, float d): glRotatef(a, b, c, d)
def _impl_glScalef(float a, float b, float c): glScalef(a, b, c)
def _impl_glShadeModel(int a): glShadeModel(a)
def _impl_glTranslatef(float a, float b, float c): glTranslatef(a, b, c)
def _impl_glVertex3f(float a, float b, float c): glVertex3f(a, b, c)
def _impl_glViewport(int a, int b, int c, int d): glViewport(a, b, c, d)

def _impl_gluPerspective(float a, float b, float c, float d): gluPerspective(a, b, c, d)

_cmd_map = {
	CMD_glBegin: _impl_glBegin,
	CMD_glClearColor: _impl_glClearColor,
	CMD_glClearDepth: _impl_glClearDepth,
	CMD_glClear: _impl_glClear,
	CMD_glColor3f: _impl_glColor3f,
	CMD_glDepthFunc: _impl_glDepthFunc,
	CMD_glEnable: _impl_glEnable,
	CMD_glHint: _impl_glHint,
	CMD_glLoadIdentity: _impl_glLoadIdentity,
	CMD_glMatrixMode: _impl_glMatrixMode,
	CMD_glPopMatrix: _impl_glPopMatrix,
	CMD_glPushMatrix: _impl_glPushMatrix,
	CMD_glRotatef: _impl_glRotatef,
	CMD_glScalef: _impl_glScalef,
	CMD_glShadeModel: _impl_glShadeModel,
	CMD_glTranslatef: _impl_glTranslatef,
	CMD_glVertex3f: _impl_glVertex3f,
	CMD_glViewport: _impl_glViewport,

	CMD_gluPerspective: _impl_gluPerspective,
}

def init():
	pass

def flush(cmdlist):
	for cmdseq in cmdlist:
		_cmd_map[cmdseq[0]](*(cmdseq[1:]))
