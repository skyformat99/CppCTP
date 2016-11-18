# -*- coding: utf-8 -*-

"""
Module implementing MainWindow.
"""

import sys
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QMainWindow
from Ui_ToolWindow import Ui_MainWindow
from InfoContent import InfoContent
from FutureTab import FutureTab
from PipeLine import PipeLine
from FutureSpider import InfoParser

class MainWindow(QMainWindow, Ui_MainWindow):
    """
    Class documentation goes here.
    """
    def __init__(self, parent=None):
        """
        Constructor
        
        @param parent reference to the parent widget
        @type QWidget
        """
        super(MainWindow, self).__init__(parent)
        self.setupUi(self)

        self.hideAction = QtGui.QAction("&隐藏", self, triggered=self.hide)
        self.showAction = QtGui.QAction("&显示", self, triggered=self.showNormal)
        self.quitAction = QtGui.QAction("&退出", self, triggered=self.quitWindow)
        self.trayIconMenu = QtGui.QMenu(self)
        self.trayIconMenu.addAction(self.hideAction)
        self.trayIconMenu.addAction(self.showAction)
        self.trayIconMenu.addSeparator()
        self.trayIconMenu.addAction(self.quitAction)

        self.icon = QtGui.QIcon('img/info.png')
        self.trayIcon = QtGui.QSystemTrayIcon()
        self.trayIcon.setToolTip("我是交易所公告程序")
        self.trayIcon.setIcon(self.icon)
        self.setWindowIcon(self.icon)
        self.trayIcon.activated.connect(self.iconActivated)

        self.trayIcon.setContextMenu(self.trayIconMenu)
        self.trayIcon.show()
    
    @pyqtSlot()
    def on_action_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        raise NotImplementedError

    @pyqtSlot()
    def quitWindow(self):
        self.pipeline.closeDB()
        self.trayIcon.hide()
        QtGui.qApp.quit()

    @pyqtSlot()
    def on_action_2_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        raise NotImplementedError
    
    @pyqtSlot()
    def on_action_3_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        raise NotImplementedError


    def iconActivated(self, reason):
        if reason in (QtGui.QSystemTrayIcon.Trigger, QtGui.QSystemTrayIcon.DoubleClick):
            self.show()


    def closeEvent(self, event):
        self.trayIcon.showMessage("期货公告程序", "我将隐藏在右下角哦!")

    def setPipeLine(self, pipe):
        self.pipeline = pipe

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    QtGui.QApplication.setQuitOnLastWindowClosed(False)

    # style sheet
    file = QtCore.QFile('img/silvery.css')

    file.open(QtCore.QFile.ReadOnly)
    styleSheet = file.readAll().data().decode("utf-8")
    file.close()

    QtGui.qApp.setStyleSheet(styleSheet)



    # 爬虫目标地址
    sh_url = "http://www.shfe.com.cn/news/notice/"

    mainwin = MainWindow()

    desRect = QtGui.QApplication.desktop().availableGeometry()
    mainwin.move(desRect.left() + 200, 200)

    info_win = InfoContent()

    info_win.move(desRect.left() + 200, 200)

    sh_ftab = FutureTab()
    sh_ftab.setContentWindow(info_win)

    # 创建管道类
    pipe = PipeLine()
    # 互相绑定
    pipe.set_SH_FutureTab(sh_ftab)
    # sh_ftab.setPipeLine(pipe)
    mainwin.setPipeLine(pipe)

    # 初始化爬虫类
    spider = InfoParser(pipe)
    # 调用爬虫爬虫方法
    spider.SH_getPageLink(sh_url)

    # time.sleep(2)

    # pipe获取数据
    pipe.getInfo('SH')

    mainwin.tabWidget.addTab(sh_ftab, "上海期货交易所")

    mainwin.show()
    # QtCore.QCoreApplication.quit()
    sys.exit(app.exec_())