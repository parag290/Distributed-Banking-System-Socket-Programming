#ifndef LOGIN_H_
#define LOGIN_H_

#include <pqxx/pqxx>
#include<postgresql/libpq-fe.h>
using namespace std;


class Login{
public:

	Login();
	string get_username();
	string get_password();
	string get_type();

	void set_username(string);
	void set_password(string);
	void set_type(string);
	string get_account_no();
	string get_process_id();
	pg_conn* get_connection();

	void set_user_Id(string id);
	void set_account_no(string account);
	void set_process_id(string processid);
	void set_connection(pg_conn* conn);

private:
	string username;
	string password;
	string account_no;
	string process_id;
	string type;
	pg_conn *conn;

};

#endif
