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
    get_logined = False
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
    download_list = []
    download_status = []
    upload_list = []
    upload_status = []

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
        hostsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        hostsocket.connect(('8.8.8.8', 80))
        self.client_ip = hostsocket.getsockname()[0]
        print(self.client_ip)
        pass

    def Init_Window(self):
        '''
        初始化图形化窗口内内容
        '''
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
        self.MainWindow.Show_File.setModel(self.file_model)
        self.MainWindow.Show_Download.setModel(self.download_model)
        self.MainWindow.Show_Upload.setModel(self.upload_model)

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
        self.MainWindow.Button_Removedir.clicked.connect(self.Removedir)
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
                return 1
        return 0

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
            port = int(self.MainWindow.Input_ModePort.toPlainText())
            self.connection_listen = socket.socket()
            self.connection_listen.bind((self.client_ip, port))
            self.connection_listen.listen(10)
            client_ip_re = re.match(
                r'([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})', self.client_ip)
            message_port = 'PORT '+client_ip_re[1]+','+client_ip_re[2]+','+client_ip_re[3] + \
                ','+client_ip_re[4]+',' + \
                str(port//256)+','+str(port % 256)+'\r\n'
            print(port, message_port)
            self.connection_id.send(str.encode(message_port))
            print(self.connection_id)
            try:
                message = self.connection_id.recv(
                    socket_maxlen).decode('UTF-8', 'strict')
                if self.Check_Invalid(message):
                    self.Warning_Box('Port command error', message)
                    return 1
                print(message)
            except:
                print("no message")
                pass
            print("end recv")
            print("begin listen")
        else:
            message_pasv = "PASV\r\n"
            self.connection_id.send(str.encode(message_pasv))
            message = self.connection_id.recv(
                socket_maxlen).decode('UTF-8', 'strict')
            if self.Check_Invalid(message):
                self.Warning_Box('PASV command error', message)
                return 1
            print(message)
            server_ip_port_re = re.match(
                r'.*[(]([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})[)].*', message)
            self.server_port_data = int(
                server_ip_port_re[5])*256+int(server_ip_port_re[6])
        return 0

    def Connect(self):
        '''
        与server连接
        返回：
            0成功 1失败
        '''
        print("begin connect")
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Port command error', message)
            return 1
        if self.transport_mode == 0:  # PORT
            try:
                self.connection_data, ip_port = self.connection_listen.accept()
            except:
                self.Warning_Box('Connect error', 'Can\'t listen connection')
                return 1
        else:  # PASV
            try:
                self.connection_data = socket.socket()
                print(self.server_ip, self.server_port_data)
                self.connection_data.connect(
                    (self.server_ip, self.server_port_data))
                print("connected", self.connection_data)
            except:
                self.Warning_Box('Connect error', 'Data connection failed')
                return 1
        return 0

    def Login(self):
        '''
        登陆操作，即进行USER与PASS命令
        '''
        if self.get_logined:
            self.Warning_Box('Login error', 'already logined.')
            return
        try:
            self.server_ip = self.MainWindow.Input_IP.toPlainText()
            self.server_port_connection = int(
                self.MainWindow.Input_Port.toPlainText())
            username = self.MainWindow.Input_Username.toPlainText()
            password = self.MainWindow.Input_Password.toPlainText()
        except:
            self.Warning_Box('Login error', 'Can\'t get info for login.')
            return

        try:
            self.connection_id = socket.socket()
            self.connection_id.connect(
                (self.server_ip, self.server_port_connection))
        except:
            print("connect error", self.server_ip, self.server_port_connection)
            print(self.server_ip)
            print(self.server_port_connection)
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

        self.get_logined = True

        pass

    def Logout(self):
        '''
        登出操作，即进行QUIT命令
        '''
        if not self.get_logined:
            self.Warning_Box('Logout error', 'hav\'t login')
            return
        message_quit = 'QUIT\r\n'
        self.connection_id.send(str.encode(message_quit))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Logout error', message)
            return

        self.connection_id.close()
        self.connection_id = None
        if self.connection_listen != None:
            self.connection_listen.close()
            self.connection_listen = None
        if self.connection_data != None:
            self.connection_data.close()
            self.connection_data = None
        self.MainWindow.Input_IP.setReadOnly(False)
        self.MainWindow.Input_Port.setReadOnly(False)
        self.MainWindow.Input_Username.setReadOnly(False)
        self.MainWindow.Input_Password.setReadOnly(False)

        self.get_logined = False
        pass

    def Refresh(self):
        '''
        刷新文件列表及将路径转换为当前路径，即进行LIST命令和PWD命令
        '''
        # LIST
        if self.Connect_Mode():
            return
        print("connect")
        message_list = 'LIST\r\n'
        self.connection_id.send(str.encode(message_list))
        if self.Connect():
            return
        # message = self.connection_id.recv(
        #     socket_maxlen).decode('UTF-8', 'strict')
        # if self.Check_Invalid(message):
        #     self.Warning_Box('Refresh error', message)
        #     return
        list_data = self.connection_data.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        print(len(list_data))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        print(message)
        if self.Check_Invalid(message):
            self.Warning_Box('Refresh error', message)
            return
        print(list_data)
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
        # print(list_data)
        # print(data_list)
        # print(len(data_list))
        for i in range(len(data_list)):
            data = data_list[i]
            data_info = data.split()
            if len(data_info) != 9:
                continue
            # print(data_info)
            # print(len(data_info))
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
        self.MainWindow.Show_File.setModel(self.file_model)

        # PWD
        message_pwd = 'PWD\r\n'
        self.connection_id.send(str.encode(message_pwd))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Refresh error', message)
            return
        current_dir = message.split()[-1]
        current_dir = current_dir[1:-1]
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

    def Refresh_Download_View(self):
        '''
        刷新下载文件列表
        '''
        self.download_model = QStandardItemModel()
        self.download_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.download_model.setHorizontalHeaderItem(
            1, QStandardItem('status'))
        self.download_model.setRowCount(len(self.download_list))
        for i in range(len(self.download_list)):
            self.download_model.setItem(
                i, 0, QStandardItem(self.download_list[i]))
            self.download_model.setItem(
                i, 1, QStandardItem(self.download_status[i]))
        self.MainWindow.Show_Download.setModel(self.download_model)

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
            self.download_status[num] = 'failed'
            self.Refresh_Download_View()
            return
        print("begin download")
        while(1):
            # print("download")
            try:
                # print("download begin1")
                data_receive = self.connection_data.recv(socket_maxlen)
                # print("download end1")
            except:
                self.Warning_Box('Download error', 'Download failed.')
                self.download_status[num] = 'failed'
                self.Refresh_Download_View()
                message = self.connection_id.recv(
                    socket_maxlen).decode('UTF-8', 'strict')
                print("message download", message)
                if self.Check_Invalid(message):
                    self.Warning_Box('Download error', message)
                    return
                # self.download_model.setItem(num, 1, QStandardItem("Failed"))
                # self.MainWindow.Show_Download.setModel(self.download_model)
                return
            if data_receive == None or len(data_receive) == 0:
                self.download_status[num] = 'complete'
                self.Refresh_Download_View()
                message = self.connection_id.recv(
                    socket_maxlen).decode('UTF-8', 'strict')
                print("message download", message)
                if self.Check_Invalid(message):
                    self.Warning_Box('Download error', message)
                    return
                # print("before setitem")
                # self.download_model.setItem(num, 1, QStandardItem("Completed"))
                # print("before setmodel")
                # self.MainWindow.Show_Download.setModel(self.download_model)
                # print("end setmodel")
                return
            file.write(data_receive)
            # print(len(data_receive))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        print("message download", message)
        if self.Check_Invalid(message):
            self.Warning_Box('Download error', message)
            return

    def Download(self):
        '''
        下载文件操作，即进行PORT(PASV)与RETR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        if self.Connect_Mode():
            return
        message_retr = 'RETR '+server_file_path+'\r\n'
        print("message_retr ", message_retr)
        self.connection_id.send(str.encode(message_retr))
        print("message_retr send")
        if self.Connect():
            return
        # message = self.connection_id.recv(
        #     socket_maxlen).decode('UTF-8', 'strict')
        # print("message download", message)
        # if self.Check_Invalid(message):
        #     self.Warning_Box('Download error', message)
        #     return
        file_info = client_file_path.split('/')
        download_file_info = []
        download_file_info.append(QStandardItem(file_info[-1]))
        download_file_info.append(QStandardItem('downloading'))
        self.download_list.append(file_info[-1])
        self.download_status.append('downloading')
        self.download_model.insertRow(
            self.download_model.rowCount(), download_file_info)
        _thread.start_new_thread(
            self.Download_File, (self.download_model.rowCount()-1, client_file_path))
        pass

    def Refresh_Upload_View(self):
        '''
        刷新上传文件列表
        '''
        self.upload_model = QStandardItemModel()
        self.upload_model.setHorizontalHeaderItem(
            0, QStandardItem('name'))
        self.upload_model.setHorizontalHeaderItem(
            1, QStandardItem('status'))
        self.upload_model.setRowCount(len(self.upload_list))
        for i in range(len(self.upload_list)):
            self.upload_model.setItem(
                i, 0, QStandardItem(self.upload_list[i]))
            self.upload_model.setItem(
                i, 1, QStandardItem(self.upload_status[i]))
        self.MainWindow.Show_Upload.setModel(self.upload_model)

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
            self.upload_model.setItem(num, 1, QStandardItem("Failed"))
            self.MainWindow.Show_Upload.setModel(self.upload_model)
            return
        print("begin upload")
        while(1):
            print("layer begin")
            try:
                data_send = file.read(socket_maxlen)
            except:
                self.Warning_Box('Upload error', 'Read file failed.')
                self.upload_status[num] = 'failed'
                self.Refresh_Upload_View()
                # self.upload_model.setItem(num, 1, QStandardItem("Failed"))
                # self.MainWindow.Show_Upload.setModel(self.upload_model)
                return
            print("layer 1")
            if data_send == None or len(data_send) == 0:
                self.upload_status[num] = 'complete'
                self.Refresh_Upload_View()
                self.connection_data.close()
                self.connection_data = None
                message = self.connection_id.recv(
                    socket_maxlen).decode('UTF-8', 'strict')
                print("message", message)
                if self.Check_Invalid(message):
                    self.Warning_Box('Upload error', message)
                    return
                # self.upload_model.setItem(num, 1, QStandardItem("Completed"))
                # self.MainWindow.Show_Upload.setModel(self.upload_model)
                return
            print("layer 2")
            print(len(data_send))
            try:
                self.connection_data.send(data_send)
            except:
                print("mid etrpt")
                self.Warning_Box('Upload error', 'File transport failed.')
                self.upload_status[num] = 'failed'
                self.Refresh_Upload_View()
                # self.upload_model.setItem(num, 1, QStandardItem("Failed"))
                # self.MainWindow.Show_Upload.setModel(self.upload_model)
                message = self.connection_id.recv(
                    socket_maxlen).decode('UTF-8', 'strict')
                print("message upload", message)
                if self.Check_Invalid(message):
                    self.Warning_Box('Download error', message)
                    return
                return
            print("layer end")
        print("upload end")
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        print("message upload", message)
        if self.Check_Invalid(message):
            self.Warning_Box('Download error', message)
            return

    def Upload(self):
        '''
        上传文件操作，即进行PORT(PASV)与STOR命令
        '''
        client_file_path = self.MainWindow.Input_Filepath.toPlainText()
        server_file_path = self.MainWindow.Input_Path.toPlainText()
        if self.Connect_Mode():
            return
        message_stor = 'STOR '+server_file_path+'\r\n'
        print("message_stor", message_stor)
        self.connection_id.send(str.encode(message_stor))
        if self.Connect():
            return
        # print("message_stor sent")
        # message = self.connection_id.recv(
        #     socket_maxlen).decode('UTF-8', 'strict')
        # print("message receive", message)
        # if self.Check_Invalid(message):
        #     self.Warning_Box('Upload error', message)
        #     return
        file_info = server_file_path.split('/')
        upload_file_info = []
        upload_file_info.append(QStandardItem(file_info[-1]))
        upload_file_info.append(QStandardItem('uploading'))
        self.upload_list.append(file_info[-1])
        self.upload_status.append('uploading')
        self.upload_model.insertRow(
            self.upload_model.rowCount(), upload_file_info)
        _thread.start_new_thread(
            self.Upload_File, (self.upload_model.rowCount()-1, client_file_path))
        pass

    def Makedir(self):
        '''
        新建文件夹操作，即进行MKD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        if len(path) == 0:
            self.Warning_Box('RMD error', 'path error')
            return
        path = self.MainWindow.Input_Path.toPlainText()
        message_mkd = 'MKD '+path+'\r\n'
        self.connection_id.send(str.encode(message_mkd))
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
        if len(path) == 0:
            self.Warning_Box('RMD error', 'path error')
            return
        path = self.MainWindow.Input_Path.toPlainText()
        message_cwd = 'CWD '+path+'\r\n'
        self.connection_id.send(str.encode(message_cwd))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Changedir error', message)
            return
        print("message cwd:", message)
        print("begin refresh")
        self.Refresh()
        pass

    def Removedir(self):
        '''
        删除目录，即进行RMD命令
        '''
        path = self.MainWindow.Input_Path.toPlainText()
        if len(path) == 0:
            self.Warning_Box('RMD error', 'path error')
            return
        message_cwd = 'RMD '+path+'\r\n'
        self.connection_id.send(str.encode(message_cwd))
        message = self.connection_id.recv(
            socket_maxlen).decode('UTF-8', 'strict')
        if self.Check_Invalid(message):
            self.Warning_Box('Removedir error', message)
            return

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
