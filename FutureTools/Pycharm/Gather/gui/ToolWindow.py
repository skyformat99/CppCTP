# -*- coding: utf-8 -*-

"""
Module implementing MainWindow.
"""

import sys
from Ui_ToolWindow import Ui_MainWindow
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QMainWindow

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
        # self.quitAction = QtGui.QAction("&退出", self, triggered=QtGui.qApp.quit)
        self.trayIconMenu = QtGui.QMenu(self)
        self.trayIconMenu.addAction(self.hideAction)
        self.trayIconMenu.addAction(self.showAction)
        self.trayIconMenu.addSeparator()
        self.trayIconMenu.addAction(self.quitAction)

        self.icon = QtGui.QIcon('img\info.png')
        self.trayIcon = QtGui.QSystemTrayIcon()
        self.trayIcon.setToolTip("我是交易所公告程序")
        self.trayIcon.setIcon(self.icon)
        self.setWindowIcon(self.icon)
        self.trayIcon.activated.connect(self.iconActivated)

        self.trayIcon.setContextMenu(self.trayIconMenu)
        self.trayIcon.show()

        self.gather_timer = QtCore.QTimer()
        self.is_check_info = True
        QtCore.QObject.connect(self.gather_timer, QtCore.SIGNAL("timeout()"), self.OnTimer)
        self.gather_timer.start(1800000)

    def setIs_Check_Info(self, is_check_info):
        self.is_check_info = is_check_info

    def OnTimer(self):
        self.showTrayMessage()
        sh_url = "http://www.shfe.com.cn/news/notice/"
        dl_url = "http://www.dce.com.cn/dalianshangpin/yw/fw/jystz/ywtz/index.html"
        from FutureSpider import InfoParser
        self.spider.SH_getPageLink(sh_url)
        self.spider.DL_getPageLink(dl_url)
        from PipeLine import PipeLine
        self.pipeline.countNum()
        pass

    def showTrayMessage(self):
        if (not self.is_check_info):
                self.trayIcon.showMessage("您有新公告!", "请查看新公告!")
                self.is_check_info = True

    @pyqtSlot()
    def on_action_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        pass

    @pyqtSlot()
    def quitWindow(self):
        from PipeLine import PipeLine
        self.pipeline.closeDB()
        self.trayIcon.hide()

        del self.sh_ftab
        del self.dl_ftab
        del self.win_content
        del self.spider
        del self.pipeline
        self.deleteLater()

        # print("haha1")
        # QtGui.qApp.quit()
        sys.exit(0)
        # print("haha2")

    @pyqtSlot()
    def on_action_2_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        pass
    
    @pyqtSlot()
    def on_action_3_triggered(self):
        """
        Slot documentation goes here.
        """
        # TODO: not implemented yet
        pass


    def iconActivated(self, reason):
        if reason in (QtGui.QSystemTrayIcon.Trigger, QtGui.QSystemTrayIcon.DoubleClick):
            self.is_check_info = True
            self.show()


    def closeEvent(self, event):
        self.trayIcon.showMessage("期货公告程序", "我将隐藏在右下角哦!")

    def setPipeLine(self, pipe):
        self.pipeline = pipe

    def getPipeLine(self):
        return self.pipeline

    def setSpider(self, spider):
        # 爬虫目标地址
        self.spider = spider

    def setContent(self, win_content):
        self.win_content = win_content

    def setSHTab(self, sh_ftab):
        self.sh_ftab = sh_ftab

    def setDLTab(self, dl_ftab):
        self.dl_ftab = dl_ftab

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    QtGui.QApplication.setQuitOnLastWindowClosed(False)

    # 添加样式表
    file = QtCore.QFile('img/silvery.css')
    file.open(QtCore.QFile.ReadOnly)
    styleSheet = file.readAll().data().decode("utf-8")
    file.close()
    QtGui.qApp.setStyleSheet(styleSheet)



    # 爬虫目标地址
    sh_url = "http://www.shfe.com.cn/news/notice/"
    dl_url = "http://www.dce.com.cn/dalianshangpin/yw/fw/jystz/ywtz/index.html"

    mainwin = MainWindow()
    # app.setActiveWindow(mainwin)

    desRect = QtGui.QApplication.desktop().availableGeometry()
    mainwin.move(desRect.left() + 200, 200)

    from InfoContent import InfoContent
    info_win = InfoContent()

    info_win.move(desRect.left() + 200, 200)
    mainwin.setContent(info_win)

    from FutureTab import FutureTab
    sh_ftab = FutureTab()
    sh_ftab.setContentWindow(info_win)

    dl_ftab = FutureTab()
    dl_ftab.setContentWindow(info_win)

    mainwin.setSHTab(sh_ftab)
    mainwin.setDLTab(dl_ftab)

    from FutureSpider import InfoParser
    from PipeLine import PipeLine
    # 创建管道类
    pipe = PipeLine()

    # 互相绑定
    pipe.set_SH_FutureTab(sh_ftab)
    pipe.set_DL_FutureTab(dl_ftab)
    # sh_ftab.setPipeLine(pipe)
    mainwin.setPipeLine(pipe)
    pipe.setMainWindow(mainwin)

    # 初始化爬虫类
    spider = InfoParser(pipe)
    # 调用爬虫爬虫方法
    spider.SH_getPageLink(sh_url)
    spider.DL_getPageLink(dl_url)
    mainwin.setSpider(spider)

    # 获取数量
    pipe.countNum()

    # time.sleep(2)

    # pipe获取数据
    pipe.getInfo('SH')
    pipe.getInfo('DL')

    mainwin.tabWidget.addTab(sh_ftab, "上海期货交易所")
    mainwin.tabWidget.addTab(dl_ftab, "大连商品交易所")

    mainwin.show()
    # QtCore.QCoreApplication.quit()
    sys.exit(app.exec_())