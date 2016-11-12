# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\CTP\CppCTP\FutureTools\FutureTab.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_FutureTab(object):
    def setupUi(self, FutureTab):
        FutureTab.setObjectName(_fromUtf8("FutureTab"))
        FutureTab.resize(950, 500)
        self.horizontalLayout = QtGui.QHBoxLayout(FutureTab)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.tableWidget = QtGui.QTableWidget(FutureTab)
        self.tableWidget.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.SingleSelection)
        self.tableWidget.setSelectionBehavior(QtGui.QAbstractItemView.SelectRows)
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.setColumnCount(4)
        self.tableWidget.setRowCount(0)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(0, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(1, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(2, item)
        item = QtGui.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(3, item)
        self.tableWidget.horizontalHeader().setVisible(True)
        self.tableWidget.horizontalHeader().setCascadingSectionResizes(True)
        self.tableWidget.horizontalHeader().setDefaultSectionSize(300)
        self.tableWidget.horizontalHeader().setHighlightSections(True)
        self.tableWidget.horizontalHeader().setMinimumSectionSize(20)
        self.tableWidget.horizontalHeader().setSortIndicatorShown(False)
        self.tableWidget.horizontalHeader().setStretchLastSection(True)
        self.tableWidget.verticalHeader().setVisible(False)
        self.horizontalLayout.addWidget(self.tableWidget)

        self.retranslateUi(FutureTab)
        QtCore.QMetaObject.connectSlotsByName(FutureTab)

    def retranslateUi(self, FutureTab):
        FutureTab.setWindowTitle(_translate("FutureTab", "交易所页面", None))
        item = self.tableWidget.horizontalHeaderItem(0)
        item.setText(_translate("FutureTab", "公告ID", None))
        item = self.tableWidget.horizontalHeaderItem(1)
        item.setText(_translate("FutureTab", "交易所公告", None))
        item = self.tableWidget.horizontalHeaderItem(2)
        item.setText(_translate("FutureTab", "发布时间", None))
        item = self.tableWidget.horizontalHeaderItem(3)
        item.setText(_translate("FutureTab", "阅读否", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    FutureTab = QtGui.QWidget()
    ui = Ui_FutureTab()
    ui.setupUi(FutureTab)
    FutureTab.show()
    sys.exit(app.exec_())

