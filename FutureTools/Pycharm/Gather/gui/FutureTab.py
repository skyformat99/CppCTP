# -*- coding: utf-8 -*-

"""
Module implementing FutureTab.
"""
import sys
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QWidget
from InfoContent import InfoContent
from Ui_FutureTab import Ui_FutureTab
from PipeLine import PipeLine


class FutureTab(QWidget, Ui_FutureTab):
    """
    Class documentation goes here.
    """
    def __init__(self, parent=None):
        """
        Constructor
        
        @param parent reference to the parent widget
        @type QWidget
        """
        super(FutureTab, self).__init__(parent)
        self.setupUi(self)
        self.tableWidget.setRowCount(20)
        self.tableWidget.setColumnCount(4)
        # self.tableWidget.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        # self.tableWidget.horizontalHeader().setStretchLastSection(True)
        # self.tableWidget.resizeColumnsToContents()
        self.tableWidget.setColumnWidth(0, 70)
        self.tableWidget.setColumnWidth(1, 510)
        self.tableWidget.setColumnWidth(2, 120)
        self.tableWidget.setColumnWidth(3, 50)

    @pyqtSlot(int, int)
    def on_tableWidget_cellDoubleClicked(self, row, column):
        """
        Slot documentation goes here.
        
        @param row DESCRIPTION
        @type int
        @param column DESCRIPTION
        @type int
        """
        if (self.tableWidget.item(row, 0) is not None):
            for item in self.table_data:
                if str(item['id']) == self.tableWidget.item(row, 0).text():
                    if (item['isread'] == 0):
                        self.tableWidget.item(row, 3).setIcon(QtGui.QIcon('img/read.png'))
                        self.updateReadStatus(item['id'], 1)

                    self.content_window.textBrowser.setText(item['content'])
                    self.content_window.setWindowModality(QtCore.Qt.ApplicationModal)
                    self.content_window.show()


    def updateReadStatus(self, id, isread):
        self.pipeline.updateInfo(id, isread)

    def clear_tab_data(self):
        for i in range(20):
            self.tableWidget.item(i, 0).setText("")
            self.tableWidget.item(i, 1).setText("")
            self.tableWidget.item(i, 2).setText("")
            self.tableWidget.item(i, 3).setIcon(QtGui.QIcon(''))


    def set_init_data(self, data, pipe):
        # print("FutureTab set_init_data")
        self.clear_tab_data()
        self.table_data = data
        # print("table_data", self.table_data)
        self.pipeline = pipe
        i = 0
        for row_item in data:
            # print(self.tableWidget.item(i, 0))
            self.tableWidget.item(i, 0).setTextAlignment(QtCore.Qt.AlignCenter)
            self.tableWidget.item(i, 1).setTextAlignment(QtCore.Qt.AlignCenter)
            self.tableWidget.item(i, 2).setTextAlignment(QtCore.Qt.AlignCenter)
            self.tableWidget.item(i, 3).setTextAlignment(QtCore.Qt.AlignCenter)

            self.tableWidget.item(i, 0).setText(str(row_item['id']))
            self.tableWidget.item(i, 1).setText(row_item['title'])
            self.tableWidget.item(i, 2).setText(row_item['pubtime'])

            if (row_item['isread'] == 0):
                self.tableWidget.item(i, 3).setIcon(QtGui.QIcon('img/unread.png'))
            else:
                self.tableWidget.item(i, 3).setIcon(QtGui.QIcon('img/read.png'))
            i = i + 1
        pass
    def setContentWindow(self, obj):
        self.content_window = obj

    def setPipeLine(self, pipe):
        self.pipeline = pipe

if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    ft = FutureTab()
    ft.show()
    sys.exit(app.exec_())
