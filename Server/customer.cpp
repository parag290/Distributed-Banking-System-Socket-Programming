#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pqxx/pqxx>
#include<string>
#include<postgresql/libpq-fe.h>
#include "headers.h"
#include "Login.h"
#include <math.h>
#include "ClientInfo.h"
#include <sstream>
#include <iomanip>


using namespace std;
using namespace pqxx;


void balance_check(int socket, Login objlogin);
void make_statment(int socket, Login objlogin);
void change_password(int socket, Login objlogin);
void make_transfer(int socket, Login objlogin);

enum Buffer_Size{
	BUFFER_64 = 64, BUFFER_256 = 256, BUFFER_512 = 512, BUFFER_1024 = 2048
};

enum ACTIONS{
	LOGOUT = 0,TRANSFER =3, PASS_CHANGE = 4, STATEMENT = 2,BAL_CHECK = 1
};

enum CREDENTIALS {
	CORRECT = 1, INCORRECT = 2, INSUFFICIENT = 3
};

void customer_init(int socket, Login& objlogin)
{
	char msg[BUFFER_64];

		while(1){
			if(recv(socket, &msg, sizeof(msg),0) < 0)
				throw string("*****error in teller****");
			else
				switch (atoi(msg)) {
					case TRANSFER:
						make_transfer(socket, objlogin);
						break;
					case PASS_CHANGE:
						change_password(socket, objlogin);
						break;
					case STATEMENT:
						make_statment(socket, objlogin);
						break;
					case BAL_CHECK:
						balance_check(socket, objlogin);
						break;
					case LOGOUT:
					return;
					break;
					default:
						return;
						break;
				}

		}
}


void balance_check(int socket, Login objlogin){

	ClientInfo objClientInfo;
	string sqll;
	PGresult *res;
	float exist_bal;
	char sendmsg[BUFFER_1024];

						sqll = string("select account_no from bank_login where user_id=")+string("'")+objlogin.get_username()+string("'");
									cout << "sqll is " << sqll << endl;
									res = PQexec(objlogin.get_connection(),sqll.c_str());
									objClientInfo.set_accountno(PQgetvalue(res,0,0));
									cout<<"accno = "<<PQgetvalue(res,0,0)<<"\n";
									PQclear(res);

								sqll = string("select balance from bank_account where account_no=")+string("'")+objClientInfo.get_accountno()+string("'");
								cout << "sqll is " << sqll << endl;
								res = PQexec(objlogin.get_connection(),sqll.c_str());
								exist_bal = atof(PQgetvalue(res,0,0));
								cout<<"Balance = "<<exist_bal<<"\n";

								stringstream stream;
								stream << fixed << setprecision(2) << exist_bal;
								string bal = stream.str();
								memset(&sendmsg, 0, sizeof(sendmsg));
								strcpy(sendmsg,bal.c_str());
								if (send(socket, &sendmsg, sizeof(sendmsg), 0) < 0){
									cout << "Balance not sent\n" << endl;
											}

}


void make_statment(int socket, Login objlogin){

	ClientInfo objClientInfo;
	string sqll;
	PGresult *res;
	float exist_bal;
	string data;
	char msg[BUFFER_1024];


	sqll = string("select account_no from bank_login where user_id=")+string("'")+objlogin.get_username()+string("'");
										cout << "sqll is " << sqll << endl;
										res = PQexec(objlogin.get_connection(),sqll.c_str());
										objClientInfo.set_accountno(PQgetvalue(res,0,0));
										cout<<"accno = "<<PQgetvalue(res,0,0)<<"\n";
										PQclear(res);


					sqll = string("select trans_date,trans_details,balance from bank_transaction where account_no=")+string("'")+objClientInfo.get_accountno()+"' order by trans_date desc";
					cout << "sqll is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());

					int rows = PQntuples(res);

					if(rows==0){
						if (send(socket, "4", sizeof("4"), 0) < 0){
							cout << "Balance not sent\n" << endl;
									}

					}

					for(int i=0; i<rows; i++) {

					printf("%s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1), PQgetvalue(res, i, 2));

					if(i!=0){
						data += string("/");
					}

					data += PQgetvalue(res, i, 0)+string("/")+PQgetvalue(res, i, 1)+string("/")+PQgetvalue(res, i, 2);

					}

					memset(&msg, 0, sizeof(msg));
					strcpy(msg,data.c_str());

					if (send(socket, &msg, sizeof(msg), 0) < 0){
							cout << "Balance not sent\n" << endl;
														}



	}


void change_password(int socket, Login objlogin){


	ClientInfo objClientInfo;
	string sqll;
	PGresult *res;
	string data;
	char msg[BUFFER_256];

	while(1){
				memset(&msg, 0, sizeof(msg));
				if(recv(socket, &msg, sizeof(msg),0) < 0)
					throw string("*****error in depositing by teller****");
				else{

				string oldpass;
				string newpass;

					char *point;
						point = strtok(msg,"}");
						oldpass = point;
						point = strtok(NULL, "}");
						newpass = point;
						point = strtok(NULL, "}");

						sqll = string("select password from bank_login where user_id=")+string("'")+objlogin.get_username()+string("'");
						cout << "sqll is " << sqll << endl;
						res = PQexec(objlogin.get_connection(),sqll.c_str());


						if(strcmp(oldpass.c_str(),PQgetvalue(res, 0, 0))==0){

						sqll = string("update bank_login set password=")+string("'")+newpass +string("'")+
												string("where user_id=")+string("'")+objlogin.get_username()+string("'");
						cout << "sqll1 is " << sqll << endl;
						PQclear(res);

						res = PQexec(objlogin.get_connection(),sqll.c_str());


						if (PQresultStatus(res) != PGRES_COMMAND_OK) {

						cout << "password not changed" << endl;
										    }
						PQclear(res);

						if (send(socket, "1", sizeof("1"), 0) < 0){
							cout << "New Password not sent\n" << endl;
									}

						break;

						}
						else{

							if (send(socket, "2", sizeof("2"), 0) < 0){
									cout << "Password Incorrect\n" << endl;
											}


						}


				}

}
}


void make_transfer(int socket, Login objlogin){

	ClientInfo objClientInfo;
		string sqll;
		PGresult *res;
		string data;
		char msg[BUFFER_256];

		while(1){
					memset(&msg, 0, sizeof(msg));
					if(recv(socket, &msg, sizeof(msg),0) < 0)
						throw string("*****error in depositing by teller****");

					else{

					string recvacc;
					string transamt;

					char *point;
					point = strtok(msg,"/");
					recvacc = point;
					point = strtok(NULL, "/");
					transamt = point;
					point = strtok(NULL, "/");

					sqll = string("select account_no from bank_acc_details where account_no=")+string("'")+recvacc+string("'");
					cout << "sqll is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());
					int rows = PQntuples(res);

						if(rows==0){
							if (send(socket, "2", sizeof("2"), 0) < 0){
								cout << "Account Incorrect\n" << endl;
						}
							PQclear(res);

					}
					else{

						float tamt;

						tamt = floorf(atof(transamt.c_str())*100)/100.0;
										   cout<<"Balance after rounding = "<<tamt<<"\n";

										   stringstream stream;
										   stream << fixed << setprecision(2) << tamt;
										   string bal = stream.str();

										sqll = string("select account_no from bank_login where user_id=")+string("'")+objlogin.get_username()+string("'");
										cout << "sqll is " << sqll << endl;
										res = PQexec(objlogin.get_connection(),sqll.c_str());
										string sendacc = PQgetvalue(res, 0, 0);
										PQclear(res);


											sqll = string("insert into bank_transfer (sender_account,receiver_account,amount,state) VALUES (")+ string("'")+sendacc+string("','")
													+recvacc+string("','")+bal+string("','")+"PENDING"+string("')");

											cout << "insert sql is " << sqll << endl;
											res = PQexec(objlogin.get_connection(),sqll.c_str());

											if (PQresultStatus(res) != PGRES_COMMAND_OK) {

											    cout << "cannot insert" << endl;
											    }

											PQclear(res);
											if (send(socket, "1", sizeof("1"), 0) < 0){
												cout << "Transfer requested\n" << endl;
																}

											break;

						}

					}

}
}

