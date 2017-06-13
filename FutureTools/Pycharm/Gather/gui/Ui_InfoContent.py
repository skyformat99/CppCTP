# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\CTP\CppCTP\FutureTools\InfoContent.ui'
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

class Ui_InfoContent(object):
    def setupUi(self, InfoContent):
        InfoContent.setObjectName(_fromUtf8("InfoContent"))
        InfoContent.resize(800, 500)
        self.horizontalLayout = QtGui.QHBoxLayout(InfoContent)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.frame = QtGui.QFrame(InfoContent)
        self.frame.setFrameShape(QtGui.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtGui.QFrame.Raised)
        self.frame.setObjectName(_fromUtf8("frame"))
        self.horizontalLayout_2 = QtGui.QHBoxLayout(self.frame)
        self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))
        self.textBrowser = QtGui.QTextBrowser(self.frame)
        self.textBrowser.setObjectName(_fromUtf8("textBrowser"))
        self.horizontalLayout_2.addWidget(self.textBrowser)
        self.horizontalLayout.addWidget(self.frame)

        self.retranslateUi(InfoContent)
        QtCore.QMetaObject.connectSlotsByName(InfoContent)

    def retranslateUi(self, InfoContent):
        InfoContent.setWindowTitle(_translate("InfoContent", "资讯内容", None))
        self.textBrowser.setHtml(_translate("InfoContent", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p></body></html>", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    InfoContent = QtGui.QWidget()
    ui = Ui_InfoContent()
    ui.setupUi(InfoContent)
    InfoContent.show()
    sys.exit(app.exec_())

