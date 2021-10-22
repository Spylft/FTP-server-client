# 166.111.80.66 测试ip

import socket
import sys
import os
import math
from PyQt5.QtGui import QStandardItem, QStandardItemModel
from PyQt5.QtWidgets import QApplication, QMainWindow,  QFileDialog, QPushButton, QLineEdit, QTableWidgetItem, QMessageBox, QAbstractItemView
from PyQt5.QtCore import QObject, Qt, QThread, pyqtSignal
from PyQt5.uic import loadUi
import time
import _thread
import random
import re

socket_maxlen = 8192


class MyClient:
    connection_data = None
    connection_id = socket.socket()
    connection_listen = None
    server_ip = ''
    server_port_connection = 0  # 命令传输端口
    server_port_data = 0  # 数据传输端口
    client_ip = ''
    port_arg = 0  # PORT命令的参数
    transport_mode = 1  # 0PORT 1PASV
    is_transporting = 0  # 是否在传输文件
    file_model = QStandardItemModel()
    download_model = QStandardItemModel()
    upload_model = QStandardItemModel()

    def __init__(self) -> None:
        '''
        初始化客户端信息以及图形化窗口
        '''
        self.Init_Client()
        self.MainWindow = loadUi('mainwindow.ui')
        self.Init_Window()
        self.Connect_Signal_Slots()
        self.MainWindow.show()

    def Init_Client(self):
        '''
        初始化客户端信息
        '''
        hostname = socket.gethostname()
        self.client_ip = socket.gethostbyname(hostname)
        pass

    def Init_Window(self):
        '''
        初始化图形化窗口内内容
        '''
        self.MainWindow.Show_File.setModel(self.file_model)
        self.MainWindow.Show_Download.setModel(self.download_model)
        self.MainWindow.Show_Upload.setModel(self.upload_model)
        self.file_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.file_model.setHorizontalHeaderItem(
            1, QStandardItem('file/directory'))
        self.file_model.setHorizontalHeaderItem(
            2, QStandardItem('size'))
        self.file_model.setHorizontalHeaderItem(
            3, QStandardItem('last edit time'))
        self.download_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.download_model.setHorizontalHeaderItem(
            1, QStandardItem('status'))
        self.upload_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.upload_model.setHorizontalHeaderItem(
            1, QStandardItem('status'))

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
        self.MainWindow.Button_Portmode.clicked.connect(self.Portmode)
        self.MainWindow.Button_Pasvmode.clicked.connect(self.Pasvmode)

    def Warning_Box(self,  title, message):
        '''
        弹出错误信息串口
        参数：
            window：错误信息窗口出现在window之上
            title：错误信息窗口标题
            message：错误信息窗口文字
        '''
        QMessageBox.warning(self.MainWindow, title, message)

    def Check_Invalid(self, message):
        '''
        检查错误码是否错误
        参数：
            message：server传过来的信息
        返回：
            0正常 1异常
        '''
        if message[0:2].isdigit() and message[3] == ' ':
            num = int(message[0:2])
            if num >= 200 and num <= 400:
                return True
        return False

    def Get_Random_Port(self):
        '''
        返回一个随机端口
        返回：
            随机端口
        '''
        return random.randint(20000, 65535)

    def Connect_Mode(self):
        '''
        根据自己的连接模式向server发送连接消息
        返回：
            0成功 1失败
        '''
        if self.connection_data != None:
            self.connection_data.close()
            self.connection_data = None
        if self.connection_listen != None:
            self.connection_listen.close()
            self.connection_listen = None
        if self.transport_mode == 0:  # PORT
            port = self.Get_Random_Port()
            server_ip_re = re.match(
                r'([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1-3})\.([0-9]{1,3})', self.server_ip)
            message_port = 'PORT ('+server_ip_re[1]+','+server_ip_re[2]+','+server_ip_re[3] + \
                ','+server_ip_re[4]+',' + \
                str(port/256)+','+str(port % 256)+')\r\n'
            self.connection_id.send(str.encode(message_port))
            message = self.connection_id.recv(
                socket_maxlen).decode('UTF-8', 'strict')
            if self.Check_Invalid(message):
                self.Warning_Box('Port command error', message)
                return False
            self.connection_listen = socket.socket()
            self.connection_listen.bind(self.client_ip, port)
            self.connection_listen.listen(10)
        else:
            message_pasv = "PASV\r\n"
            self.connection_id.send(str.encode(message_pasv))
            message = self.connection_id.recv(
                socket_maxlen).decode('UTF-8', 'strict')
            if self.Check_Invalid(message):
                self.Warning_Box('PASV command error', message)
                return False
            server_ip_port_re = re.match(
                r'.*\(([0-9]{1,3}),([0-9]{1,3}),([0-9]{1-3}),([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})\).*', self.server_ip)
            self.server_port_data = int(
                server_ip_port_re[5])*256+int(server_ip_port_re[6])
        return True

    def Connect(self):
        '''
        与server连接
        返回：
            0成功 1失败
        '''
        if self.transport_mode == 0:  # PORT
            try:
                self.connection_data = socket.socket()
                self.connection_data.connect(
                    self.server_ip, self.server_port_data)
            except:
                self.Warning_Box('Connect error', 'Data connection failed')
                return False
        else:  # PASV
            try:
                self.connection_data, ip_port = self.connection_listen.accept()
            except:
                self.Warning_Box('Connect error', 'Can\'t listen connection')
                return False
        return True

    def Login(self):
        '''
        登陆操作，即进行USER与PASS命令
        '''
        try:
            self.server_ip = self.MainWindow.Input_IP.toPlainText()
            self.server_port = int(self.MainWindow.Input_Port.toPlainText())
            username = self.MainWindow.Input_Username.toPlainText()
            password = self.MainWindow.Input_Password.toPlainText()
        except:
            self.Warning_Box('Login error', 'Can\'t get info for login.')
            return

        try:
            self.connection_id.connect(
                self.server_ip, self.server_port_connection)
        except:
            self.Warning_Box('Login error', 'Can\'t connect.')
            return
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Login error', 'Can\'t connect.')
            return

        message_user = 'USER '+username+'\r\n'
        self.connection_id.send(str.encode(message_user))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Login error', message)
            return

        message_pass = 'PASS '+password+'\r\n'
        self.connection_id.send(str.encode(message_pass))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Login error', message)
            return

        self.MainWindow.Input_IP.setReadOnly(True)
        self.MainWindow.Input_Port.setReadOnly(True)
        self.MainWindow.Input_Username.setReadOnly(True)
        self.MainWindow.Input_Password.setReadOnly(True)

        self.Refresh()

        pass

    def Logout(self):
        '''
        登出操作，即进行QUIT命令
        '''
        message_quit = 'QUIT\r\n'
        self.connection_id.send(str.encode(message_quit))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Logout error', message)
            return

        self.connection_id.close()
        if self.connection_listen != None:
            self.connection_listen.close()
        if self.connection_data != None:
            self.connection_data.close()
        self.MainWindow.Input_IP.setReadOnly(False)
        self.MainWindow.Input_Port.setReadOnly(False)
        self.MainWindow.Input_Username.setReadOnly(False)
        self.MainWindow.Input_Password.setReadOnly(False)
        pass

    def Refresh(self):
        '''
        刷新文件列表及将路径转换为当前路径，即进行LIST命令和PWD命令
        '''
        # LIST
        if self.Connect_Mode():
            return
        message_list = 'LIST\r\n'
        self.connection_id.send(str.encode(message_list))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Refresh error', message)
            return
        if self.Connect():
            return
        list_data = self.connection_data.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Refresh error', message)
            return
        data_list = list_data.split('\n')
        self.MainWindow.Show_File.setModel(self.file_model)
        self.file_model = QStandardItemModel()
        self.file_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.file_model.setHorizontalHeaderItem(
            1, QStandardItem('file/directory'))
        self.file_model.setHorizontalHeaderItem(
            2, QStandardItem('size'))
        self.file_model.setHorizontalHeaderItem(
            3, QStandardItem('last edit time'))
        for i in range(len(data_list)):
            data = data_list[i]
            data_info = data.split(' ')
            model_insert = []
            model_insert.append(QStandardItem(data_info[8]))
            if data_info[0][0] == '-':
                model_insert.append(QStandardItem('file'))
            else:
                model_insert.append(QStandardItem('directory'))
            model_insert.append(QStandardItem(data_info[4]))
            model_insert.append(QStandardItem(
                data_info[5]+' '+data_info[6]+' '+data_info[7]))
            self.file_model.insertRow(self.file_model.rowCount(), model_insert)

        # PWD
        message_pwd = 'PWD\r\n'
        self.connection_id.send(str.encode(message_pwd))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Refresh error', message)
            return
        current_dir = message.split(' ')[-1]
        self.MainWindow.Input_Path.setPlainText(current_dir)

        pass

    def Rename(self):
        '''
        重命名操作，即进行RNFR与RNTO命令
        '''
        path_name = self.MainWindow.Input_Pathname.toPlainText()
        new_path_name = self.MainWindow.Input_Newpathname.toPlainText()

        message_rnfr = 'RNFR '+path_name+'\r\n'
        self.connection_id.send(str.encode(message_rnfr))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('RNFR command error', message)
            return

        message_rnto = 'RNTO '+new_path_name+'\r\n'
        self.connection_id.send(str.encode(message_rnto))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Rename error', message)
            return

        pass

    def Download_File(self, num, client_file_path):
        '''
        下载文件
        参数：
            num：第几个下载任务
        '''
        try:
            file = open(client_file_path, 'wb+')
        except:
            self.Warning_Box('Download error', 'Can\'t open file.')
            return
        while(1):
            try:
                data_receive = self.connection_data.recv(socket_maxlen)
            except:
                self.Warning_Box('Download error', 'Download failed.')
                self.download_model.setItem(num, 1, QStandardItem("Failed"))
                return
            if data_receive == None or len(data_receive) == 0:
                self.download_model.setItem(num, 1, QStandardItem("Completed"))
                return
            file.write(data_receive)

    def Download(self):
        '''
        下载文件操作，即进行PORT(PASV)与RETR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        if self.Connect_Mode():
            return
        message_retr = 'RETR'+client_file_path+'\r\n'
        self.connection_id.send(str.encode(message_retr))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Download error', message)
            return
        if self.Connect():
            return
        file_info = client_file_path.split('/')
        download_file_info = []
        download_file_info.append(file_info[-1])
        download_file_info.append('downloading')
        self.download_model.insertRow(
            self.download_model.rowCount(), download_file_info)
        _thread.start_new_thread(
            self.Download_File, self.download_model.rowCount(), client_file_path)
        pass

    def Upload_File(self, num, client_file_path):
        '''
        上传文件
        参数：
            num：第几个下载任务
        '''
        try:
            file = open(client_file_path, 'rb')
        except:
            self.Warning_Box('Upload error', 'Can\'t open file.')
            return
        while(1):
            try:
                data_send = file.read(socket_maxlen)
            except:
                self.Warning_Box('Upload error', 'Read file failed.')
                self.upload_model.setItem(num, 1, QStandardItem("Failed"))
                return
            if data_send == None or len(data_send) == 0:
                self.upload_model.setItem(num, 1, QStandardItem("Completed"))
                return
            try:
                self.connection_data.send(data_send)
            except:
                self.Warning_Box('Upload error', 'File transport failed.')
                self.upload_model.setItem(num, 1, QStandardItem("Failed"))
                return

    def Upload(self):
        '''
        上传文件操作，即进行PORT(PASV)与STOR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        if self.Connect_Mode():
            return
        message_retr = 'STOR'+client_file_path+'\r\n'
        self.connection_id.send(str.encode(message_retr))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Upload error', message)
            return
        if self.Connect():
            return
        file_info = client_file_path.split('/')
        upload_file_info = []
        upload_file_info.append(file_info[-1])
        upload_file_info.append('uploading')
        self.upload_model.insertRow(
            self.upload_model.rowCount(), upload_file_info)
        _thread.start_new_thread(
            self.Upload_File, self.upload_model.rowCount(), client_file_path)
        pass

    def Makedir(self):
        '''
        新建文件夹操作，即进行MKD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        message_mkd = 'MKD '+path+'\r\n'
        self.connection_id.send(message_mkd)
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Makedir error', message)
            return
        pass

    def Changedir(self):
        '''
        更换工作目录操作，即进行CWD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        message_cwd = 'CWD '+path+'\r\n'
        self.connection_id.send(message_cwd)
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Changedir error', message)
            return
        pass

    def Portmode(self):
        '''
        更换传输模式为主动模式(PORT)，保存port参数，但不向server传输命令
        '''
        self.transport_mode = 0
        self.port_arg = int(self.MainWindow.Input_Port.toPlainText())
        pass

    def Pasvmode(self):
        '''
        更换传输模式为被动模式(PASV)，但不向server传输命令
        '''
        self.transport_mode = 1
        pass


if __name__ == '__main__':
    app = QApplication(sys.argv)
    Client = MyClient()
    sys.exit(app.exec_())
