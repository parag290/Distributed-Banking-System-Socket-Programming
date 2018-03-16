import socket
import sys
import customer
import admin
import teller
import os


def main():
    global logedin_user
    logedin_user = ""

    os.system('clear')

    while True:

        print("\n******************* Welcome to SJSU Bank *********************\n")
        ignore = input("Press enter to continue!!!")

        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except socket.error:
            print('\033[1;3;31m' + "could not create socket" + '\033[0m')
            continue
            #sys.exit()

        host = "localhost"
        port = 9098

        try:
            remote_ip = socket.gethostbyname(host)
        except socket.error:
            print('\033[1;3;31m' + "error resolving host" + '\033[0m')
            continue
            #sys.exit()

        try:
            client_socket.connect((remote_ip, port))
        except socket.error:
            print('\033[1;3;31m' + "Unable to connect server\nPlease try again" + '\033[0m')
            continue
            #sys.exit()

        client_socket.settimeout(500)

        try:
            welcome_screen(client_socket)
        except socket.timeout:
            print('\033[1;3;31m' + "\nSession timeout.... Try Again" + '\033[0m')
        except KeyboardInterrupt:
            print('\033[1;3;31m' + "\nSession Aborted" + '\033[0m')

        client_socket.close()

    return


def welcome_screen(client_socket):

    os.system('clear')
    while True:
        print("\n******************* Welcome to SJSU Bank *********************\n")
        print("1. Customer \n2. Teller \n3. System Administrator\n")
        user_type = input('Select client type : ')

        if user_type == '1' or user_type == '2' or user_type == '3':
            break
        else:
            print('Invalid option...... Select again\n')
            continue

    client_socket.send(str.encode(user_type))

    if int(user_type) == 1:
        customer.main_page(client_socket)
    elif int(user_type) == 2:
        teller.main_page(client_socket)
    elif int(user_type) == 3:
        admin.main_page(client_socket)

    return

main()
