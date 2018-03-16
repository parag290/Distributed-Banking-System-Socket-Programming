#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pqxx/pqxx>
#include<postgresql/libpq-fe.h>
#include <exception>
#include "headers.h"
#include "Login.h"

using namespace std;
using namespace pqxx;


enum CREDENTIALS {
	CORRECT = 1, INCORRECT = 2
};

enum ACTIONS{
	lOGIN = 1
};

enum CLIENTYPE{
	CUSTOMER = 1, TELLER = 2, ADMIN =3
};

enum Buffer_Size{
	BUFFER_64 = 64, BUFFER_256 = 256, BUFFER_512 = 512, BUFFER_1024 = 1024
};

//pg_conn *conn;


void check_clientType(int, Login& login);

int main() {
	int listening_socket, connected_socket;
	try {
		int process_id;


		//socket created and bind
		listening_socket = socketconnection();

		//socket listening
		if(listen(listening_socket, 10) == -1)
			throw string("Socket could not listen");

		cout << "Process %u has been created \n" << getpid() << endl;
		// server running infinitely
		while (1) {
			try {

				connected_socket = accept(listening_socket, NULL, NULL);
				if (connected_socket < 0) {
					throw string("Socket could not accept");
				}

				process_id = fork();

		if (process_id == 0) // to check if the child has been created and handle one process per request
		{

				printf("%u Child process running \n", getpid());

				close(listening_socket);
				Login obj_login;


				//conn = PQconnectdb("dbname = BankSystem user = ghost password = wolf host = 10.0.0.210");

				obj_login.set_connection(PQconnectdb("dbname = BankSystem user = postgres password = 916 host = localhost"));
				if (PQstatus(obj_login.get_connection()) == CONNECTION_BAD) {
							throw string("Unable to connect to database");
						}
				cout<<"Conn -->"<<obj_login.get_connection()<<endl;

				check_clientType(connected_socket,obj_login);

				close(connected_socket);
				cout<<" Connected Socket close ...."<<endl;
				PQfinish(obj_login.get_connection());
				exit(0);

		} //if (process_id == 0)
			}//try
			catch(string &errormsg)
			{
				close(connected_socket);
				cout<< errormsg <<endl;
			}//catch
		}//while

		//close the socket
		close(listening_socket);

		return 0;
	}//try{
	catch (string &errormsg) {
		cout<< errormsg <<endl;
		close(listening_socket);
	}//catch
}

void check_clientType(int listenSock,Login& objlogin)
{
	char client_type[BUFFER_64];
	char login_type[BUFFER_64];


		memset(&client_type, 0, sizeof(client_type));
		if(recv(listenSock, &client_type, sizeof(client_type),0) < 0)
			throw string("Incorrect Client Type");
		else
			cout<<"--->  "<<client_type<<endl;
		memset(&login_type, 0, sizeof(login_type));
		if(recv(listenSock,&login_type, sizeof(login_type) ,0) < 0)
				throw string("Could not login");
		else{
			if(atoi(login_type) == lOGIN)
			{
				switch (atoi(client_type)) {
							case CUSTOMER:
								objlogin.set_type("C");

								break;
							case TELLER:
								objlogin.set_type("T");

								break;
							case ADMIN:
								objlogin.set_type("A");

								break;
							default:
								break;
						}
				getLoginInfo(listenSock,objlogin);
			}
		}

		switch (atoi(client_type)) {
			case CUSTOMER:
				objlogin.set_type("C");
				customer_init(listenSock,objlogin);
				break;
			case TELLER:
				objlogin.set_type("T");
				teller_init(listenSock,objlogin);
				break;
			case ADMIN:
				objlogin.set_type("A");
				admin_init(listenSock,objlogin);
				break;
			default:
				break;
		}
}



