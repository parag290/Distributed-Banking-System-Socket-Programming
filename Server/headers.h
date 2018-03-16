
using namespace std;
using namespace pqxx;
#include "Login.h"



void teller_init(int,Login& obj_login);
void getLoginInfo(int,Login& obj_login);
//void makeDeposit(int,Login& obj_login);
//void makeWithdrawal(int,Login& obj_login);
//void checkBalance();
//void print_MiniStatement();
int socketconnection() throw (string);
void admin_init(int,Login& obj_login);
void open_account(int socket, Login& objlogin);
void customer_init(int socket, Login& objlogin);
void send_email(const char *email_to, const char *email_body,const char* email_subject);
