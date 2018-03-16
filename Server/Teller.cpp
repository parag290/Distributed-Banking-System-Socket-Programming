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
#include <ctime>




using namespace std;
using namespace pqxx;



enum Buffer_Size{
	BUFFER_64 = 64,BUFFER_128 = 128, BUFFER_256 = 256, BUFFER_512 = 512, BUFFER_1024 = 1024
};

enum ACTIONS{
	LOGOUT = 0,OPENACCNT =1, CLOSEACCNT = 2, DEPOSIT = 3,WITHDRAW = 4, CUSTOMER_DETAIL = 5, MODIFY_DETAIL = 6
};

enum CREDENTIALS {
	CORRECT = 1, INCORRECT = 2, INSUFFICIENT = 3
};

enum MODIFY{
	MODIFY_ADDRESS = 1, MODIFY_PHN = 2, MODIFY_EMAIL = 3
};

void makeDeposit(int socket, Login& obj_login);
void makeWithdrawal(int,Login& obj_login);
int depositAmount(string account_no, string amount,Login& obj_login);
int withdrawAmount(string account_no, string amount,Login& obj_login);
void open_account(int socket, Login& objlogin);
void delimiting_string(char* message, ClientInfo& objclientInfo);
void close_account(int socket,Login obj_login);
void display_customer_detail(int socket, Login& objlogin);
void modify_customer_Details(int socket, Login& objlogin);




void teller_init(int socket, Login& objlogin)
{
	char msg[BUFFER_64];

		while(1){
			memset(&msg,0,sizeof(msg));
			if(recv(socket, &msg, sizeof(msg),0) < 0)
				throw string("*****error in teller****");
			else
				switch (atoi(msg)) {
					case OPENACCNT:
						open_account(socket, objlogin);
						break;
					case CLOSEACCNT:
						close_account(socket,objlogin);
						break;
					case DEPOSIT:
						makeDeposit(socket, objlogin);
						break;
					case WITHDRAW:
						makeWithdrawal(socket,objlogin);
						break;
					case CUSTOMER_DETAIL:
						display_customer_detail(socket, objlogin);
						break;
					case MODIFY_DETAIL:
						modify_customer_Details(socket,objlogin);
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


void makeDeposit(int socket, Login& obj_login)
{
	char msg[BUFFER_256];
	while(1){
		memset(&msg, 0, sizeof(msg));
		if(recv(socket, &msg, sizeof(msg),0) < 0)
			throw string("*****error in depositing by teller****");
		else{
			char *point;
			string amount, account_no;
			point = strtok(msg, "/");
			amount = point;
			point = strtok(NULL, "/");
			account_no = point;

			int deposit = depositAmount(account_no,amount,obj_login);
			switch (deposit) {
				case CORRECT:
				if (send(socket, "1", sizeof("1"), 0) < 0) {
					cout << "Can't send back to client\n" << endl;
					close (socket);
					return;
				}
				return;
					break;
				case INCORRECT:
				if (send(socket, "2", sizeof("2"), 0) < 0) {
					cout << "Can't send back to client\n" << endl;
					close(socket);
					return;
				}
					break;
			}

		}
	}
}




void makeWithdrawal(int socket,Login& obj_login)
{

	char msg[BUFFER_256];
	while (1) {
		memset(&msg, 0, sizeof(msg));
		if (recv(socket, &msg, sizeof(msg), 0) < 0)
			throw string("*****error in withdrawing by teller****");
		else {
			char *point;
			string amount, account_no;
			point = strtok(msg, "/");
			amount = point;
			point = strtok(NULL, "/");
			account_no = point;

			int validateAccnt = withdrawAmount(account_no,amount,obj_login);
			switch (validateAccnt) {
			case CORRECT:
				if (send(socket, "1", sizeof("1"), 0) < 0) {
					cout << "Can't send back to client\n" << endl;
					close(socket);
					return;
				}
				return;
				break;
			case INCORRECT:
				if (send(socket, "2", sizeof("2"), 0) < 0) {
					cout << "Can't send back to client\n" << endl;
					close(socket);
					return;
				}
				break;
			case INSUFFICIENT:
				if (send(socket, "3", sizeof("3"), 0) < 0) {
					cout << "Insufficient Balance to withdraw\n" << endl;
					close(socket);
					//return;
				}
				break;
			}

		}
	}

}


void print_MiniStatement()
{
	cout<<" Your statement is :"<<endl;
}

void checkBalance()
{
	cout<<"The balance in your account :"<<endl;

}


int depositAmount(string account_no, string amount,Login& obj_login)
{

	PGresult *res;
	string sqll;
	//time_t  timev;
		//compare with database here
		double exist_bal;
		sqll = string("select COUNT(*) from bank_acc_details where account_no=")+string("'")+account_no+string("'")+"and type='C'";
		cout << "sqll is " << sqll << endl;
		res = PQexec(obj_login.get_connection(),sqll.c_str());


		if(atof(PQgetvalue(res,0,0))==0){
						return INCORRECT;
						PQclear(res);
		}
		   else{
			   sqll = string("select balance from bank_account where account_no=")+string("'")+account_no+string("'");
			   cout << "sqll is " << sqll << endl;
			   res = PQexec(obj_login.get_connection(),sqll.c_str());
			   exist_bal = atof(PQgetvalue(res,0,0));
			   exist_bal += atof(amount.c_str());

			   PGresult *res1;
			   			//compare with database here

			   sqll = string("update bank_account set balance=")+string("'")+to_string(exist_bal) +string("'")+
					string("where account_no=")+string("'")+account_no+string("'");
			   			cout << "sqll1 is " << sqll << endl;
			   			res = PQexec(obj_login.get_connection(),sqll.c_str());
			   			PQclear(res);

			   			//update transaction table

			   		 cout << "insert sql is " << sqll << endl;
			   		time_t result = time(&result);
			   sqll = string("insert into bank_transaction (account_no, trans_date, trans_details, balance) VALUES (") + string("'") +
					   account_no+ string("','") + asctime(localtime(&result)) +string("','")+ "Deposit $"+amount+string("','")+to_string(exist_bal)+string("')");
					   //+ account_no + string("','") + time(&timev)+string("','")+ "Deposit"+string("','")+to_string(exist_bal)+string("')");

			  cout << "insert sql is " << sqll << endl;
			  res = PQexec(obj_login.get_connection(),sqll.c_str());

			   			if (PQresultStatus(res) != PGRES_COMMAND_OK) {

			   			    cout << "cannot insert" << endl;
			   			    }

			   			PQclear(res);


			   				   return CORRECT;


		   }

}

int withdrawAmount(string account_no, string amount,Login& obj_login){

	PGresult *res;
	string sqll;
//compare with database here
float exist_bal;
 sqll = string("select COUNT(*) from bank_acc_details where account_no=")+string("'")+account_no+string("'")+"and type='C'";
cout << "sqll is " << sqll << endl;
res = PQexec(obj_login.get_connection(),sqll.c_str());

if(atof(PQgetvalue(res,0,0))==0){
	return INCORRECT;
	PQclear(res);
}

   else{

	    sqll = string("select balance from bank_account where account_no=")+string("'")+account_no+string("'");
	   			cout << "sqll is " << sqll << endl;
	   			res = PQexec(obj_login.get_connection(),sqll.c_str());

	   exist_bal = atof(PQgetvalue(res,0,0));


	   cout<<"Balance = "<<exist_bal<<"\n";
	   cout<<"Amount requested = "<<atof(amount.c_str())<<"\n";
	   cout<<"Amount requested = "<<floorf(atof(amount.c_str())*100)/100.0<<"\n";
	   exist_bal -= floorf(atof(amount.c_str())*100)/100.0;
	   cout<<"Balance after rounding = "<<exist_bal<<"\n";

	   stringstream stream;
	   stream << fixed << setprecision(2) << exist_bal;
	   string bal = stream.str();

	   if(exist_bal<0){
	   					 return INSUFFICIENT;
	   				 }
	   PQclear(res);


	 sqll = string("update bank_account set balance=")+string("'")+bal+string("'")+
			string("where account_no=")+string("'")+account_no+string("'");
	   			cout << "sqll1 is " << sqll << endl;
	   	res = PQexec(obj_login.get_connection(),sqll.c_str());

	   	PQclear(res);
	   			 cout << "insert sql is " << sqll << endl;
	   						   		time_t result = time(&result);
	   						   sqll = string("insert into bank_transaction (account_no, trans_date, trans_details, balance) VALUES (") + string("'") +
	   								   account_no+ string("','") + asctime(localtime(&result)) +string("','")+ "Withdraw $"+amount+string("','")+to_string(exist_bal)+string("')");
	   								   //+ account_no + string("','") + time(&timev)+string("','")+ "Deposit"+string("','")+to_string(exist_bal)+string("')");

	   						  cout << "insert sql is " << sqll << endl;
	   						  res = PQexec(obj_login.get_connection(),sqll.c_str());

	   						   			if (PQresultStatus(res) != PGRES_COMMAND_OK) {

	   						   			    cout << "cannot insert" << endl;
	   						   			    }

	   						   			PQclear(res);

	   						   		 return CORRECT;

   }

}


void open_account(int socket, Login& objlogin)
{
	ClientInfo objClientInfo;
		char msg[BUFFER_1024];
		char account_no[BUFFER_256];
		string sqll;
		PGresult *res;
		string typ;

			memset(&msg, 0, sizeof(msg));
			if(recv(socket, &msg, sizeof(msg),0) < 0)
				throw string("*****error in creating client account by teller****");
			else{

				delimiting_string(msg,objClientInfo);

				//Query to create account and store the account not back into client Info object

					sqll = string("select NEXTVAL('account_sequence')");
					 cout << "sql is " << sqll << endl;
					   	   	 res = PQexec(objlogin.get_connection(),sqll.c_str());

					   	   	 string result = PQgetvalue(res,0,0);
					   	   	 cout<< "result is "<<result<<endl;
					   	  objClientInfo.set_accountno(string("SJSU")+result);

					   	  PQclear(res);

			if(objlogin.get_type()=="T"){
				typ="C";
			}
			else{
				typ="T";
				objClientInfo.set_adddress("");
			}


			sqll = string("insert into bank_acc_details (account_no, first_name, last_name, address, phone_number, email_id, type, account_type) VALUES (")+
					string("'")+objClientInfo.get_accountno()+string("','")+objClientInfo.get_first_name()+string("','")+ objClientInfo.get_last_name()+string("','")+
					objClientInfo.get_address()+string("','")+objClientInfo.get_phone_number()+string("','")+objClientInfo.get_email_id()+string("','")+
					typ+string("','")+objClientInfo.get_account_type()+string("')");

			cout << "insert sql is " << sqll << endl;
			res = PQexec(objlogin.get_connection(),sqll.c_str());

			if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			    cout << "cannot insert" << endl;
			    PQclear(res);
			    return;
			 }

			PQclear(res);

			string ran_username_teller = objClientInfo.get_first_name()+result;

			sqll = string("insert into bank_account VALUES (")+string("'")+ran_username_teller+string("','")+
					objClientInfo.get_accountno()+string("','")+"0"+string("')");

					cout << "insert sql is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());

					if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					    cout << "cannot insert" << endl;
					    PQclear(res);
					    return;
					    }

					PQclear(res);

				char account_no[BUFFER_64];
				memset(&account_no, 0, sizeof(account_no));
				strcpy(account_no,objClientInfo.get_accountno().c_str());
				cout<<"Account no is "<< account_no;

				if(objlogin.get_type()=="T"){

				if (send(socket, &account_no,sizeof(account_no), 0) < 0) {
					cout << "Can't send back to client\n" << endl;
					//close(socket);
				}
			}//if(objlogin.get_type()=="T"){

				int rand_num;
				do{
				srand((unsigned)time(0));
				rand_num = rand()%10000;
				}while(rand_num<1000);


				string rand_password = to_string(rand_num);
				cout<<"Password is "<<rand_password<<endl;

				sqll = string("insert into bank_login (account_no, user_id, type,password) VALUES (")+
				string("'")+objClientInfo.get_accountno()+string("','")+ran_username_teller+string("','")+typ+string("','")+rand_password+string("')");

				cout << "insert sql is " << sqll << endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());

						if (PQresultStatus(res) != PGRES_COMMAND_OK) {
							PQclear(res);
						    cout << "cannot insert" << endl;
						    }
						else{
							PQclear(res);
							char msg[BUFFER_128];

							string sendMessage = ran_username_teller+"/"+rand_password;
							memset(&msg, 0, sizeof(msg));
							strcpy(msg, sendMessage.c_str());
							send(socket, &msg, sizeof(msg),0);


							//update the encrypted password into db

							memset(&msg, 0, sizeof(msg));
							recv(socket, &msg, sizeof(msg),0);

							string encrypted_password = msg;

							sqll = string("update bank_login set password=")+string("'")+encrypted_password +string("'")+
									string("where user_id=")+ string("'") + ran_username_teller + string("'");
							cout << "sqll1 is " << sqll << endl;


							res = PQexec(objlogin.get_connection(), sqll.c_str());

							if (PQresultStatus(res) != PGRES_COMMAND_OK) {
								PQclear(res);
								cout << "password not changed" << endl;
								return;
							}
							PQclear(res);

							// send an email to customer with the credentials
							string body;
							string to_emailId;
							string email_subject = "Welcome to SJSU Bank";


							body = string(" Dear ")+objClientInfo.get_first_name() + string( ",\n\nThank you for creating an Account at SJSU Bank. \n")+
									string("Your account number is ")+objClientInfo.get_accountno()+
									string("\nYou can access online banking services using following credentials")+
												string("\nUsername : ")+ran_username_teller+string("\nPassword : ")+rand_password +
												string("\n\nYou can access your account at www.sjsubank.com")+
												string("\n\n\n Regards,\n SJSU Bank");
							to_emailId =string("<")+ objClientInfo.get_email_id()+string(">");


							send_email(to_emailId.c_str(), body.c_str(), email_subject.c_str());

						}
	}
}

void close_account(int socket,Login obj_login)
{
	ClientInfo objClientInfo;
	char msg[BUFFER_1024];
	string sqll;
	PGresult *res;
	double exist_bal;
	while(1){
		memset(&msg, 0, sizeof(msg));
		if(recv(socket, &msg, sizeof(msg),0) < 0)
			throw string("*****error in creating client account by teller****");
		else{

			objClientInfo.set_accountno(msg);

			//compare with database here

			sqll = string("select COUNT(*) from bank_acc_details where type = 'C' and account_no=")+ string("'") + objClientInfo.get_accountno() + string("'");
			cout << "sqll is " << sqll << endl;
			res = PQexec(obj_login.get_connection(), sqll.c_str());

			if (atof(PQgetvalue(res, 0, 0)) == 0) {
				if (send(socket, "2", sizeof("2"), 0) < 0) {
					cout << "Error in deleting account\n" << endl;
					//return;
				}
				PQclear(res);

			}
			else{

				//delete bank account

				sqll = string("delete from bank_account WHERE account_no=")
						+ string("'") + objClientInfo.get_accountno()
						+ string("'");
				cout << "sql is " << sqll << endl;
				res = PQexec(obj_login.get_connection(), sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					cout << "record do not exist....continue to delete next.. "
							<< "\n" << endl;
				}
				PQclear(res);


				//delete bank login

				sqll = string("delete from bank_login WHERE account_no=")
						+ string("'") + objClientInfo.get_accountno()
						+ string("'");
				cout << "sql is " << sqll << endl;
				res = PQexec(obj_login.get_connection(), sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					cout << "record do not exist....continue to delete next.. "
							<< "\n" << endl;
				}
				PQclear(res);

				//delete transaction table


				 sqll = string("delete from bank_transaction WHERE account_no=")
						+ string("'") + objClientInfo.get_accountno()
						+ string("'");
				cout << "sql is " << sqll << endl;
				res = PQexec(obj_login.get_connection(), sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					cout << "record do not exist....continue to delete next.. "
							<< "\n" << endl;
				}
				PQclear(res);


				//delete fom transfer table for sender account no

				sqll = string("delete from bank_transfer WHERE sender_account=")
						+ string("'") + objClientInfo.get_accountno()
						+ string("' or ")+string(" receiver_account =")+string("'")+objClientInfo.get_accountno()+string("'");
				cout << "sql is " << sqll << endl;
				res = PQexec(obj_login.get_connection(), sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					cout << "record do not exist....continue to delete next.. "
							<< "\n" << endl;
				}
				PQclear(res);


				//delete bank account details

		sqll = string("delete from bank_acc_details WHERE account_no=")+ string("'") + objClientInfo.get_accountno() + string("'");
								cout << "sql is " << sqll << endl;
							res = PQexec(obj_login.get_connection(),sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {
					cout << "record do not exist....continue to delete next.. " << "\n"<< endl;
								 }
								PQclear(res);

				if (send(socket, "1", sizeof("1"), 0) < 0) {
					cout << "deleted account\n" << endl;
					//return;
				}
				break;
			}
		}
	}

}


void display_customer_detail(int socket, Login& objlogin)
{
	char message[BUFFER_1024];
	PGresult *res;
	string sqll;
	ClientInfo objClientInfo;

	while(1){
		memset(&message, 0, sizeof(message));
		recv(socket, &message, sizeof(message), 0);

		string account_no = message;
		sqll = string("select COUNT(*) from bank_acc_details where account_no=")
				+ string("'") + account_no + string("'") + "and type='C'";
		cout << "sqll is " << sqll << endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());


		if (atof(PQgetvalue(res, 0, 0)) == 0) {
			send(socket,"2", sizeof("2"),0);
			PQclear(res);
		}
		else {
			PQclear(res);
			//send client 1 to check if account no exists.
			send(socket,"1", sizeof("1"),0);

			sqll = string("SELECT a.first_name, a.last_name, a.address, a.phone_number, a.email_id, b.balance, a.account_type "
					"FROM bank_acc_details a, bank_account b"
					" where a.account_no = b.account_no and a.type = 'C'")+" and a.account_no = '"+string(account_no)+"'";

			cout<<"sql Display tellers query is : "<<sqll<<endl;

			res = PQexec(objlogin.get_connection(),sqll.c_str());

			string sendMessage;
			int rows = PQntuples(res);
			if(rows == 0){
				cout<<" No entry found against the customer";
				PQclear(res);
				//send(socket,"4", sizeof("4"),0);
			}
			else{

				for (int i = 0; i < rows; i++) {
					if(i!= 0)
						sendMessage+= string("/");
							printf("%s %s %s %s %s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1),
									PQgetvalue(res, i, 2),PQgetvalue(res,i,3),PQgetvalue(res,i,4),PQgetvalue(res,i,5), PQgetvalue(res,i,6));
							sendMessage += PQgetvalue(res, i, 0) + string("/")+ PQgetvalue(res, i, 1) + string("/")
									+ PQgetvalue(res, i, 2)+string("/")+PQgetvalue(res,i,3)+string("/")+PQgetvalue(res,i,4)
									+string("/")+PQgetvalue(res,i,5)+string("/")+PQgetvalue(res,i,6);

						} //for


						memset(&message, 0, sizeof(message));
						strcpy(message, sendMessage.c_str());
						send(socket, &message, sizeof(message),0);
						PQclear(res);

						return;
			}

		}//if account no present in database
	}//while(1)
}

void delimiting_string(char* message, ClientInfo& objclientInfo)
{
	char *point;
	point = strtok(message,"/");
	objclientInfo.set_first_name(point);
	point = strtok(NULL, "/");
	objclientInfo.set_last_name(point);
	point = strtok(NULL, "/");
	objclientInfo.set_adddress(point);
	point = strtok(NULL, "/");
	objclientInfo.set_phone_number(point);
	point = strtok(NULL, "/");
	objclientInfo.set_email_id(point);
	point = strtok(NULL, "/");
	objclientInfo.set_account_type(point);
	point = strtok(NULL, "/");

}



void modify_customer_Details(int socket, Login& objlogin)
{
	char message[BUFFER_1024];
		PGresult *res;
		string sqll;
		ClientInfo objClientInfo;

		while(1){
			memset(&message, 0, sizeof(message));
			recv(socket, &message, sizeof(message), 0);

			string account_no = message;
			sqll = string("select COUNT(*) from bank_acc_details where account_no=")
					+ string("'") + account_no + string("'") + "and type='C'";
			cout << "sqll is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());


			if (atof(PQgetvalue(res, 0, 0)) == 0) {
				send(socket,"2", sizeof("2"),0);
				PQclear(res);
			}
			else {
				PQclear(res);
				//send client 1 to saying that the account no exists.
				send(socket,"1", sizeof("1"),0);


				while(1){
					memset(&message, 0, sizeof(message));
					recv(socket, &message, sizeof(message), 0);


					switch (atoi(message)) {
						case MODIFY_ADDRESS:
						{
							//recieve the actual data for address here
							memset(&message, 0, sizeof(message));
							recv(socket, &message, sizeof(message), 0);

							string address = message;

							 sqll = string("update bank_acc_details set address=")+string("'")+to_string(address) +string("'")+
												string("where account_no=")+string("'")+account_no+string("'");

							cout << "sqll1 is " << sqll << endl;
							res = PQexec(objlogin.get_connection(), sqll.c_str());


							if (PQresultStatus(res) != PGRES_COMMAND_OK) {
								//send that the address has not been successfully changed
								cout << "cannot insert" << endl;
								send(socket,"2", sizeof("2"),0);
							}
							else{
								//send that the address has been successfully changed
								send(socket,"1", sizeof("1"),0);
							}
							PQclear(res);
						}
							break;

						case MODIFY_EMAIL:
						{
							//recieve the actual data for email address here
							memset(&message, 0, sizeof(message));
							recv(socket, &message, sizeof(message), 0);

							string email_id = message;

							 sqll = string("update bank_acc_details set email_id=")+string("'")+to_string(email_id) +string("'")+
												string("where account_no=")+string("'")+account_no+string("'");

							cout << "sqll1 is " << sqll << endl;
							res = PQexec(objlogin.get_connection(), sqll.c_str());


							if (PQresultStatus(res) != PGRES_COMMAND_OK) {
								//send that the email-address has not been successfully changed
								cout << "cannot insert" << endl;
								send(socket,"2", sizeof("2"),0);
							}
							else{
								//send that the email-address has been successfully changed
								send(socket,"1", sizeof("1"),0);
							}
							PQclear(res);
						}
							break;
						case MODIFY_PHN:
						{
							//recieve the actual data for phone number here
							memset(&message, 0, sizeof(message));
							recv(socket, &message, sizeof(message), 0);

							string phone_no = message;

							sqll = string("update bank_acc_details set phone_number=")
									+ string("'") + to_string(phone_no) + string("'")
									+ string("where account_no=") + string("'")
									+ account_no + string("'");

							cout << "sqll1 is " << sqll << endl;
							res = PQexec(objlogin.get_connection(), sqll.c_str());

							if (PQresultStatus(res) != PGRES_COMMAND_OK) {
								//send that the phone number has not been successfully changed
								cout << "cannot insert" << endl;
								send(socket, "2", sizeof("2"), 0);
							} else {
								//send that the phone number has been successfully changed
								send(socket, "1", sizeof("1"), 0);
							}
							PQclear(res);
						}
							break;
						default:
							return;
							break;

					}//switch

				}//while(1) to modify next entity

			}//else{

		}//main while(1)
}
