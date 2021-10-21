#166.111.80.66 测试ip

import sys
from PyQt5.QtWidgets import QApplication, QMainWindow

import Ui_mainwindow

app = QApplication(sys.argv)
MainWindow = QMainWindow()
ui = Ui_mainwindow.Ui_MainWindow()
ui.setupUi(MainWindow)
MainWindow.show()
sys.exit(app.exec_())