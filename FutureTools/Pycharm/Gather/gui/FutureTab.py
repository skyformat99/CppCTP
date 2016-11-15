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
        self.tableWidget.setColumnWidth(1, 530)
        self.tableWidget.setColumnWidth(2, 100)
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


    def set_init_data(self, data, pipe):
        self.tableWidget.clearContents()
        self.table_data = data
        print("table_data", self.table_data)
        self.pipeline = pipe
        i = 1
        for row_item in data:
            id_item = QtGui.QTableWidgetItem(str(row_item['id']))
            title_item = QtGui.QTableWidgetItem(row_item['title'])
            pubtime_item = QtGui.QTableWidgetItem(row_item['pubtime'])
            isread_item = QtGui.QTableWidgetItem()

            id_item.setTextAlignment(QtCore.Qt.AlignCenter)
            isread_item.setTextAlignment(QtCore.Qt.AlignCenter)

            if (row_item['isread'] == 0):
                isread_item.setIcon(QtGui.QIcon('img/unread.png'))
            else:
                isread_item.setIcon(QtGui.QIcon('img/read.png'))


            self.tableWidget.setItem(i, 0, id_item)
            self.tableWidget.setItem(i, 1, title_item)
            self.tableWidget.setItem(i, 2, pubtime_item)
            # self.tableWidget.setColumnHidden(0, True)

            self.tableWidget.setItem(i, 3, isread_item)
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
