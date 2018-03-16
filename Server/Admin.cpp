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
	BUFFER_64 = 64, BUFFER_256 = 256, BUFFER_512 = 512, BUFFER_1024 = 1024
};
enum ACTIONS_ADMIN{
	LOGOUT = 0, CREATE_TELLER =1, DELETE_TELLER = 2, DISPLAY_TELLERS = 3,DISPLAY_DETAILS = 4, DISPLAY_TRANSFER_REQUESTS = 5
};

enum CREDENTIALS {
	CORRECT = 1, INCORRECT = 2, NO_TELLER_EXISTS = 3, NO_CUSTOMERS_EXIST = 4
};

int delete_teller(int socket, Login objlogin);
int display_bank_details(int socket, Login& objlogin);
void display_request(int socket, Login objlogin);
void process_request(int socket, Login objlogin);
int display_all_tellers(int socket, Login objlogin);

void admin_init(int socket,Login& obj_login){

	char msg[BUFFER_64];

		while(1){
			memset(&msg, 0, sizeof(msg));
			if(recv(socket, &msg, sizeof(msg),0) < 0)
				throw string("*****error in admin****");
			else{
				cout<<"In admin main menu"<<endl;
				switch (atoi(msg)) {
					case CREATE_TELLER:
						open_account(socket,obj_login);
						break;
					case DELETE_TELLER:
						delete_teller(socket,obj_login);
						break;
					case DISPLAY_TELLERS:
						display_all_tellers(socket,obj_login);
						break;
					case DISPLAY_DETAILS:
						display_bank_details(socket,obj_login);
						cout<<"In admin display menu"<<endl;
						break;
					case DISPLAY_TRANSFER_REQUESTS:
						display_request(socket, obj_login);
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
}


int delete_teller(int socket, Login objlogin){
	char msg[BUFFER_1024];

		memset(&msg,0,sizeof(msg));

		PGresult *res;
		string sqll;

		ClientInfo objClientInfo;

		sqll = string("SELECT a.first_name, a.last_name, b.user_id FROM bank_acc_details a, bank_login b"
				" where a.account_no = b.account_no and a.type = 'T'");
		cout<<"sql Display tellers query is : "<<sqll<<endl;

		res = PQexec(objlogin.get_connection(),sqll.c_str());

		string sendMessage;
		int rows = PQntuples(res);
		if(rows == 0){
			PQclear(res);
			send(socket,"3", sizeof("3"),0);
			return NO_TELLER_EXISTS;
		}
		else {

		for (int i = 0; i < rows; i++) {
			if(i!= 0)
				sendMessage+= string("/");
			printf("%s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1),
					PQgetvalue(res, i, 2));
			sendMessage += PQgetvalue(res, i, 0) + string("/")+ PQgetvalue(res, i, 1) + string("/")
					+ PQgetvalue(res, i, 2);

		} //for


		memset(&msg, 0, sizeof(msg));
		strcpy(msg, sendMessage.c_str());
		send(socket, &msg, sizeof(msg),0);
		PQclear(res);


		//wait for client to send which teller to delete
		while(1)
		{

			memset(&msg,0,sizeof(msg));
			if(recv(socket, &msg, sizeof(msg),0) > 0)
				objClientInfo.set_userid(msg);

			sqll = string("select account_no from bank_login where type = 'T' and user_id=")+ string("'") + objClientInfo.get_userid() + string("'");
			cout << "sqll is " << sqll << endl;
			res = PQexec(objlogin.get_connection(), sqll.c_str());

			int rows = PQntuples(res);

			if (rows == 0) {
				cout<< "record of teller do not exist. "<<endl;
				send(socket, "2", sizeof("2"), 0);
				PQclear(res);

				//return INCORRECT;
			}
			else{

				string account_no =	PQgetvalue(res,0,0);
				cout <<"Account no of teller is -->"<<account_no<<endl;
				objClientInfo.set_accountno(account_no);

				PQclear(res);


				//after fetching the account no of teller delete the teller from bank_login
				sqll = string("delete from bank_login WHERE user_id=")+ string("'") + objClientInfo.get_userid() + string("'");
						cout << "sql is " << sqll << endl;
						res = PQexec(objlogin.get_connection(),sqll.c_str());

								if (PQresultStatus(res) != PGRES_COMMAND_OK) {
									cout<<"Could not delete teller from bank_login"<<endl;
								}
								PQclear(res);

								sqll = string("delete from bank_account WHERE user_id=")+ string("'") + objClientInfo.get_userid() + string("'");
														cout << "sql is " << sqll << endl;
														res = PQexec(objlogin.get_connection(),sqll.c_str());

																if (PQresultStatus(res) != PGRES_COMMAND_OK) {
																	cout<<"Could not delete teller from bank_login"<<endl;
																}
																PQclear(res);
				//delete teller from bank_acount details table as well
				sqll = string("delete from bank_acc_details WHERE account_no=")+ string("'")+ objClientInfo.get_accountno()+ string("'");
								cout << "sql is " << sqll << endl;
								res = PQexec(objlogin.get_connection(),sqll.c_str());

										if (PQresultStatus(res)
												!= PGRES_COMMAND_OK) {
											cout<< "Could not delete teller from bank account details"
													<< endl;
										}
										PQclear(res);
										//Send an ack to client of teller beeing deleted
										cout<<"Teller -->"<<objClientInfo.get_userid()<<" successfully deleted"<<endl;
										send(socket, "1", sizeof("1"),0);
										break;
			}

	}//while

		return CORRECT;

	}
}


int display_all_tellers(int socket, Login objlogin){
	char msg[BUFFER_1024];

		memset(&msg,0,sizeof(msg));

		PGresult *res;
		string sqll;

		ClientInfo objClientInfo;

		sqll = string("SELECT a.first_name, a.last_name, b.user_id FROM bank_acc_details a, bank_login b"
				" where a.account_no = b.account_no and a.type = 'T'");
		cout<<"sql Display tellers query is : "<<sqll<<endl;

		res = PQexec(objlogin.get_connection(),sqll.c_str());

		string sendMessage;
		int rows = PQntuples(res);
		if(rows == 0){
			PQclear(res);
			send(socket,"3", sizeof("3"),0);
			return NO_TELLER_EXISTS;
		}
		else {

		for (int i = 0; i < rows; i++) {
			if(i!= 0)
				sendMessage+= string("/");
			printf("%s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1),
					PQgetvalue(res, i, 2));
			sendMessage += PQgetvalue(res, i, 0) + string("/")+ PQgetvalue(res, i, 1) + string("/")
					+ PQgetvalue(res, i, 2);

		} //for

		//Send client the list of tellers in the bank
		memset(&msg, 0, sizeof(msg));
		strcpy(msg, sendMessage.c_str());
		send(socket, &msg, sizeof(msg),0);
		PQclear(res);

		return CORRECT;

	}
}


int display_bank_details(int socket, Login& objlogin)
{
	char message[BUFFER_1024];

			PGresult *res;
			string sqll;

			ClientInfo objClientInfo;

			sqll = string("SELECT a.account_no ,a.first_name, a.last_name, b.balance ,a.account_type FROM bank_acc_details a, bank_account b"
					" where a.account_no = b.account_no and a.type = 'C'");
			cout<<"sql Display tellers query is : "<<sqll<<endl;

			res = PQexec(objlogin.get_connection(),sqll.c_str());

			cout<<res<<endl;
			string sendMessage;
			int rows = PQntuples(res);
			cout<<rows<<endl;
			if(rows == 0){
				memset(&message, 0, sizeof(message));
				strcpy(message, "4");
				PQclear(res);
				cout<<"result set cleared out"<<endl;
				send(socket, &message, sizeof(message),0);
				//send(socket,"4", sizeof("4"),0);
				cout<<"Display sent to client"<<endl;
				return NO_CUSTOMERS_EXIST;
			}
			else{

				for (int i = 0; i < rows; i++) {
					if(i!= 0)
						sendMessage+= string("/");
							printf("%s %s %s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1),
									PQgetvalue(res, i, 2),PQgetvalue(res,i,3), PQgetvalue(res,i,4));
							sendMessage += PQgetvalue(res, i, 0) + string("/")+ PQgetvalue(res, i, 1) + string("/")
									+ PQgetvalue(res, i, 2)+string("/")+PQgetvalue(res,i,3)+string("/")+PQgetvalue(res,i,4);

						} //for


						memset(&message, 0, sizeof(message));
						strcpy(message, sendMessage.c_str());
						send(socket, &message, sizeof(message),0);
						PQclear(res);

			}

			return CORRECT;
}


void display_request(int socket, Login objlogin) {

	ClientInfo objClientInfo;
	string sqll;
	PGresult *res;
	string data;
	char msg[BUFFER_512];

	sqll =
			string(
					"select id, sender_account,receiver_account, amount from bank_transfer where state=")
					+ string("'") + "PENDING" + string("'");
	cout << "sqll is " << sqll << endl;
	res = PQexec(objlogin.get_connection(), sqll.c_str());

	int rows = PQntuples(res);

	if (rows == 0) {

		if (send(socket, "4", sizeof("4"), 0) < 0) {
			cout << "No PENDING requests\n" << endl;
			return;
		}
		return;

	}

	for (int i = 0; i < rows; i++) {
		if (i != 0)
			data += "/";
		printf("%s %s %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1),
				PQgetvalue(res, i, 2), PQgetvalue(res, i, 3));
		data += PQgetvalue(res, i, 0) + string("/") + PQgetvalue(res, i, 1)
				+ string("/") + PQgetvalue(res, i, 2) + string("/")
				+ PQgetvalue(res, i, 3);

	}

	memset(&msg, 0, sizeof(msg));
	strcpy(msg, data.c_str());
	PQclear(res);

	if (send(socket, &msg, sizeof(msg), 0) < 0) {
		cout << "Data not sent\n" << endl;
	}
	else{
		process_request(socket, objlogin);
	}

	return;

}


void process_request(int socket, Login objlogin){
		ClientInfo objClientInfo;
		string sqll;
		PGresult *res;
		string data;
		char msg[BUFFER_256];
		string body;
		string to_emailId;

		while(1){
			memset(&msg, 0, sizeof(msg));
			if(recv(socket, &msg, sizeof(msg),0) < 0)
				throw string("*****error in depositing by teller****");

			else{

				int id = atoi(msg);
				if(id == 0)
					return;
				float sender_b,receiver_b,amt;
				string sender_bal,receiver_bal;
				//get request data
				sqll = string("select sender_account,receiver_account, amount from bank_transfer where id=")+string("'")+to_string(id)+string("'");
				cout << "sqll is " << sqll << endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());

				string sender_acc,recv_acc, amount;

				sender_acc = PQgetvalue(res, 0, 0);
				recv_acc = PQgetvalue(res, 0, 1);
				amt = floorf(atof(PQgetvalue(res,0,2))*100)/100.0;
				PQclear(res);

				///////check balance of sender

				sqll = string("select balance from bank_account where account_no=")+string("'")+sender_acc+string("'");
				cout << "sqll is " << sqll << endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());
				sender_b = floorf(atof(PQgetvalue(res,0,0))*100)/100.0;
				cout<<"Balance = "<<sender_b<<"\n";
				PQclear(res);

				///check balance of receiver

				sqll = string("select balance from bank_account where account_no=")+string("'")+recv_acc+string("'");
				cout << "sqll is " << sqll << endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());
				receiver_b = floorf(atof(PQgetvalue(res,0,0))*100)/100.0;
				cout<<"Balance = "<<receiver_b<<"\n";
				PQclear(res);

				float diff = sender_b - amt;

				if(diff>0){


					sqll = string("update bank_account set balance=")+string("'")+to_string(diff)+string("'")+
							string("where account_no=")+string("'")+sender_acc+string("'");
					cout << "sqll1 is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());
					PQclear(res);

					float u_recvbal = receiver_b+amt;
					sqll = string("update bank_account set balance=")+string("'")+to_string(u_recvbal)+string("'")+
					string("where account_no=")+string("'")+recv_acc+string("'");
					cout << "sqll1 is " << sqll << endl;
					res = PQexec(objlogin.get_connection(),sqll.c_str());
					PQclear(res);


					sqll = string("update bank_transfer set state=")+string("'")+"APPROVED"+string("'")+
																string("where id=")+string("'")+to_string(id)+string("'");
														   			cout << "sqll1 is " << sqll << endl;
														   	res = PQexec(objlogin.get_connection(),sqll.c_str());

														   	PQclear(res);

					//Added by Niketa Mane
					//Insert into transaction table for sender
                   time_t transc_time = time(&transc_time);

                  sqll = string("insert into bank_transaction (account_no, trans_date, trans_details, balance) VALUES (")
                          + string("'") +sender_acc+ string("','") + asctime(localtime(&transc_time)) +string("','")+ "Transfer $"+to_string(amt)+string("','")
																	+to_string(diff)+string("')");
                  cout<<"Insert Query for transanction for sender is: ---"<<sqll<<endl;
                  res = PQexec(objlogin.get_connection(),sqll.c_str());

					if (PQresultStatus(res) != PGRES_COMMAND_OK) {

						cout << "cannot insert" << endl;
					}

					PQclear(res);



					//Added by Niketa Mane
					//Insert into transaction table for reciever
					sqll = string("insert into bank_transaction (account_no, trans_date, trans_details, balance) VALUES (")
					                          + string("'") +recv_acc+ string("','") + asctime(localtime(&transc_time)) +string("','")+ "Deposit $"+to_string(amt)+string("','")
																						+to_string(u_recvbal)+string("')");
				cout << "Insert Query is for transaction for reciever is ---" << sqll << endl;
				res = PQexec(objlogin.get_connection(), sqll.c_str());

				if (PQresultStatus(res) != PGRES_COMMAND_OK) {

					cout << "cannot insert" << endl;
				}

				PQclear(res);

				//Send the successfully approved message to client
				memset(&msg, 0, sizeof(msg));
				strcpy(msg, "1");

				send(socket, &msg, sizeof(msg), 0);

				//send an email to the customer saying that transfer request has been approved
				sqll = string("select first_name,email_id from bank_acc_details where account_no = '")+sender_acc +string("'");

				cout<<"sql select query is : "<<sqll<<endl;
				res = PQexec(objlogin.get_connection(),sqll.c_str());

				int i = 0;
				printf("Customer name is ---> %s \n", PQgetvalue(res, i, 0));
				printf("Customer Email Id  is ---> %s \n", PQgetvalue(res, i, 1));
				string customer_name = PQgetvalue(res, i, 0);
				string customer_email_id = PQgetvalue(res, i, 1);
				string email_subject = "Transfer request - SUCCESS";

				PQclear(res);

				body = string(" Dear ")+customer_name +string( ",\n\nYour transfer request for $")+to_string(amt)+
						string(" to bank account ")+recv_acc+string(" has been approved. \n")+
						string("\nThank you for banking with SJSU Bank.")+
						string("\n\n\n Regards,\n SJSU Bank");
				to_emailId = string("<") + customer_email_id + string(">");

				send_email(to_emailId.c_str(), body.c_str(),email_subject.c_str());

				/* Call for Email Send to customer for transfer request end*/

				cout << "Transfer Request Processed\n" << endl;
				return;

				}else{
					//Send the approve has been rejected due to insufficient balance request to client
					memset(&msg, 0, sizeof(msg));
					strcpy(msg, "2");
					send(socket, &msg, sizeof(msg), 0);
					cout << "Insufficient balance\n" << endl;
					sqll = string("update bank_transfer set state=")+string("'")+"REJECTED"+string("'")+
											string("where id=")+string("'")+to_string(id)+string("'");
									   			cout << "sqll1 is " << sqll << endl;
									   	res = PQexec(objlogin.get_connection(),sqll.c_str());
									   	PQclear(res);
									   	//send an email to the customer saying that transfer request has been approved
				sqll =
						string("select first_name,email_id from bank_acc_details where account_no = '")
								+ sender_acc + string("'");

				cout << "sql select query is : " << sqll << endl;
				res = PQexec(objlogin.get_connection(), sqll.c_str());

				int i = 0;
				printf("Customer name is ---> %s \n", PQgetvalue(res, i, 0));
				printf("Customer Email Id  is ---> %s \n",
						PQgetvalue(res, i, 1));
				string customer_name = PQgetvalue(res, i, 0);
				string customer_email_id = PQgetvalue(res, i, 1);
				string email_subject = "Transfer request - REJECTED";

				PQclear(res);

				body = string(" Dear ") + customer_name
						+ string(",\n\nYour transfer request for $") +to_string(amt)
						+ string(" to bank account ") + recv_acc
						+ string(" has been rejected due to the insufficient balance. \n")
						+ string("\nThank you for banking with SJSU Bank.")
						+ string("\n\n\n Regards,\n SJSU Bank");
				to_emailId = string("<") + customer_email_id + string(">");

				send_email(to_emailId.c_str(), body.c_str(),email_subject.c_str());

				/* Call for Email Send to customer for transfer request end*/






									   	break;

				}
			}
		}

	}







