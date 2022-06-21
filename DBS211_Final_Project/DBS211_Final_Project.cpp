/*
==========================================
DBS211 FINAL PROJECT - FINAL SUBMISSION
==========================================
Section Code: NII
Group Number: 07
Group Member#1: 
Group Member#2: 
Group Member#3: Mengyao Liu
==========================================
*/

#include <iostream>
#include <occi.h>
#include "string.h"
#include <iomanip>
#include <cctype>
#include <cstring>
#include <string>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

// Function that gets input from user and validates if it is a valid integer.
int getint(const char* prompt = nullptr);

// Display employee info in the appropriate way.
void displayEmployee(Connection* conn, struct Employee emp);

// Returns 0 if the employee does not exist. Returns 1 otherwise and sets the employee information to the *emp object accordingly.
int findEmployee(Connection* conn, int employeeNumber, struct Employee* emp);

// This function receives a connection pointer and displays all employees� information if exists
void displayAllEmployees(Connection* conn);

int menu(void);

// Inserts the given Employee information into the Employees table
void insertEmployee(Connection* conn, struct Employee emp);

// Updates employee
void updateEmployee(Connection* conn, int employeeNumber);

// This function receives a connection pointer and an integer number as the employee number and deletes a row with the given employee number from table employees.
void deleteEmployee(Connection* conn, int employeeNumber);

struct Employee {
    int employeeNumber;
    char lastName[50];
    char firstName[50];
    char email[100];
    char phone[50];
    char extension[10];
    char reportsTo[100];
    char jobTitle[50];
    char city[50];
};

int main()
{
    // OCCI Variables
    Environment* env = nullptr;
    Connection* conn = nullptr;
    // User Variables
    string usr = ""; // Update with user
    string pass = ""; // Update with password
    string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";

    try
    {
        env = Environment::createEnvironment(Environment::DEFAULT);
        conn = env->createConnection(usr, pass, srv);

        int selection = -1;

        // Calls the menu function until user enters 0 (terminate program)
        while (selection != 0) {
            selection = menu();

            // Find Employee option
            if (selection == 1)
            {
                Employee emp = { 0 };
                displayEmployee(conn, emp);
            }

            // Display All Employees option
            if (selection == 2)
                displayAllEmployees(conn);

            // Add Employee option
            if (selection == 3) {
                // Creating employee object to save entered information
                Employee emp;

                // Prompts user for the information and saves it to emp object
                cout << "\nEmployee Number: ";
                cin >> emp.employeeNumber;
                cout << "\nLast Name: ";
                cin >> emp.lastName;
                cout << "\nFirst Name: ";
                cin >> emp.firstName;
                cout << "\nEmail: ";
                cin >> emp.email;
                cout << "\nextension: ";
                cin >> emp.extension;
                cout << "\nJob Title: ";
                cin.ignore(1000, '\n');
                cin.clear();
                string temp_jobtitle;
                getline(cin, temp_jobtitle, '\n');
                strcpy_s(emp.jobTitle, temp_jobtitle.c_str());
                cout << "\nCity: ";
                cin >> emp.city;

                // Calls insertEmployee function. If employeeNumber already exists, prints error. Otherwise, it the employee is added to the database
                insertEmployee(conn, emp);
            }

            // Update employee option
            if (selection == 4) {
                Employee emp;
                int check;
                // Update Employee data
                cout << "********************* Upd emp *********************" <<
                    endl;
                // Prompt the user to enter a value for the employee number
                cout << "Employee Number: ";
                emp.employeeNumber = getint();
                // Check if the employee with the given employee number exists
                check = findEmployee(conn, emp.employeeNumber, &emp);
                if (check == 0) {
                    // If the returning value of function findEmployee() is 0, display a proper error message.
                    cout << "Employee " << emp.employeeNumber << " does not exist."
                        << endl << endl;
                }
                else {
                    // If employee does exist, ask the user to enter a new phone extension.
                    updateEmployee(conn, emp.employeeNumber);
                }
            }

            // Remove employee option
            if (selection == 5) {
                int employeeNo;
                cout << "Employee Number: ";
                cin >> employeeNo;
                deleteEmployee(conn, employeeNo);
            }
        }        

        env->terminateConnection(conn);
        Environment::terminateEnvironment(env);

    }
    catch (SQLException& sqlExcp)
    {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }

    return 0;

}

int getint(const char* prompt) {
    // Display the prompt message if there is any.
    if (prompt != nullptr)
        cout << prompt;

    int input;
    cin >> input;

    // If the next char in the input stream is not the newline character, the input is invalid.
    // The user will be prompted to reenter a value until a valid integer is entered.
    while (cin.peek() != '\n') {
        cout << "Invalid Integer, retry: ";
        cin.clear();
        cin.ignore(1000, '\n');
        cin >> input;
    }

    // Valid int is returned
    return input;
}

int findEmployee(Connection* conn, int employeeNumber, struct Employee* emp) {
    Statement* stmt = conn->createStatement();

    try {
        // Creating query + appending employeeNumber
        string query = "SELECT e.employeenumber, e.firstname, e.lastname, e.extension, e.email, e.jobtitle, "
            "m.firstname || ' ' || m.lastname, o.phone, o.city FROM employees e INNER JOIN offices o ON o.officecode = e.officecode "
            "LEFT JOIN employees m ON m.employeenumber = e.reportsto WHERE e.employeenumber = " + to_string(employeeNumber);
        ResultSet* rs = stmt->executeQuery(query);

        // Checks if rs.next() returns false (meaning no employee was found)
        if (!rs->next()) {
            conn->terminateStatement(stmt);
            return 0;
        }

        // If the if statement was not executed, the employee was found. So the data can be assigned to the Employee emp object.
        emp->employeeNumber = rs->getInt(1);
        strcpy_s(emp->firstName, rs->getString(2).c_str());
        strcpy_s(emp->lastName, rs->getString(3).c_str());
        strcpy_s(emp->extension, rs->getString(4).c_str());
        strcpy_s(emp->email, rs->getString(5).c_str());
        strcpy_s(emp->jobTitle, rs->getString(6).c_str());
        strcpy_s(emp->reportsTo, rs->getString(7).c_str());
        strcpy_s(emp->phone, rs->getString(8).c_str());
        strcpy_s(emp->city, rs->getString(9).c_str());


        conn->terminateStatement(stmt);
        return 1;
    }
    catch (SQLException& sqlExcp) {
        cout << "Error code" << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage() << endl;
    }
}

void displayEmployee(Connection* conn, struct Employee emp) {
    int employeeNumber = 0;

    // Prompting user to insert value for the employee number
    employeeNumber = getint("Please enter the employee number : ");

    // If employee does not exist, the error message will be printed
    if (findEmployee(conn, employeeNumber, &emp) == 0) {
        cout << "\nEmployee " << employeeNumber << " does not exist.\n" << endl;
        return;
    }

    // If the if statement was not executed, the employee was found and can be displayed
    cout << "\nemployeeNumber = " << emp.employeeNumber << endl;
    cout << "lastName = " << emp.lastName << endl;
    cout << "firstName = " << emp.firstName << endl;
    cout << "email = " << emp.email << endl;
    cout << "phone = " << emp.phone << endl;
    cout << "extension = " << emp.extension << endl;
    cout << "reportsTo = " << emp.reportsTo << endl;
    cout << "jobTitle = " << emp.jobTitle << endl;
    cout << "city = " << emp.city << endl << endl;
}

int menu(void) {
    // User Variables
    int selection; // holds user selection

    cout << "********************* HR Menu *********************" << endl;
    cout <<
        // menu selection options
        "1) Find Employee\n"
        "2) Employees Report\n"
        "3) Add Employee\n"
        "4) Update Employee\n"
        "5) Remove Employee\n"
        "0) Exit\n" << endl;
    cout << "Select an option from the menu " << endl;
    do {
        cin >> selection;
        // selection not in menu
        if (selection < 0 || selection > 5)
        {
            cout << "You have selected an incorrect option" << endl;
        }
    } while (selection < 0 || selection > 5);

    return selection;
}

// This function receives a connection pointer and displays all employees' information if exists
void displayAllEmployees(Connection* conn) {
    bool isEmpty = true;
    // Write a query to select and display the following attributes for all employees
    Statement* stmt = conn->createStatement(); // Create Statement and ResultSet objects
    ResultSet* rs = stmt->executeQuery("SELECT emp.employeenumber, CONCAT(CONCAT(emp.firstName, ' '), emp.lastName) AS employeeName, emp.email, office.phone, emp.extension, CONCAT(CONCAT(mgr.firstName, ' '), mgr.lastName) AS reportsTo FROM employees emp LEFT JOIN employees mgr on emp.reportsTo = mgr.employeenumber JOIN offices office on emp.officeCode = office.officecode ORDER BY employeenumber");
    cout << "E	      Employee Name 	     Email 				                    		    	 Phone           Ext      Manager" << endl;
    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;
    while (rs->next()) {
        isEmpty = false;
        int employeeNum = rs->getInt(1);
        string employeeName = rs->getString(2);
        string email = rs->getString(3);
        string phone = rs->getString(4);
        string ext = rs->getString(5);
        string manager = rs->getString(6);
        // Displays all employees� information 
        cout << endl
            << setw(8) << left << employeeNum
            << setw(20) << left << employeeName
            << setw(36) << left << email
            << setw(20) << left << phone
            << setw(12) << left << ext
            << setw(24) << left << manager
            << endl;
    }
    // If the query does not return any rows, display a proper message
    if (isEmpty == true) {
        cout << "There is no employees� information to be displayed." << endl;
    }
    conn->terminateStatement(stmt); // Terminate the query
};

void insertEmployee(Connection* conn, struct Employee emp) {
    // If findEmployee returns a value different from 0, it means the employee already exists and their info shouldn't be added to the table.
    // Prints "error" message and exits the function.
    if (findEmployee(conn, emp.employeeNumber, &emp) != 0) {
        cout << "\nAn employee with the same employee number exists.\n" << endl;
        return;
    }

    try {
        // A new statement is created
        Statement* stmt = conn->createStatement();

        // Creating query according to emp object's information
        string query = "INSERT INTO employees VALUES(" + to_string(emp.employeeNumber) + ", '" + emp.lastName + "', '" + emp.firstName + "', '" + emp.extension + "', '" + emp.email + "', 1, 1002, '" + emp.jobTitle + "')";
        stmt->executeQuery(query);

        // Successful message is printed to the user
        cout << "\nThe new employee is added successfully.\n" << endl;

        // Terminates statement
        conn->terminateStatement(stmt);
    }
    catch (SQLException& sqlExcp) {
        cout << "Error code" << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage() << endl;
    }
}

void updateEmployee(Connection* conn, int employeeNumber) {
    string temp_extension = "x0000";
    cout << "Enter the new employee extension:";
    cin >> temp_extension;
    try {
        // 1) Exist the emp?
        const string quote = "\"";
        Statement* stmt = conn->createStatement("UPDATE employees SET extension = '" +
            temp_extension + "' WHERE employeenumber = " + to_string(employeeNumber));
        //const char* ex_q = exist_query.c_str(); // string to char
        ResultSet* rs = stmt->executeQuery();
        conn->commit();
        cout << "The employee is updated." << endl;
        delete rs;
        conn->terminateStatement(stmt);
    }
    catch (SQLException& sqlExcp) {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
        cout << "There is an error query statement. Refer error code." << endl
            << endl;
    }
}

void deleteEmployee(Connection* conn, int employeeNumber) {
    struct Employee temp;
    // In function deleteEmployee(), call function findEmployee() to see if the employee with the given employee number exists.
    if (findEmployee(conn, employeeNumber, &temp) == 1) {
        // If the employee exits, delete the row from table employees and display a proper message: "The employee is deleted."
        try {
            Statement* delete_stmt = conn->createStatement("DELETE FROM employees WHERE employeeNumber = " + to_string(employeeNumber));
                delete_stmt->executeUpdate();
            // commit to save the transaction
            conn->commit();
            conn->terminateStatement(delete_stmt);
            cout << "The employee is deleted." << endl << endl;
        }
        // Handle the errors and display the proper message including the error_code. 
        // Error_code is a number returned if the query execution is not successful.
        catch (SQLException& sqlExcp) {
            cout << "Error code" << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage() << endl;
        }
    }
    else {
        // If the employee does not exist display a proper message: "The employee does not exist."
        cout << "The employee does not exist." << endl;
    }

}