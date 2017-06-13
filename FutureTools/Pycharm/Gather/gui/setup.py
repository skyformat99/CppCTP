from distutils.core import setup
import py2exe
import sys
from urllib.request import urlopen
from urllib.error import HTTPError
from bs4 import BeautifulSoup
import time, datetime
from PipeLine import PipeLine
from FutureTab import FutureTab
from FutureSpider import InfoParser
from Info import Info
from PipeLine import PipeLine
from ToolWindow import MainWindow
from InfoContent import InfoContent
from Ui_ToolWindow import Ui_MainWindow
from Ui_FutureTab import Ui_FutureTab
from Ui_InfoContent import Ui_InfoContent

sys.path.append("D:\CTP\CppCTP\FutureTools\Pycharm\Gather\gui")

class Make_Exe():
    def __init__(self, python_script):
        # this allows to run it with a simple double click.
        sys.argv.append('py2exe')

        py2exe_options = {
            "includes" : ["sip","Ui_InfoContent", "Ui_FutureTab", "Ui_ToolWindow", "ToolWindow", "InfoContent", "PipeLine", "FutureTab", "FutureSpider", "Info"],  # 如果打包文件中有PyQt代码，则这句为必须添加的
            "packages" : ["bs4"],  # additional packages
            "dll_excludes" : ["MSVCP90.dll"],  # 这句必须有，不然打包后的程序运行时会报找不到MSVCP90.dll，如果打包过程中找不到这个文件，请安装相应的库
            "dist_dir" : 'exec',
            "compressed" : True,
            "optimize" : 2,
            "ascii" : 0,
            "bundle_files" : 1,  # 关于这个参数请看第三部分中的问题(2)
        }

        setup(
            name='FutureTools',
            version='1.0',
            windows=[{"script":python_script, "icon_resources":[(1, "img\info.ico")]}],  # 括号中更改为你要打包的代码文件名
            # console=[python_script],  # 括号中更改为你要打包的代码文件名
            zipfile=None,
            data_files=[('imageformats', [r'D:\Python34\Lib\site-packages\PyQt4\plugins\imageformats\qico4.dll'])],
            options={'py2exe': py2exe_options}
        )

if __name__ == '__main__':
    Make_Exe('ToolWindow.py')