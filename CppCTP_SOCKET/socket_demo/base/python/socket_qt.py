#coding=utf-8

import socket
import threading
import sys

from PyQt4 import QtCore as core
from PyQt4 import QtGui as gui

#host = '192.168.1.44'
host = '10.0.0.29'
port = 8000
username = 'Default'

class Client(gui.QWidget):
    def __init__(self, parent = None):
        super(Client, self).__init__(parent)
        self.setWindowTitle('CLient')

        self.setNameWidget = gui.QWidget()
        self.layout = gui.QGridLayout(self)
        self.setNameLayout = gui.QGridLayout(self.setNameWidget)
        self.btnSend = gui.QPushButton('send')
        self.btnSet = gui.QPushButton('Set')
        self.input = gui.QLineEdit()
        self.name = gui.QLineEdit('Default')
        self.chat = gui.QTextEdit()
        self.label = gui.QLabel('name:')

        self.timer = core.QTimer()
        self.messages = []

        self.build()
        self.createAction()

        recvThread = threading.Thread(target = self.recvFromServer)
        recvThread.setDaemon(True)
        recvThread.start()

    def sendToServer(self):
        global username
        text = str(self.input.text())
        self.input.setText('')
        if text == 'q':
            self.exit()
        elif text.strip() == '':
            return
        try:
            s.send('%s' % text)
            print 'Client Send --> %s' % text
        except:
            self.exit()

    def recvFromServer(self):
        while 1:
            try:
                data = s.recv(1024)
                if not data:
                    exit()
                else:
                    print "Client Received <-- %s" % data
                self.messages.append(data)
                self.showChat()
            except:
                return

    def showChat(self):
        for m in self.messages:
            self.chat.append("Client Received:" + m)
        self.messages = []

    def slotExtension(self):
        global username
        name = str(self.name.text())
        if name.strip() != '':
            username = name
            print username
        self.setNameWidget.hide()

    def exit(self):
        s.close()
        sys.exit()

    def build(self):
        self.layout.addWidget(self.chat, 0, 0, 5, 4)
        self.layout.addWidget(self.input, 5, 0, 1, 4)
        self.layout.addWidget(self.btnSend, 5, 4)

        self.setNameLayout.addWidget(self.label, 0, 0)
        self.setNameLayout.addWidget(self.name, 0, 1)
        self.setNameLayout.addWidget(self.btnSet, 0, 4)
        self.layout.addWidget(self.setNameWidget, 6, 0)

        self.layout.setSizeConstraint(gui.QLayout.SetFixedSize)

    def createAction(self):
        self.btnSend.clicked.connect(self.sendToServer)
        self.btnSet.clicked.connect(self.slotExtension)
        #self.timer.timeout.connect(self.showChat)
        #self.timer.start(1000)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
s.send(username)
print '[%s] connect' % username

app = gui.QApplication(sys.argv)
c = Client()
c.show()
app.exec_()