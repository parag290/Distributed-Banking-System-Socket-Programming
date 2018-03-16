#ifndef CLIENTINFO_H_
#define CLIENTINFO_H_

#include <string>
using namespace std;
class ClientInfo
{
public:
	ClientInfo();

	string get_first_name();
	string get_last_name();
	string get_address();
	string get_phone_number();
	string get_email_id();
	string get_accountno();
	string get_userid();
	string get_account_type();


	void set_first_name(string fname);
	void set_last_name(string lname);
	void set_adddress(string addr);
	void set_phone_number(string ph);
	void set_email_id(string email);
	void set_accountno(string accnt);
	void set_userid(string user);
	void set_account_type(string accnt_type);

private:
	string first_name;
	string last_name;
	string address;
	string phone_number;
	string email_id;
	string accountno;
	string userid;
	string account_type;

};


#endif
