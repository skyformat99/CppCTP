# -*- coding: utf-8 -*-

"""
Module implementing InfoContent.
"""

import sys
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QWidget

from Ui_InfoContent import Ui_InfoContent


class InfoContent(QWidget, Ui_InfoContent):
    """
    Class documentation goes here.
    """
    def __init__(self, parent=None):
        """
        Constructor
        
        @param parent reference to the parent widget
        @type QWidget
        """
        super(InfoContent, self).__init__(parent)
        self.setupUi(self)

    def setPipeLine(self, pipeline):
        self.pipeline = pipeline


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    ft = InfoContent()
    ft.show()
    sys.exit(app.exec_())