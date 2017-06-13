# from distutils.core import setup
# from Cython.Build import cythonize
# from Cython.Distutils import build_ext
# import distutils.sysconfig
# import platform
#
# # sources = ['SecurityFtdcMdApi.pyx']
# #
# # optional = {}
# #
# # if platform.system() == 'Windows':
# #     optional['include_dirs'] = ['./']
# #     optional['library_dirs'] = ['./']
# #     if '64 bit' in platform.python_compiler():
# #         optional['include_dirs'] = ['./']
# #         optional['library_dirs'] = ['./']
# #
# # argments = dict(name='gmctp',
# #                 sources=sources,
# #                 language='c++',
# #                 libraries=['securitymduserapi'])
# # argments.update(optional)
#
# setup(
#     cmdclass = {'build_ext': build_ext},
#     ext_modules = cythonize(
#     "SecurityFtdcMdApi.pyx",
#     language="c++"))
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [
    Extension("SecurityFtdcMdApi", ["SecurityFtdcMdApi.pyx"],
        include_dirs = ["./"],
        libraries = ["securitymduserapi"],
        library_dirs = ["./lib"])
]

setup(
    name = "pylts",
    ext_modules = cythonize(extensions)
)