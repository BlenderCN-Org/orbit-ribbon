VariantDir('buildtmp', 'cpp', duplicate=0)
env = Environment()
env.Program('orbit-ribbon', Glob('buildtmp/*.cpp'), LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image'])
