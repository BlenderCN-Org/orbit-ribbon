VariantDir('buildtmp', 'cpp', duplicate=0)
env = Environment()
env.Program('orbit-ribbon', Glob('buildtmp/*.cpp'), LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image', 'zzip', 'boost_filesystem-mt'], CCFLAGS='-Wall -Wextra -pedantic-errors')
