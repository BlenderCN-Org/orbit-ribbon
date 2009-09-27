VariantDir('buildtmp', 'cpp')
env = Environment()
env.Program('orbit-ribbon', Glob('buildtmp/*.cpp'), LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image'])
