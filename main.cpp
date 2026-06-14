#include <iostream>
#include <cmath>
using namespace std;

int main() {
    string in = "";
    int time = 1;

    cout << "Welcome to the to Money Network Inc " << endl;
    cout << "-------------------------------------------" << endl;


    while(in != "done" && in != "4"){
        cout << "1. View Accounts" << endl;
        cout << "2. Create Account" << endl;
        cout << "3. Access Account" << endl;
        cout << "4. Done" << endl;
        cout << endl;
        cout << "What do you want to do? ";

        //convert input to lowercase
        cin >> in;
        for (char& c : in) {
            c = tolower(c);
        }

        if(in == "view" || in == "1"){

        }

        if(in == "create" || in == "2"){
            int accountNum = 1; //placeholder (account number will be the size of the network)
            cout<< "------------------" << endl;
            cout<< "    Account number: " << accountNum << endl;
            cout<< "    How much money would you like to deposit into the account to begin with? ";
            int CashAmount = 0;
            cin >> CashAmount;
            //Adding account to netwrok code here


            //
            cout<< "    Account added to Network"<< endl;
            cout<< "------------------" << endl;


        }

        if(in == "access" || in == "3"){
            int accountNum = 1;
            cout<< "------------------" << endl;
            cout<< "    What accoutn do you want to acess";
            cin >> accountNum;

            //options

            cout<< "------------------" << endl;
            
        }

        time++;

        

    }

    return 0;
}