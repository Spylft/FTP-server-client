# 166.111.80.66 测试ip

import socket
import sys
import os
import math
from PyQt5.QtWidgets import QApplication, QMainWindow,  QFileDialog, QPushButton, QLineEdit, QTableWidgetItem, QMessageBox, QAbstractItemView
from PyQt5.QtCore import QObject, Qt, QThread, pyqtSignal
from PyQt5.uic import loadUi
import time
import _thread


class MyClient:
    connection_data = socket.socket()
    connection_id = socket.socket()
    connection_listen = socket.socket()
    server_path = ''
    server_ip = ''
    server_port_connection = 0  # 命令传输端口
    server_port_data = 0  # 数据传输端口
    client_path = ''
    client_ip = ''
    port_arg = 0  # PORT命令的参数
    transport_mode = 1  # 0PORT 1PASV
    is_transporting = 0  # 是否在传输文件

    def __init__(self) -> None:
        '''
        初始化客户端信息以及图形化窗口
        '''
        self.init_client()
        self.MainWindow = loadUi("mainwindow.ui")
        self.Connect_Signal_Slots()
        self.MainWindow.show()

    def init_client(self):
        '''
        初始化客户端信息
        '''
        pass

    def Connect_Signal_Slots(self):
        '''
        进行组件的信号与槽的连接
        '''
        self.MainWindow.Button_Login.clicked.connect(self.Login)
        self.MainWindow.Button_Logout.clicked.connect(self.Logout)
        self.MainWindow.Button_Refresh.clicked.connect(self.Refresh)
        self.MainWindow.Button_Rename.clicked.connect(self.Rename)
        self.MainWindow.Button_Download.clicked.connect(self.Download)
        self.MainWindow.Button_Upload.clicked.connect(self.Upload)
        self.MainWindow.Button_Makedir.clicked.connect(self.Makedir)
        self.MainWindow.Button_Changedir.clicked.connect(self.Changedir)
        self.MainWindow.Button_Backdir.clicked.connect(self.Backdir)
        self.MainWindow.Button_Portmode.clicked.connect(self.Portmode)
        self.MainWindow.Button_Pasvmode.clicked.connect(self.Pasvmode)

    def Write_Message(self, message):
        '''
        向server发送命令
        参数：
            message：需要发送的信息
        '''
        pass

    def Read_Message(self):
        '''
        从server处获取命令返回信息
        '''
        pass

    def File_Receive(self):
        '''
        接受文件到本地
        '''
        pass

    def File_Transport(self):
        '''
        发送文件到server
        '''
        pass

    def Login(self):
        '''
        登陆操作，即进行USER与PASS命令
        '''
        ip = self.MainWindow.Input_IP.toPlainText()
        port = self.MainWindow.Input_Port.toPlainText()
        username = self.MainWindow.Input_Username.toPlainText()
        password = self.MainWindow.Input_Password.toPlainText()
        pass

    def Logout(self):
        '''
        登出操作，即进行QUIT命令
        '''
        pass

    def Refresh(self):
        '''
        刷新文件列表，即进行LIST命令
        '''
        pass

    def Rename(self):
        '''
        重命名操作，即进行RNFR与RNTO命令
        '''
        path_name = self.MainWindow.Input_Pathname.toPlainText()
        new_path_name = self.MainWindow.Input_Newpathname.toPlainText()
        pass

    def Download(self):
        '''
        下载文件操作，即进行PORT(PASV)与RETR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        pass

    def Upload(self):
        '''
        上传文件操作，即进行PORT(PASV)与STOR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        pass

    def Makedir(self):
        '''
        新建文件夹操作，即进行MKD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        pass

    def Changedir(self):
        '''
        更换工作目录操作，即进行CWD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        pass

    def Backdir(self):
        '''
        退回上一级目录，即进行CWD命令
        '''
        pass

    def Portmode(self):
        '''
        更换传输模式为主动模式(PORT)，保存port参数，但不向server传输命令
        '''
        self.transport_mode = 0
        pass

    def Pasvmode(self):
        '''
        更换传输模式为被动模式(PASV)，但不向server传输命令
        '''
        self.transport_mode = 1
        self.port_arg = int(self.MainWindow.Input_Port.toPlainText())
        pass


if __name__ == '__main__':
    app = QApplication(sys.argv)
    Client = MyClient()
    sys.exit(app.exec_())
