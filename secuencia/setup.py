# -*- coding: iso-8859-1 -*-
from distutils.core import setup, Extension

musicseq = Extension('musicseq',
#                    include_dirs = ['/usr/local/include'],
#                    libraries = ['tcl83'],
#                    library_dirs = ['/usr/local/lib'],
                    sources = ['musicseqmodule.c'])

setup (name = 'MusicSeq',
       version = '0.1',
       description = 'Entrada de datos del tipo secuencia musical',
       author = 'Carlos Gómez',
       ext_modules = [musicseq])
