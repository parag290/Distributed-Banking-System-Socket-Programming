import getpass
import os
import encrypt


actions = {'LOGIN': 1}
password_check = {'CORRECT': 1, 'P_INCORRECT': 2, 'U_INCORRECT': 3}


def login_page(client_socket, client_type):

    os.system('clear')
    action = str(actions['LOGIN'])
    client_socket.send(str.encode(action))
    print("\n******************* SJSU Bank *********************\n")

    if client_type == "A":
        print("\t\tWelcome Administrator!!!!\n")
    elif client_type == "T":
        print("\t\tWelcome Teller!!!!\n")
    elif client_type == "C":
        print("\t\tWelcome Customer!!!!\n")

    while True:
        username = input('Enter Username : ')
        if len(username) == 0:
            print('\033[1;3m' + "Please enter username\n" + '\033[0m')
            continue

        #password = input('Enter Password : ')
        password = getpass.getpass('Enter Password : ')
        if len(password) == 0:
            print('\033[1;3m' + "Please enter password\n" + '\033[0m')
            continue

        password = encrypt.AESCipher('0123456789abcdef').encrypt(password)

        client_socket.send(str.encode(username + "}")+password)

        data = client_socket.recv(64).decode()
        data = data[:-1]

        data = client_socket.recv(64).decode()
        data = data[:-1]

        if int(data) == password_check['P_INCORRECT']:
            print('\033[1;3;31m' + "\nIncorrect Password\n" + '\033[0m' + '\033[1;3m' + "Please enter correct password\n" + '\033[0m')
            continue
        elif int(data) == password_check['U_INCORRECT']:
            print('\033[1;3;31m' + "\nIncorrect Username\n" + '\033[0m' + '\033[1;3m' + "Please enter correct user name\n" + '\033[0m')
            continue
        elif int(data) == password_check['CORRECT']:
            return username


