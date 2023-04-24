## 目录树：  
.  
├── Readme.txt  
├── app  
│   ├── demo  
│   │   ├── Makefile  
│   │   ├── client  
│   │   ├── client.cpp  
│   │   ├── testfile.txt  
│   │   └── testfile3.txt  
│   └── stress_test  
│       ├── Makefile  
│       ├── stress  
│       ├── stress.cpp  
│       └── testfile.txt  
├── bin  
│   └── server  
├── build  
│   └── Makefile  
├── code  
│   ├── common  
│   │   └── common.h  
│   ├── ftp_conn  
│   │   ├── ftp_conn.cpp  
│   │   └── ftp_conn.h  
│   ├── ftp_server  
│   │   ├── epoller.cpp  
│   │   ├── epoller.h  
│   │   ├── ftp_server.cpp  
│   │   └── ftp_server.h  
│   ├── main.cpp  
│   ├── pool  
│   │   ├── locker.h  
│   │   ├── thread_pool.cpp  
│   │   └── thread_pool.h  
│   ├── progress_bar  
│   │   ├── prog_bar.cpp  
│   │   └── prog_bar.h  
│   ├── timer  
│   │   ├── heap_timer.cpp  
│   │   └── heap_timer.h  
│   └── type  
│       └── basic_type.h  
└── data  
    ├── testfile.txt  
    ├── testfile2.txt  
    └── testfile3.txt  
  
  
## Linux/C++实现文件下载服务器：  
  
服务端：  
*    build文件夹为make文件，执行make命令可进行编译  
*    bin文件夹下为服务端运行程序，运行命令：./server  
  
客户端：  
*    文件路径：app/demo/  
*    可执行make命令进行编译  
*    文件下载命令：./client 192.168.43.254 9876 testfile.txt  
*    注：需要将ip地址需更换为./server程序运行的主机ip  

压力测试程序：  
*    路径：app/stress/  
*    可执行make命令进行编译  
*    命令：./stress 192.168.43.254 9876 100  
*    其中100表示客户端的数量  
*    注：需要将ip地址需更换为./server程序运行的主机ip  

服务端功能:  
 *   利用epoll实现I/O多路复用和线程池实现Reactor高并发模型;  
 *   利用了小根堆定时器实现了非活动连接的关闭；
 *   实现了文件的断点续传；
    
客户端功能：  
*    显示下载进度条；
