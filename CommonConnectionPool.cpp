﻿#include "CommonConnectionPool.h"
#include "public.h"

//线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool;//对于静态局部变量的初始化由编译器自动进行lock和unlock
	return &pool;
}

//给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty()) {
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeOut))) {
			if (_connectionQue.empty()) {
				LOG("获取空闲连接超时了...获取连接失败！");
				return nullptr;
			}
		}

	}

	/*shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用connection的析构函数，
	connection就被close掉了，这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue中*/
	shared_ptr<Connection> sp(_connectionQue.front(),
		[&](Connection* pcon) {
			//这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();//刷新进入空闲状态的时刻
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();//消费完连接以后，通知生产者线程检察一下，如果队列为空了，赶紧生产连接

	return sp;
}

//从配置文件中加载配置项
bool ConnectionPool::loadConfigFile() {
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr) {
		LOG("mysql.ini file is not exist!");
		return false;
	}

	while (!feof(pf)) {
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;
		int idx = str.find('=', 0);
		if (idx == -1) {//无效的配置项
			continue;
		}

		int endidx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip") {
			_ip = value;
		}
		else if (key == "port") {
			_port = atoi(value.c_str());
		}
		else if (key == "username") {
			_username = value;
		}
		else if (key == "password") {
			_password = value;
		}
		else if (key == "dbname") {
			_dbname = value;
		}
		else if (key == "initSize") {
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut") {
			_connectionTimeOut = atoi(value.c_str());
		}
	}
	return true;
}

//运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			cv.wait(lock);//队列不空，此处生产线程进入等待状态
		}

		//连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//刷新进入空闲状态的时刻
			_connectionQue.push(p);
			_connectionCnt++;
		}

		cv.notify_all();//通知消费者线程，可以消费连接了
	}
}

//运行在独立的线程中，专门负责空闲连接的回收
void ConnectionPool::scannerConnectionTask()
{
	for (;;) {
		//通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		//扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime*1000)) {
				_connectionQue.pop();
				_connectionCnt--;
				delete p;//释放连接
			}
			else {
				break;//队头的连接没有超过_maxIdleTime，其他连接肯定没有
			}
		}
	}
}

//连接池的构造函数
ConnectionPool::ConnectionPool() {
	//加载配置项
	if (!loadConfigFile()) {
		return;
	}

	//创建初始数量的连接
	for (int i = 0; i < _initSize; i++) {
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();//刷新进入空闲状态的时刻
		_connectionQue.push(p);
		_connectionCnt++;
	}

	//启动一个新的线程，作为连接的生产者
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	//启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行空闲连接的回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}