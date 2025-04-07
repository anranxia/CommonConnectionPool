#include <iostream>
#include "Connection.h"
#include "CommonConnectionPool.h"
using namespace std;

int main()
{
	clock_t begin = clock();
	//ConnectionPool* cp = ConnectionPool::getConnectionPool();
	Connection conn;
	conn.connect("127.0.0.1", 3306, "root", "123456", "chat");

	thread t1([&]() {
		for (int i = 0; i < 250; ++i) {
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
			conn.update(sql);

			//shared_ptr<Connection> sp = cp->getConnection();
			//char sql[1024] = { 0 };
			//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			//sp->update(sql);
		} });
	thread t2([&]() {
		for (int i = 0; i < 250; ++i) {
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
			conn.update(sql);

			//shared_ptr<Connection> sp = cp->getConnection();
			//char sql[1024] = { 0 };
			//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			//sp->update(sql);
		} });
	thread t3([&]() {
		for (int i = 0; i < 250; ++i) {
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
			conn.update(sql);

			//shared_ptr<Connection> sp = cp->getConnection();
			//char sql[1024] = { 0 };
			//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			//sp->update(sql);
		} });
	thread t4([&]() {
		for (int i = 0; i < 250; ++i) {
			Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
			conn.update(sql);

			//shared_ptr<Connection> sp = cp->getConnection();
			//char sql[1024] = { 0 };
			//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			//sp->update(sql);
		} });

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
	return 0;

#if 0
	for (int i = 0; i < 1000; ++i) {
		//Connection conn;
		//char sql[1024] = { 0 };
		//sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
		//conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
		//conn.update(sql);

		shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
		sp->update(sql);
	}
#endif
}