#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "Connection.h"
using namespace std;

//ʵ�����ӳع���ģ��
class ConnectionPool {
public:
	static ConnectionPool* getConnectionPool();//��ȡ���ӳض���ʵ��
	shared_ptr<Connection> getConnection();//���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
private:
	ConnectionPool();//���� ���캯��˽�л�

	bool loadConfigFile();//�������ļ��м���������

	void produceConnectionTask();//�����ڶ������߳��У�ר�Ÿ�������������
	void scannerConnectionTask();//�����ڶ������߳��У�ר�Ÿ���������ӵĻ���

	string _ip;//MySQL��IP��ַ
	unsigned short _port;//MySQL�Ķ˿ں� 3306
	string _username;//MySQL�ĵ�¼�û���
	string _password;//MySQL�ĵ�¼����
	string _dbname;//���ӵ����ݿ�����
	int _initSize;//���ӳصĳ�ʼ������
	int _maxSize;//���ӳص����������
	int _maxIdleTime;//���ӳ��������ӵ�������ʱ��
	int _connectionTimeOut;//���ӳػ�ȡ���ӵĳ�ʱʱ��

	queue<Connection*> _connectionQue;//�洢MySQL���ӵĶ���
	mutex _queueMutex;//ά�����Ӷ��е��̰߳�ȫ������
	atomic_int _connectionCnt;//��¼������������connection���ӵ�������
	condition_variable cv;//���������������������������̺߳������̵߳�ͨ��
};