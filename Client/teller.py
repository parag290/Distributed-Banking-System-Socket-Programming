import login
import re
import os
from prettytable import PrettyTable
import encrypt

actions = {'OPEN_ACCNT': 1, 'CLOSE_ACCNT': 2, 'DEPOSIT': 3, 'WITHDRAW': 4, 'DISPLAY_DETAILS': 5, 'MODIFY': 6}
account_check = {'CORRECT': 1, 'INCORRECT': 2, 'INSUFFICIENT_BALANCE': 3}

logedin_user = ""


def main_page(client_socket):

    global logedin_user
    logedin_user = login.login_page(client_socket, "T")

    while True:
        os.system('clear')
        print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
        print("\n******************* Menu *******************\n")
        print('\n1. Open account\n2. Close account\n3. Deposit for Customer\n4. Withdraw for Customer\n5. Display Customer details\n6. Modify Customer details\n7. Logout\n')

        selection = input('Select from above options : ')

        if selection == '1' or selection == '2' or selection == '3' or selection == '4' or selection == '5' or selection == '6':
            client_socket.send(str.encode(selection))
            if int(selection) == actions['OPEN_ACCNT']:
                open_account(client_socket)
            elif int(selection) == actions['CLOSE_ACCNT']:
                close_account(client_socket)
            elif int(selection) == actions['DEPOSIT']:
                deposit(client_socket)
            elif int(selection) == actions['WITHDRAW']:
                withdraw(client_socket)
            elif int(selection) == actions['MODIFY']:
                modify_customer_details(client_socket)
            elif int(selection) == actions['DISPLAY_DETAILS']:
                display_customer_details(client_socket)
            continue
        elif selection == '7':
            client_socket.send(str.encode('0'))
            break
        else:
            print('\033[1;3m' + 'Choose correct option......\n\n' + '\033[0m')
            continue

    return


def open_account(client_socket):

    while True:
        os.system('clear')
        print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
        print("\n***************** Open New Account *****************\n")

        print('\nFill the following information\n')

        while True:
            account_type = input('Enter Account Type - Checking(C) or Saving(S) : ')
            if len(account_type) == 0:
                continue
            elif account_type == 'C' or account_type == 'c':
                account_type = 'Checking'
                break
            elif account_type == 's' or account_type == 'S':
                account_type = 'Saving'
                break
            else:
                print('\033[3m' + "Please enter correct option\n" + '\033[0m')
                continue

        while True:
            first_name = input('First Name : ')
            if len(first_name) == 0:
                print('\033[3m' + "Please enter first name\n" + '\033[0m')
                continue
            else:
                break

        while True:
            last_name = input('Last name : ')
            if len(last_name) == 0:
                print('\033[3m' + "Please enter last name\n" + '\033[0m')
                continue
            else:
                break

        while True:
            address = input('Address : ')
            if len(address) == 0:
                print('\033[3m' + "Please enter address\n" + '\033[0m')
                continue
            else:
                break

        while True:
            phone_number = input('Phone Number : ')
            try:
                i = int(phone_number)
                if len(phone_number) != 10 or i < 0:
                    print(
                        '\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                    continue
                else:
                    break
            except ValueError:
                # Handle the exception
                print(
                    '\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                continue

        while True:
            email_id = input('E-mail address : ')

            match = re.search(r'[\w.-]+@[\w.-]+.\w+', email_id)
            if match:
                break
            else:
                print(
                    '\033[3;31m' + "Incorrect email address\n" + '\033[0m' + '\033[3m' + "Please enter correct email address\n" + '\033[0m')
                continue

        confirmation = input('\033[3m' + "\nEnter 'Y' to confirm or 'N' to re-enter data : " + '\033[0m')

        while True:
            if confirmation == 'Y' or confirmation == 'y' or confirmation == 'N' or confirmation == 'n':
                break
            else:
                confirmation = input('\033[3m' + "\nEnter 'Y' to confirm or 'N' to re-enter data : " + '\033[0m')
                continue

        if confirmation == 'Y' or confirmation == 'y':
            break
        elif confirmation == 'N' or confirmation == 'n':
            continue

    client_socket.send(str.encode(first_name+"/"+last_name+"/"+address+"/"+phone_number+"/"+email_id+"/"+account_type))

    account_no = ''
    while account_no == '':
        account_no = client_socket.recv(64).decode('ascii')
        account_no = account_no.rstrip('\x00')

    print('\033[3;94m' + "\nAccount created successfully\nAccount number is " + '\033[1m' + account_no + '\033[0m')

    data_rcv = ''
    while data_rcv == '':
        data_rcv = client_socket.recv(64).decode()
        data_rcv = data_rcv.rstrip('\x00')

    teller_details = re.split('/', data_rcv)
    user_name = teller_details[0]
    password = teller_details[1]

    password = encrypt.AESCipher('0123456789abcdef').encrypt(password)
    client_socket.send(password)
    #print("User name is = " + user_name)
    #print("Password is = " + password)
    print('\033[3;94m' + "Login details has been emailed to " + '\033[1m' + email_id + '\033[0m')

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def close_account(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Close Account ******\n")

    while True:
        account_no = input('Enter Account number : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()

        client_socket.send(str.encode(account_no))

        #data = client_socket.recv(64).decode()
        #data = data[:-1]

        data = ''
        while data == '':
            data = client_socket.recv(64).decode()
            data = data.rstrip('\x00')

        account_chk = int(data)
        if account_chk == account_check['INCORRECT']:
            print('\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')
            continue
        elif account_chk == account_check['CORRECT']:
            print('\033[3;94m' + "Account with account number " + account_no + " has been closed" + '\033[0m')
            break

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def deposit(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Deposit Money ******\n")

    while True:
        account_no = input('Enter Account number : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()

        while True:
            amount = input('Enter amount to deposit : ')
            if len(amount) == 0:
                continue

            try:
                i = float(amount)
                if i < 0:
                    print('\033[3;31m' + "Please Enter a positive amount\n" + '\033[0m')
                    continue
                else:
                    break
            except ValueError:
                # Handle the exception
                print('\033[3;31m' + 'Please enter an integer in amount field\n' + '\033[0m')
                continue

        client_socket.send(str.encode(amount + "/" + account_no))

        #data = client_socket.recv(64).decode()
        #data = data[:-1]

        data = ''
        while data == '':
            data = client_socket.recv(64).decode()
            data = data.rstrip('\x00')

        account_chk = int(data)
        if account_chk == account_check['INCORRECT']:
            print('\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')
            continue
        elif account_chk == account_check['CORRECT']:
            print('\033[3;94m' + amount + "$ deposited to account number " + account_no + '\033[0m')
            break
    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def withdraw(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Withdraw Money ******\n")

    while True:
        account_no = input('Enter Account number : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()

        while True:
            amount = input('Enter amount to withdraw : ')
            if len(amount) == 0:
                continue

            try:
                i = float(amount)
                break
            except ValueError:
                # Handle the exception
                print('\033[3;31m' + 'Please enter an integer in amount field\n' + '\033[0m')
                continue

        client_socket.send(str.encode(amount + "/" + account_no))

        data = ''
        while data == '':
            data = client_socket.recv(64).decode()
            data = data.rstrip('\x00')

        account_chk = int(data)
        if account_chk == account_check['INCORRECT']:
            print('\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')
            continue
        elif account_chk == account_check['CORRECT']:
            print('\033[3;94m' + amount + "$ withdrawn from account number " + account_no + '\033[0m')
            break
        elif account_chk == account_check['INSUFFICIENT_BALANCE']:
            print('\033[3;31m' + "\nInsufficient balance in account\n" + '\033[0m' + '\033[1;3m' + "Enter another amount\n" + '\033[0m')
            continue

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def modify_customer_details(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Change Customer Details ******\n")

    while True:
        account_no = input('\nEnter Account number : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()

        client_socket.send(str.encode(account_no))

        #data = client_socket.recv(64).decode()
        #data = data[:-1]

        data = ''
        while data == '':
            data = client_socket.recv(64).decode()
            data = data.rstrip('\x00')

        account_chk = int(data)
        if account_chk == account_check['INCORRECT']:
            print(
                '\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')
            continue
        elif account_chk == account_check['CORRECT']:
            break

    while True:

        print("\nSelect the information to be modified")
        print("Please select one at a time....\n")

        print("1. Address\n2. Phone Number\n3. Email Address\n4. Exit")

        selection = input("Select details to be modified : ")

        if selection == '1' or selection == '2' or selection == '3':
            client_socket.send(str.encode(selection))
            if selection == '1':
                while True:
                    changed_data = input("Enter address : ")
                    if len(changed_data) == 0:
                        print('\033[3m' + "Please enter address\n" + '\033[0m')
                        continue
                    else:
                        break

            if selection == '2':
                while True:
                    changed_data = input('Enter Phone Number : ')
                    try:
                        i = int(changed_data)
                        if len(changed_data) != 10 or i < 0:
                            print(
                                '\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                            continue
                        else:
                            break
                    except ValueError:
                        # Handle the exception
                        print(
                            '\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                        continue

            if selection == '3':
                while True:
                    changed_data = input('Enter E-mail address : ')
                    match = re.search(r'[\w.-]+@[\w.-]+.\w+', changed_data)
                    if match:
                        break
                    else:
                        print(
                            '\033[3;31m' + "Incorrect email address\n" + '\033[0m' + '\033[3m' + "Please enter correct email address\n" + '\033[0m')
                        continue

            client_socket.send(str.encode(changed_data))

        elif selection == '4':
            client_socket.send(str.encode(selection))
            return

        else:
            print('\033[1;3m' + 'Choose correct option......\n\n' + '\033[0m')
            continue

        #data = client_socket.recv(64).decode()
        #data = data[:-1]

        data = ''
        while data == '':
            data = client_socket.recv(64).decode()
            data = data.rstrip('\x00')

        info_changed = int(data)
        if info_changed == account_check['INCORRECT']:
            print('\033[3;31m' + "\nInformation could not be changed.\nPlease try again later" + + '\033[0m')
        elif info_changed == account_check['CORRECT']:
            print('\033[3;94m' + "\nInformation changed successfully" + '\033[0m')


def display_customer_details(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n****** Display Customer Details ******\n")

    while True:
        account_no = input('Enter Account number : ')
        if len(account_no) == 0:
            continue
        account_no = account_no.upper()

        client_socket.send(str.encode(account_no))

        data = ''
        while data == '':
            data = client_socket.recv(2048).decode()
            data = data.rstrip('\x00')

        account_chk = int(data)
        if account_chk == account_check['INCORRECT']:
            print('\033[3;31m' + "\nIncorrect Account number\n" + '\033[0m' + '\033[1;3m' + "Please enter again\n" + '\033[0m')
            continue
        elif account_chk == account_check['CORRECT']:
            break

    data = ''
    while data == '':
        data = client_socket.recv(2048).decode()
        data = data.rstrip('\x00')

    s1 = re.split('/', data)
    i = 0

    t = PrettyTable(['Account No.', 'Account Type', 'First Name', 'Last Name', 'Address', 'Phone Number', 'e-mail Address', 'Balance'])

    first_name = s1[i]
    i += 1
    last_name = s1[i]
    i += 1
    address = s1[i]
    i += 1
    phone_number = s1[i]
    i += 1
    email_addr = s1[i]
    i += 1
    balance = s1[i]
    i += 1
    account_type = s1[i]
    t.add_row([account_no, account_type, first_name, last_name, address, phone_number, email_addr, balance])

    print(t)

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return
