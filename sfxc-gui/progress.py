# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'progress.ui'
#
# Created: Mon Nov 21 15:12:37 2016
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_Dialog1(object):
    def setupUi(self, Dialog1):
        Dialog1.setObjectName(_fromUtf8("Dialog1"))
        Dialog1.resize(659, 365)
        self.Layout3 = QtGui.QWidget(Dialog1)
        self.Layout3.setGeometry(QtCore.QRect(20, 10, 200, 24))
        self.Layout3.setObjectName(_fromUtf8("Layout3"))
        self.hboxlayout = QtGui.QHBoxLayout(self.Layout3)
        self.hboxlayout.setSpacing(6)
        self.hboxlayout.setMargin(0)
        self.hboxlayout.setMargin(0)
        self.hboxlayout.setObjectName(_fromUtf8("hboxlayout"))
        self.scanLabel = QtGui.QLabel(self.Layout3)
        self.scanLabel.setWordWrap(False)
        self.scanLabel.setObjectName(_fromUtf8("scanLabel"))
        self.hboxlayout.addWidget(self.scanLabel)
        spacerItem = QtGui.QSpacerItem(100, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem)
        self.scanEdit = QtGui.QLineEdit(self.Layout3)
        self.scanEdit.setReadOnly(True)
        self.scanEdit.setObjectName(_fromUtf8("scanEdit"))
        self.hboxlayout.addWidget(self.scanEdit)
        self.Layout2 = QtGui.QWidget(Dialog1)
        self.Layout2.setGeometry(QtCore.QRect(20, 50, 200, 24))
        self.Layout2.setObjectName(_fromUtf8("Layout2"))
        self.hboxlayout1 = QtGui.QHBoxLayout(self.Layout2)
        self.hboxlayout1.setSpacing(6)
        self.hboxlayout1.setMargin(0)
        self.hboxlayout1.setMargin(0)
        self.hboxlayout1.setObjectName(_fromUtf8("hboxlayout1"))
        self.timeLabel = QtGui.QLabel(self.Layout2)
        self.timeLabel.setWordWrap(False)
        self.timeLabel.setObjectName(_fromUtf8("timeLabel"))
        self.hboxlayout1.addWidget(self.timeLabel)
        spacerItem1 = QtGui.QSpacerItem(100, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.hboxlayout1.addItem(spacerItem1)
        self.timeEdit = QtGui.QLineEdit(self.Layout2)
        self.timeEdit.setReadOnly(True)
        self.timeEdit.setObjectName(_fromUtf8("timeEdit"))
        self.hboxlayout1.addWidget(self.timeEdit)
        self.Layout1 = QtGui.QWidget(Dialog1)
        self.Layout1.setGeometry(QtCore.QRect(20, 320, 620, 33))
        self.Layout1.setObjectName(_fromUtf8("Layout1"))
        self.hboxlayout2 = QtGui.QHBoxLayout(self.Layout1)
        self.hboxlayout2.setSpacing(6)
        self.hboxlayout2.setMargin(0)
        self.hboxlayout2.setMargin(0)
        self.hboxlayout2.setObjectName(_fromUtf8("hboxlayout2"))
        self.buttonReset = QtGui.QPushButton(self.Layout1)
        self.buttonReset.setEnabled(False)
        self.buttonReset.setAutoDefault(False)
        self.buttonReset.setObjectName(_fromUtf8("buttonReset"))
        self.hboxlayout2.addWidget(self.buttonReset)
        spacerItem2 = QtGui.QSpacerItem(20, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.hboxlayout2.addItem(spacerItem2)
        self.buttonAbort = QtGui.QPushButton(self.Layout1)
        self.buttonAbort.setAutoDefault(False)
        self.buttonAbort.setDefault(False)
        self.buttonAbort.setObjectName(_fromUtf8("buttonAbort"))
        self.hboxlayout2.addWidget(self.buttonAbort)
        self.logEdit = QtGui.QPlainTextEdit(Dialog1)
        self.logEdit.setGeometry(QtCore.QRect(20, 130, 621, 171))
        self.logEdit.setReadOnly(True)
        self.logEdit.setObjectName(_fromUtf8("logEdit"))
        self.progressBar = QtGui.QProgressBar(Dialog1)
        self.progressBar.setGeometry(QtCore.QRect(20, 90, 621, 23))
        self.progressBar.setProperty("value", 24)
        self.progressBar.setObjectName(_fromUtf8("progressBar"))
        self.Layout3_2 = QtGui.QWidget(Dialog1)
        self.Layout3_2.setGeometry(QtCore.QRect(340, 10, 200, 24))
        self.Layout3_2.setObjectName(_fromUtf8("Layout3_2"))
        self._2 = QtGui.QHBoxLayout(self.Layout3_2)
        self._2.setSpacing(6)
        self._2.setMargin(0)
        self._2.setMargin(0)
        self._2.setObjectName(_fromUtf8("_2"))
        self.jobLabel = QtGui.QLabel(self.Layout3_2)
        self.jobLabel.setWordWrap(False)
        self.jobLabel.setObjectName(_fromUtf8("jobLabel"))
        self._2.addWidget(self.jobLabel)
        spacerItem3 = QtGui.QSpacerItem(93, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self._2.addItem(spacerItem3)
        self.jobEdit = QtGui.QLineEdit(self.Layout3_2)
        self.jobEdit.setReadOnly(True)
        self.jobEdit.setObjectName(_fromUtf8("jobEdit"))
        self._2.addWidget(self.jobEdit)
        self.Layout3_3 = QtGui.QWidget(Dialog1)
        self.Layout3_3.setGeometry(QtCore.QRect(340, 50, 200, 24))
        self.Layout3_3.setObjectName(_fromUtf8("Layout3_3"))
        self._3 = QtGui.QHBoxLayout(self.Layout3_3)
        self._3.setSpacing(6)
        self._3.setMargin(0)
        self._3.setMargin(0)
        self._3.setObjectName(_fromUtf8("_3"))
        self.subjobLabel = QtGui.QLabel(self.Layout3_3)
        self.subjobLabel.setWordWrap(False)
        self.subjobLabel.setObjectName(_fromUtf8("subjobLabel"))
        self._3.addWidget(self.subjobLabel)
        spacerItem4 = QtGui.QSpacerItem(70, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self._3.addItem(spacerItem4)
        self.subjobEdit = QtGui.QLineEdit(self.Layout3_3)
        self.subjobEdit.setReadOnly(True)
        self.subjobEdit.setObjectName(_fromUtf8("subjobEdit"))
        self._3.addWidget(self.subjobEdit)

        self.retranslateUi(Dialog1)
        QtCore.QObject.connect(self.buttonAbort, QtCore.SIGNAL(_fromUtf8("clicked()")), Dialog1.abort)
        QtCore.QMetaObject.connectSlotsByName(Dialog1)

    def retranslateUi(self, Dialog1):
        Dialog1.setWindowTitle(QtGui.QApplication.translate("Dialog1", "Progress", None, QtGui.QApplication.UnicodeUTF8))
        self.scanLabel.setText(QtGui.QApplication.translate("Dialog1", "Scan:", None, QtGui.QApplication.UnicodeUTF8))
        self.timeLabel.setText(QtGui.QApplication.translate("Dialog1", "Time:", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonReset.setText(QtGui.QApplication.translate("Dialog1", "&Reset", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonReset.setShortcut(QtGui.QApplication.translate("Dialog1", "Alt+R", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAbort.setText(QtGui.QApplication.translate("Dialog1", "&Abort", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonAbort.setShortcut(QtGui.QApplication.translate("Dialog1", "Alt+A", None, QtGui.QApplication.UnicodeUTF8))
        self.jobLabel.setText(QtGui.QApplication.translate("Dialog1", "Job ID:", None, QtGui.QApplication.UnicodeUTF8))
        self.subjobLabel.setText(QtGui.QApplication.translate("Dialog1", "Subjob ID:", None, QtGui.QApplication.UnicodeUTF8))

