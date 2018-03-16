import login
import re
import os
from prettytable import PrettyTable
import encrypt

actions = {'ADD_TELLER': 1, 'DELETE_TELLER': 2, 'DISPLAY TELLER': 3, 'DISPLAY_DETAILS': 4, 'PENDING_REQUESTS': 5}
account_check = {'CORRECT': 1, 'INCORRECT': 2, 'NO_TELLER_EXIST': 3}

logedin_user = ""


def main_page(client_socket):

    global logedin_user
    logedin_user = login.login_page(client_socket, "A")


    while True:
        os.system('clear')

        print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
        print("\n******************* Menu *******************\n")

        print('\n1. Add Teller\n2. Delete Teller\n3. Display Tellers\n4. Display Customer Details\n5. Check Pending Requests\n6. Logout ')

        selection = input('\nSelect from above options : ')

        if selection == '1' or selection == '2' or selection == '3' or selection == '4' or selection == '5':
            client_socket.send(str.encode(selection))
            if int(selection) == actions['ADD_TELLER']:
                add_teller(client_socket)
            elif int(selection) == actions['DELETE_TELLER']:
                delete_teller(client_socket)
            elif int(selection) == actions['DISPLAY TELLER']:
                display_teller(client_socket)
            elif int(selection) == actions['DISPLAY_DETAILS']:
                display_bank_details(client_socket)
            elif int(selection) == actions['PENDING_REQUESTS']:
                check_pending_requests(client_socket)
            continue
        elif selection == '6':
            client_socket.send(str.encode("0"))
            break
        else:
            print('\033[1;3m' + 'Choose correct option......\n\n' + '\033[0m')
            continue
    return


def add_teller(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n******************* Add Teller *******************\n")

    print('\nFill the following information')

    while True:

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
            phone_number = input('Phone Number : ')
            try:
                i = int(phone_number)
                if len(phone_number) != 10 or i < 0:
                    print('\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                    continue
                else:
                    break
            except ValueError:
                # Handle the exception
                print('\033[3;31m' + "Incorrect phone number\n" + '\033[0m' + '\033[3m' + "Please enter 10 digit phone number\n" + '\033[0m')
                continue

        while True:
            email_id = input('E-mail address : ')

            match = re.search(r'[\w.-]+@[\w.-]+.\w+', email_id)
            if match:
                break
            else:
                print('\033[3;31m' + "Incorrect email address\n" + '\033[0m' + '\033[3m' + "Please enter correct email address\n" + '\033[0m')
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

    client_socket.send(str.encode(first_name+"/"+last_name+"/"+"null"+"/"+phone_number+"/"+email_id+"/"+"Saving"))

    data_rcv = ''
    while data_rcv == '':
        data_rcv = client_socket.recv(64).decode()
        data_rcv = data_rcv.rstrip('\x00')

    teller_details = re.split('/', data_rcv)
    user_name = teller_details[0]
    password = teller_details[1]

    password = encrypt.AESCipher('0123456789abcdef').encrypt(password)
    client_socket.send(password)

    print('\033[3;94m' + "\nTeller Added successfully" + '\033[0m')
    #print("User name is = " + user_name)
    #print("Password is = " + password)
    print('\033[3;94m' + "Login details has been emailed to " + '\033[1m' + email_id + '\033[0m')
    #print("This is temporary password.Please change your password after logging in")

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def delete_teller(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n******************* Delete Teller *******************\n")

    data = ''
    while data == '':
        data = client_socket.recv(1024).decode()
        data = data.rstrip('\x00')

    if data == '3':
        print('\033[1;31m' + "No teller exists.... Please add a teller" + '\033[0m')
        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
        return

    else:
        s1 = re.split('/', data)
        i = 0
        t = PrettyTable(['First Name', 'Last Name', 'User Name'])

        while True:
            first_name = s1[i]
            i += 1
            last_name = s1[i]
            i += 1
            if s1[i] == s1[-1]:
                user_id = s1[i]
                t.add_row([first_name, last_name, user_id])
                break
            else:
                user_id = s1[i]
                t.add_row([first_name, last_name, user_id])
                i += 1

    print(t)
    while True:
        user_id = input('Enter User Name of Teller to be deleted : ')
        if len(user_id) == 0:
            continue

        client_socket.send(str.encode(user_id))

        data_rcv = ''
        while data_rcv == '':
            data_rcv = client_socket.recv(64).decode()
            data_rcv = data_rcv.rstrip('\x00')

        if int(data_rcv) == account_check['INCORRECT']:
            print('\033[1;3;31m' + "\nIncorrect Username\n" + '\033[0m' + '\033[1;3m' + "Please enter correct user name\n" + '\033[0m')
            continue
        elif int(data_rcv) == account_check['CORRECT']:
            print('\033[3;94m' + "Teller deleted with User Name " + '\033[1m' + user_id + '\033[0m')
            break

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def display_bank_details(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n******************* Customer Details *******************\n")
    data = ''
    while data == '':
        data = client_socket.recv(1024).decode()
        data = data.rstrip('\x00')

    try:
        if data == '4' or data == "4":
            #print("first line" + data)
            print('\033[1;31m' + "Customer Database Empty" + '\033[0m')
            ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
            return

        #print("Second line" + data)
        s1 = re.split('/', data)
        i = 0
        #print("Third line" + s1)

        t = PrettyTable(['Account No.', 'Account Type', 'First Name', 'Last Name', 'Balance'])

        total_balance = 0

        while i < len(s1):
            account_no = s1[i]
            i += 1
            first_name = s1[i]
            i += 1
            last_name = s1[i]
            i += 1
            balance = s1[i]
            i += 1
            account_type = s1[i]
            i += 1
            total_balance += int(balance)
            t.add_row([account_no, account_type, first_name, last_name, balance])

        t.add_row(['', '', '', '', ''])
        t.add_row(['', '', '', '\033[1;94m' + 'Total Balance' + '\033[0m', '\033[1;94m' + str(total_balance) + '\033[0m'])
        print(t)

        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')

    except IndexError:
        print('\033[1;31m' + "Customer Database Empty" + '\033[0m')
        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
        return

    return


def check_pending_requests(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n********* Pending Requests from Customers *********\n")

    data = ''
    while data == '':
        data = client_socket.recv(2048).decode()
        data = data.rstrip('\x00')

    if data == '4':
        print('\033[1;31m' + "No pending request found" + '\033[0m')
        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
        return

    s1 = re.split('/', data)
    i = 0

    t = PrettyTable(['Transaction ID.', 'Sender Account', 'Receiver Account', 'Amount'])

    while i < len(s1):
        id = s1[i]
        i += 1
        sender_account = s1[i]
        i += 1
        receiver_account = s1[i]
        i += 1
        amount = s1[i]
        i += 1
        t.add_row([id, sender_account, receiver_account, amount])


    print(t)

    while True:
        selection = input('Enter Transaction ID to be processed or enter "E" to exit : ')
        if len(selection) == 0:
            continue
        else:
            break

    if selection == 'E' or selection == 'e':
        client_socket.send(str.encode("0"))
        return

    client_socket.send(str.encode(selection))

    data_rcv = ''
    while data_rcv == '':
        data_rcv = client_socket.recv(256).decode()
        data_rcv = data_rcv.rstrip('\x00')

    if int(data_rcv) == account_check['CORRECT']:
        print('\033[3;94m' + "\nRequest processed successfully" + '\033[0m')
    elif int(data_rcv) == account_check['INCORRECT']:
        print('\033[3;31m' + "\nInsufficient balance in account : " + sender_account)
        print("Request denied" + '\033[0m')

    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return


def display_teller(client_socket):
    os.system('clear')
    print("\nHello, " + '\033[1;3;92m' + logedin_user + '\033[0m')
    print("\n******************* Teller Details *******************\n")

    data = ''
    while data == '':
        data = client_socket.recv(1024).decode()
        data = data.rstrip('\x00')

    if data == '3':
        print('\033[1;31m' + "No teller exists.... Please add one teller" + '\033[0m')
        ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
        return

    else:
        s1 = re.split('/', data)
        i = 0
        t = PrettyTable(['First Name', 'Last Name', 'User Name'])

        while True:
            first_name = s1[i]
            i += 1
            last_name = s1[i]
            i += 1
            if s1[i] == s1[-1]:
                user_id = s1[i]
                t.add_row([first_name, last_name, user_id])
                break
            else:
                user_id = s1[i]
                t.add_row([first_name, last_name, user_id])
                i += 1

    print(t)
    ignore = input("\n" + '\033[1;3m' + "Press Enter to return to menu" + '\033[0m')
    return