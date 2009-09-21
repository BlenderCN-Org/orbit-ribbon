(
CMD_GL_PUSH_MATRIX, CMD_GL_POP_MATRIX, CMD_GL_COLOR_3F, CMD_GL_VERTEX_3F
) = range(4)

def _cmd_gl_push_matrix():
	print "PUSH MATRIX"

def _cmd_gl_pop_matrix():
	print "POP MATRIX"

def _cmd_gl_color_3f(float r, float g, float b):
	print "COLOR 3F %f %f %f" % (r, g, b)

def _cmd_gl_vertex_3f(float x, float y, float z):
	print "VERTEX 3F %f %f %f" % (x, y, z)

_cmd_map = {
	CMD_GL_PUSH_MATRIX : _cmd_gl_push_matrix,
	CMD_GL_POP_MATRIX : _cmd_gl_push_matrix,
	CMD_GL_COLOR_3F : _cmd_gl_color_3f,
	CMD_GL_VERTEX_3F : _cmd_gl_vertex_3f,
}

def init():
	pass

def flush(cmdlist):
	for cmdseq in cmdlist:
		_cmd_map[cmdseq[0]](*(cmdseq[1:]))
