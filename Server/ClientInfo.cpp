
#include <string>
#include "ClientInfo.h"
using namespace std;


ClientInfo::ClientInfo():first_name(""),last_name(""),address(""),email_id(""),phone_number(""),accountno(""){}


string ClientInfo::get_first_name(){
	return first_name;
}

string ClientInfo::get_last_name(){
	return last_name;
}

string ClientInfo::get_phone_number(){
	return phone_number;
}

string ClientInfo::get_address(){
	return address;
}

string ClientInfo::get_email_id(){
	return email_id;
}

string ClientInfo::get_accountno()
{
	return accountno;
}

string ClientInfo::get_userid()
{
	return userid;
}

string ClientInfo::get_account_type()
{
	return account_type;
}

void ClientInfo::set_userid(string user){
	userid = user;
}


void ClientInfo::set_first_name(string fname){
	first_name = fname;
}

void ClientInfo::set_last_name(string lname){
	last_name = lname;
}

void ClientInfo::set_adddress(string addr){
	address = addr;
}

void ClientInfo::set_phone_number(string phn){
	phone_number = phn;
}

void ClientInfo::set_email_id(string email){
	email_id = email;
}

void ClientInfo::set_accountno(string accnt){
	accountno = accnt;
}

void ClientInfo::set_account_type(string accnt_type){
	account_type = accnt_type;
}
















