#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <pqxx/pqxx>
#include<postgresql/libpq-fe.h>
#include "headers.h"
#include "Login.h"

using namespace std;
using namespace pqxx;

#define PORT 9098
#define BUFFER 1024

enum CREDENTIALS {
	CORRECT = 1, P_INCORRECT = 2, U_INCORRECT =3
};

enum ACTIONS{
	DEPOSIT = 1, WITHDRAW =2, BALANCE_ENQUIRY = 3, MINI_STATEMENT = 4
};

void delimiting_string(char* message, Login& obj_login);
int authorizeLoginDetails(Login& obj_login);

Login::Login():username(""),password(""){}
int socketconnection() throw (string) {

	struct sockaddr_in server_info;		// structure containing all information about server
	int server_socket;	        		//socket descriptor to send and receive the data
	int size_server = sizeof(server_info);	//store the size of the server information

	server_info.sin_addr.s_addr = INADDR_ANY;
	server_info.sin_family = AF_INET; 					//specify Address family
	server_info.sin_port = htons(PORT);	//specify the port number to connect server

	// creating socket for TCP server connection
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
		throw string("Could not create socket");
	//binding server information to socket
	int bind_status;
	bind_status = bind(server_socket, (struct sockaddr *) &server_info,
			size_server);

	if (bind_status < 0) {
		throw string("Could not bind socket");
	}

	return server_socket;
}

void getLoginInfo(int clientFD, Login& obj_Login) {
	char buffer_Username[512];
	char buffer_Password[256];
	char *split_username;

	int send_status;
	int recieve_status;
	while (1) {
		memset(&buffer_Username, 0, sizeof(buffer_Username));
		recieve_status = recv(clientFD, &buffer_Username,sizeof(buffer_Username), 0);
		if (recieve_status < 0) {
			cout << "Can't get username \n" << endl;
			close(clientFD);
			return ;
		} else {
			cout << "buffer "<<buffer_Username;
			delimiting_string(buffer_Username,obj_Login);
			cout << "Password is :";
			cout << obj_Login.get_password()<< endl;
		}

		send_status = send(clientFD, "ACK", sizeof("ACK"), 0);
		if (send_status < 0) {
			cout << "Can't connect to login\n" << endl;
			close(clientFD);
			return ;
		}
		//break;



	int credential_check;
	credential_check = authorizeLoginDetails(obj_Login);

	//credential_check = 1;
	if (credential_check == P_INCORRECT) {
		send_status = send(clientFD,"2",sizeof("2"), 0);
		if (send_status < 0) {
			cout << "Can't connect to login\n" << endl;
			close(clientFD);
			return ;
		}
	}

	else if (credential_check == U_INCORRECT) {
		send_status = send(clientFD,"3",sizeof("3"), 0);
		if (send_status < 0) {
			cout << "Can't connect to login\n" << endl;
			close(clientFD);
			return ;
		}
	}

	else {
		send_status = send(clientFD, "1", sizeof("1"), 0);
		if (send_status < 0) {
			cout << "Can't connect to login\n" << endl;
			close(clientFD);
			return;
		}
		return;
		//cout << "message sent to client" << endl;
	 }
	}
}


//to be changed by Tejas
int authorizeLoginDetails(Login& obj_login) {
	PGresult *res;
	string sqll;
	string u_name = obj_login.get_username();
	string pas = obj_login.get_password();
	cout << u_name << " and " << pas << endl;


	//compare with database here

	sqll = string("select user_id from bank_login where user_id=")+string("'")+u_name+string("'");
	cout << "sqll is " << sqll << endl;
	res = PQexec(obj_login.get_connection(),sqll.c_str());
	cout << "res is " << res << endl;

	int rows = PQntuples(res);
	cout<<"rows :"<<rows<<endl;
	PQclear(res);

	if (rows==0) {
		                    puts("We did not get any data!");
		                    cout << "inside incorrect " << endl;
		                    return U_INCORRECT;
		            }


		sqll = string("select account_no from bank_login where user_id=")+string("'")+u_name+string("'");
		cout << "sqll is " << sqll << endl;
		res = PQexec(obj_login.get_connection(),sqll.c_str());
		cout << "res is " << res << endl;

	string accno = PQgetvalue(res,0,0);
	PQclear(res);


	 sqll = string("select type from bank_acc_details where account_no=")+string("'")+accno+string("'");
			cout << "sqll is " << sqll << endl;
			res = PQexec(obj_login.get_connection(),sqll.c_str());
			cout << "res is " << res << endl;

		string typ = PQgetvalue(res,0,0);
		PQclear(res);


		if(strcmp(obj_login.get_type().c_str(),typ.c_str())!=0){

			return U_INCORRECT;

		}

	 string sqll1 = string("select password from bank_login where  user_id=")+string("'")+u_name+string("'");
	 cout << "sqll1 is " << sqll1 << endl;
	 res = PQexec(obj_login.get_connection(),sqll1.c_str());
	 cout << "res is " << res << endl;
	 if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	 		                    puts("We did not get any data!");
	 		                    return P_INCORRECT;
	 		            }
	 else if(PQntuples(res)==0){
		 return P_INCORRECT;
	 }

	 string result = PQgetvalue(res,0,0);
	 PQclear(res);
	 cout<< "result is "<<result<<endl;
	 cout<< "pas is "<<pas<<endl;

	   if(strcmp(pas.c_str(),result.c_str())==0){
		   cout<< "result "<<"CORRECT"<<endl;
		   return CORRECT;
	   }
	   else
		   return P_INCORRECT;



	return CORRECT;

}

void delimiting_string(char* message, Login& obj_login)
{

	char *point;
	point = strtok(message,"}");
	obj_login.set_username(point);
	point = strtok(NULL, "}");
	obj_login.set_password(point);
}


void Login::set_username(string str)
{
	username = str;
}

void Login::set_password(string pass)
{
	password = pass;
}

void Login::set_connection(pg_conn* connection)
{
	conn = connection;
}

void Login::set_type(string ty)
{
	type = ty;
}

string Login::get_type()
{
	return type;
}

string Login::get_password()
{
	return password;
}

string Login::get_username()
{
	return username;
}

pg_conn* Login::get_connection()
{
	return conn;
}







