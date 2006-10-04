#!/usr/bin/env python

from distutils.sysconfig import get_python_lib
from distutils.core import setup
import os
import sys
from os.path import join, exists
from time import time

# detect case-insensitive filesystem
case_sensitive_filesystem = True
os.mkdir('FOO')
try:
    f = open('foo', 'w')
except:
    case_sensitive_filesystem = False
else:
    f.close()
    os.remove('foo')
os.rmdir('FOO')
# done detecting case-insensitive filesystem

symlinks=["owlrepository","conan_client","lifter"]
scripts=[]

for s in symlinks:
    script=s+'.py'
    if not os.access(s,os.F_OK):
        os.symlink(script,s)
    scripts.append(script)
    os.chmod(script,0664)

use_scripts=symlinks
if sys.argv[1:2]==['sdist'] or not case_sensitive_filesystem:
    use_scripts=scripts

__version__='0.0.1'
setup(
        name = 'conan',
        version = __version__,
        description = "RDF/OWL document repository with inference",
        author = "Alessio Carenini",
        author_email = "carenini@gmail.com",
        license="GPL v.2",
        url = "http://veggente/berlios.de",
        packages= [''],
        scripts = use_scripts,
        )

for s in symlinks:
    if os.path.islink(s):
        os.remove(s)
