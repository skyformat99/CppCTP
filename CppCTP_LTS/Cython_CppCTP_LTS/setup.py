from distutils.core import setup
from Cython.Build import cythonize
from Cython.Distutils import build_ext
import distutils.sysconfig
import platform

# sources = ['SecurityFtdcMdApi.pyx']
#
# optional = {}
#
# if platform.system() == 'Windows':
#     optional['include_dirs'] = ['./']
#     optional['library_dirs'] = ['./']
#     if '64 bit' in platform.python_compiler():
#         optional['include_dirs'] = ['./']
#         optional['library_dirs'] = ['./']
#
# argments = dict(name='gmctp',
#                 sources=sources,
#                 language='c++',
#                 libraries=['securitymduserapi'])
# argments.update(optional)

setup(
    cmdclass = {'build_ext': build_ext},
    ext_modules = cythonize(
    "SecurityFtdcMdApi.pyx",                 # our Cython source
    language="c++",             # generate C++ code
    include_dirs=["./"],
    library_dirs=["lib"],
    extra_objects= ['securitymduserapi.lib'],
    libraries=["securitymduserapi"],
    data_files=[('dllfiles', ["lib/securitymduserapi.dll"]),
                ('libfiles', ["lib/securitymduserapi.lib"])]))