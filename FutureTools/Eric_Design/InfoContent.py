# -*- coding: utf-8 -*-

"""
Module implementing InfoContent.
"""

from PyQt4.QtCore import pyqtSlot
from PyQt4.QtGui import QWidget

from .Ui_InfoContent import Ui_InfoContent


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
