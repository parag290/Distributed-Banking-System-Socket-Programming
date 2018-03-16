import login
import re
import getpass
import os
from prettytable import PrettyTable
import encrypt

actions = {'LOGIN': 1, 'SIGN_UP': 2}
customer_request = {'BALANCE_INFO': 1, 'STATEMENT': 2, 'TRANSFER_MONEY': 3, 'CHANGE_PASSWORD': 4}
account_check = {'CORRECT': 1, 'INCORRECT': 2}
password_check = {'CORRECT': 1, 'INCORRECT': 2}

logedin_user = ""


def main_page(client_socket):

    global logedin_user
    logedin_user = login.login_page(client_socket, "C")

    while True:
        os.system('clear')
        print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
        print("\n************ Menu ************\n")
        print('\n1. Balance Information\n2. Statement\n3. Transfer Money\n4. Change Password\n5. Logout')

        selection = input('Select from above options : ')

        if selection == '1' or selection == '2' or selection == '3' or selection == '4':
            client_socket.send(str.encode(selection))
            if int(selection) == customer_request['BALANCE_INFO']:
                balance_info(client_socket)
            elif int(selection) == customer_request['STATEMENT']:
                bank_statement(client_socket)
            elif int(selection) == customer_request['TRANSFER_MONEY']:
                transfer_money(client_socket)
            elif int(selection) == customer_request['CHANGE_PASSWORD']:
                change_password(client_socket)
            continue
        elif selection == '5':
            client_socket.send(str.encode("0"))
            break
        else:
            print('\033[1;3m' + 'Choose correct option......\n\n' + '\033[0m')
            continue

    return


def balance_info(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Balance Information ******\n")
    balance = ''
    while balance == '':
        balance = client_socket.recv(1024).decode()
        balance = balance.rstrip('\x00')

    print("Your Current Balance is $"+ balance)
    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def bank_statement(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****************** Bank Statement  ******************\n")
    data = ''
    while data == '':
        data = client_socket.recv(2048).decode()
        data = data.rstrip('\x00')
    try:

        if data == '4':
            print('\033[1;31m' + "Transaction Database Empty" + '\033[0m')
            ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
            return

        s1 = re.split('/', data)
        i = 0

        t = PrettyTable(['Date', 'Transaction Details', 'Balance'])

        while i < len(s1):
            date = s1[i]
            i += 1
            transaction = s1[i]
            i += 1
            balance = s1[i]
            i += 1
            t.add_row([date, transaction, balance])

        print(t)

        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')

    except IndexError:
        print('\033[1;31m' + "Transaction Database Empty" + '\033[0m')
        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
        return

    return


def transfer_money(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n**************** Transfer Money ****************\n")

    while True:
        account_no = input('Enter account number for money transfer : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()
        amount = input('Enter amount to be transferred : ')
        if len(amount) == 0:
            continue

        client_socket.send(str.encode(account_no+"/"+amount))

        reply = ''
        while reply == '':
            reply = client_socket.recv(256).decode()
            reply = reply.rstrip('\x00')

        if int(reply) == account_check['CORRECT']:
            print('\033[3;94m' + "Money transfer request sent" + '\033[0m')
            break
        elif int(reply) == account_check['INCORRECT']:
            print('\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def change_password(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n********* Change Password *********\n")

    while True:
        #current_password = input('Enter current password : ')
        current_password = getpass.getpass('Enter current Password : ')

        while True:
            new_password = getpass.getpass('\nEnter new Password : ')
            new_password1 = getpass.getpass('Re-enter new Password : ')
            #new_password = input('Enter new password : ')
            #new_password1 = input('Re-enter new password : ')

            if len(new_password) == 0 or len(new_password1) == 0:
                continue

            if new_password == new_password1:
                break
            else:
                print('\033[1;3;31m' + "\nPasswords do not match !! \n" + '\033[0m' + '\033[1;3m' + "Please enter again" + '\033[0m')
                continue

        new_password = encrypt.AESCipher('0123456789abcdef').encrypt(new_password)
        current_password = encrypt.AESCipher('0123456789abcdef').encrypt(current_password)

        client_socket.send(current_password+str.encode("}")+new_password)

        reply = ''
        while reply == '':
            reply = client_socket.recv(256).decode()
            reply = reply.rstrip('\x00')

        if int(reply) == password_check['CORRECT']:
            print('\033[3;94m' + "\nPassword successfully changed" + '\033[0m')
            break
        elif int(reply) == password_check['INCORRECT']:
            print('\033[1;3;31m' + "\nIncorrect current password\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return




