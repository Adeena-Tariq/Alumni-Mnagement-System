#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <iomanip>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <regex>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
using namespace std;

// Platform-independent getch implementation
int getch() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// Utility function to mask password with asterisks
string getMaskedPassword() {
    string password = "";
    char ch;
    cout << "Password: ";
    while (true) {
        ch = getch();
        if (ch == '\r' || ch == '\n') break;
        if (ch == '\b' && !password.empty()) {
            password.pop_back();
            cout << "\b \b";
        } else if (ch != '\b') {
            password += ch;
            cout << '*';
        }
    }
    cout << endl;
    return password;
}

// Validation functions
bool isValidName(const string& name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool isValidPhone(const string& phone) {
    regex phonePattern("^[0-9]{4}-[0-9]{7}$");
    return regex_match(phone, phonePattern);
}

bool isValidEmail(const string& email) {
    regex emailPattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex_match(email, emailPattern);
}

bool isValidEnrollment(const string& enrollment) {
    if (enrollment.empty()) return false;
    for (char c : enrollment) {
        if (!isdigit(c) && c != '-') return false;
    }
    return true;
}

bool isValidCompanyName(const string& company) {
    if (company.empty()) return false;
    for (char c : company) {
        if (!isalnum(c) && c != ' ' && c != '&' && c != '-' && c != '.') return false;
    }
    return true;
}

bool isValidPosition(const string& position) {
    if (position.empty()) return false;
    for (char c : position) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool isValidMastersField(const string& field) {
    if (field.empty()) return false;
    for (char c : field) {
        if (!isalpha(c) && c != ' ' && c != '-') return false;
    }
    return true;
}

bool isValidAddress(const string& address) {
    return !address.empty();
}

bool isValidSalary(double salary) {
    return salary >= 0;
}

struct Student {
    string name;
    string enrollmentNo;
    string email;
    double cgpa;
    bool graduated;
    string graduationReason;
    string address;
    string phone;
    int batch;
    int semester;
    
    // Status flags
    bool isFrozen;
    string frozenReason;
    
    // Additional fields
    bool doingMasters;
    string mastersUniversity;
    string mastersDegree;
    
    bool hasJob;
    string company;
    string jobPost;
    double salary;
    
    int yearOfGraduation;

    Student() : cgpa(0.0), graduated(false), yearOfGraduation(-1), batch(0), 
                semester(1), doingMasters(false), hasJob(false), salary(0.0), 
                isFrozen(false) {}
};

// Comparator for CGPA priority queue (higher CGPA first)
struct CGPAComparator {
    bool operator()(const Student& a, const Student& b) {
        return a.cgpa < b.cgpa; // Max heap
    }
};

// Comparator for graduation year (older graduates first)
struct YearComparator {
    bool operator()(const Student& a, const Student& b) {
        return a.yearOfGraduation > b.yearOfGraduation; // Min heap
    }
};

struct Batch {
    int year;
    bool batchGraduated;
    vector<Student> students;
    queue<Student> registrationQueue;
    stack<Student> recentStack;

    Batch(int y = 0, bool grad = false) : year(y), batchGraduated(grad) {}
};

class AlumniSystem {
private:
    map<string, map<int, Batch>> departments;
    priority_queue<Student, vector<Student>, CGPAComparator> cgpaQueue;
    priority_queue<Student, vector<Student>, YearComparator> graduationYearQueue;
    
    string adminUser = "admin";
    string adminPass = "";
    string hodUser = "hod";
    string hodPass = "";
    
    bool adminFirstLogin = true;
    bool hodFirstLogin = true;

public:
    AlumniSystem() {
        loadCredentials();
        loadAllData();
        seedDepartments();
    }
    
    ~AlumniSystem() {
        saveAllData();
    }

    void run() {
        string role = loginSystem();
        if (role == "FAILED") return;
        
        while (true) {
            string deptKey = departmentDashboard();
            if (deptKey == "EXIT") break;
            handleDepartment(deptKey, role);
        }
        cout << "\n*** Exiting Alumni System. Goodbye! ***\n";
    }

private:
    // File handling functions
    void loadCredentials() {
        ifstream adminFile("admin_credentials.txt");
        if (adminFile.is_open()) {
            getline(adminFile, adminPass);
            if (!adminPass.empty()) {
                adminFirstLogin = false;
            }
            adminFile.close();
        }
        
        ifstream hodFile("hod_credentials.txt");
        if (hodFile.is_open()) {
            getline(hodFile, hodPass);
            if (!hodPass.empty()) {
                hodFirstLogin = false;
            }
            hodFile.close();
        }
    }
    
    void saveCredentials(const string& role, const string& password) {
        if (role == "ADMIN") {
            ofstream file("admin_credentials.txt");
            if (file.is_open()) {
                file << password;
                file.close();
            }
        } else if (role == "HOD") {
            ofstream file("hod_credentials.txt");
            if (file.is_open()) {
                file << password;
                file.close();
            }
        }
    }
    
    void saveAllData() {
        for (const auto& deptPair : departments) {
            string deptKey = deptPair.first;
            for (const auto& batchPair : deptPair.second) {
                saveBatchData(deptKey, batchPair.first, batchPair.second);
            }
        }
    }
    
    void saveBatchData(const string& dept, int year, const Batch& batch) {
        string filename = dept + "_" + to_string(year) + "_data.txt";
        ofstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Error: Could not save data for " << dept << " batch " << year << endl;
            return;
        }
        
        file << batch.batchGraduated << "\n";
        file << batch.students.size() << "\n";
        
        for (const auto& st : batch.students) {
            file << st.name << "|"
                 << st.enrollmentNo << "|"
                 << st.email << "|"
                 << st.cgpa << "|"
                 << st.graduated << "|"
                 << st.graduationReason << "|"
                 << st.address << "|"
                 << st.phone << "|"
                 << st.batch << "|"
                 << st.semester << "|"
                 << st.isFrozen << "|"
                 << st.frozenReason << "|"
                 << st.doingMasters << "|"
                 << st.mastersUniversity << "|"
                 << st.mastersDegree << "|"
                 << st.hasJob << "|"
                 << st.company << "|"
                 << st.jobPost << "|"
                 << st.salary << "|"
                 << st.yearOfGraduation << "\n";
        }
        
        file.close();
    }
    
    void loadAllData() {
        vector<string> deptNames = {"CS", "SE", "EE", "RIS", "AI"};
        
        time_t rawtime;
        time(&rawtime);
        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);
        int currentYear = timeinfo.tm_year + 1900;
        
        for (const auto& dept : deptNames) {
            for (int y = 2012; y <= currentYear; y++) {
                loadBatchData(dept, y);
            }
        }
    }
    
    void loadBatchData(const string& dept, int year) {
        string filename = dept + "_" + to_string(year) + "_data.txt";
        ifstream file(filename);
        
        if (!file.is_open()) {
            return; // File doesn't exist yet
        }
        
        Batch& batch = departments[dept][year];
        batch.year = year; // Ensure batch year is set correctly
        
        file >> batch.batchGraduated;
        file.ignore();
        
        int studentCount;
        file >> studentCount;
        file.ignore();
        
        for (int i = 0; i < studentCount; i++) {
            Student st;
            string line;
            getline(file, line);
            
            stringstream ss(line);
            string token;
            
            getline(ss, st.name, '|');
            getline(ss, st.enrollmentNo, '|');
            getline(ss, st.email, '|');
            ss >> st.cgpa; ss.ignore();
            ss >> st.graduated; ss.ignore();
            getline(ss, st.graduationReason, '|');
            getline(ss, st.address, '|');
            getline(ss, st.phone, '|');
            ss >> st.batch; ss.ignore();
            ss >> st.semester; ss.ignore();
            ss >> st.isFrozen; ss.ignore();
            getline(ss, st.frozenReason, '|');
            ss >> st.doingMasters; ss.ignore();
            getline(ss, st.mastersUniversity, '|');
            getline(ss, st.mastersDegree, '|');
            ss >> st.hasJob; ss.ignore();
            getline(ss, st.company, '|');
            getline(ss, st.jobPost, '|');
            ss >> st.salary; ss.ignore();
            ss >> st.yearOfGraduation;
            
            batch.students.push_back(st);
            
            if (st.graduated) {
                graduationYearQueue.push(st);
            }
        }
        
        file.close();
    }

    int getCurrentYear() {
        time_t rawtime;
        time(&rawtime);
        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);
        return timeinfo.tm_year + 1900;
    }

    string loginSystem() {
        int attempts = 0;
        const int maxAttempts = 3;
        
        cout << "\n";
        cout << "============================================\n";
        cout << "     ALUMNI MANAGEMENT SYSTEM - LOGIN\n";
        cout << "============================================\n\n";
        
        while (attempts < maxAttempts) {
            try {
                cout << "Select Role:\n";
                cout << "1. Admin\n";
                cout << "2. HOD\n";
                cout << "3. Exit\n";
                cout << "Choice: ";
                string roleChoice;
                getline(cin, roleChoice);
                
                if (roleChoice == "3") return "FAILED";
                
                string expectedUser, expectedPass, roleName;
                bool isFirstLogin = false;
                
                if (roleChoice == "1") {
                    expectedUser = adminUser;
                    expectedPass = adminPass;
                    roleName = "ADMIN";
                    isFirstLogin = adminFirstLogin;
                } else if (roleChoice == "2") {
                    expectedUser = hodUser;
                    expectedPass = hodPass;
                    roleName = "HOD";
                    isFirstLogin = hodFirstLogin;
                } else {
                    throw invalid_argument("Invalid role selection");
                }
                
                // Username validation loop
                string u;
                while (true) {
                    cout << "\nUsername: ";
                    getline(cin, u);
                    
                    if (u == expectedUser) {
                        break; // Username is correct
                    } else {
                        attempts++;
                        cout << "*** Invalid username! ***\n";
                        cout << "Attempts remaining: " << (maxAttempts - attempts) << "\n";
                        
                        if (attempts >= maxAttempts) {
                            cout << "*** Too many failed attempts. System locked for 1 minute. ***\n";
                            freezeSystem(60);
                            attempts = 0;
                        }
                    }
                }
                
                // Username is correct, now handle password
                if (isFirstLogin) {
                    cout << "\n*** FIRST TIME LOGIN - SET YOUR PASSWORD ***\n";
                    string newPass = getMaskedPassword();
                    
                    if (newPass.length() < 6) {
                        throw invalid_argument("Password must be at least 6 characters");
                    }
                    
                    cout << "Confirm Password: ";
                    string confirmPass = "";
                    char ch;
                    while (true) {
                        ch = getch();
                        if (ch == '\r' || ch == '\n') break;
                        if (ch == '\b' && !confirmPass.empty()) {
                            confirmPass.pop_back();
                            cout << "\b \b";
                        } else if (ch != '\b') {
                            confirmPass += ch;
                            cout << '*';
                        }
                    }
                    cout << endl;
                    
                    if (newPass != confirmPass) {
                        throw invalid_argument("Passwords do not match");
                    }
                    
                    if (roleName == "ADMIN") {
                        adminPass = newPass;
                        adminFirstLogin = false;
                    } else {
                        hodPass = newPass;
                        hodFirstLogin = false;
                    }
                    
                    saveCredentials(roleName, newPass);
                    cout << "\n*** Password set successfully! ***\n";
                    cout << "\n*** Login Successful! Welcome " << roleName << " ***\n\n";
                    return roleName;
                } else {
                    // Password validation loop
                    while (true) {
                        string p = getMaskedPassword();
                        
                        if (p == expectedPass) {
                            cout << "\n*** Login Successful! Welcome " << roleName << " ***\n\n";
                            return roleName;
                        } else {
                            attempts++;
                            cout << "\n*** Wrong password! ***\n";
                            cout << "Attempts remaining: " << (maxAttempts - attempts) << "\n\n";
                            
                            if (attempts >= maxAttempts) {
                                cout << "*** Too many failed attempts. System locked for 1 minute. ***\n";
                                freezeSystem(60);
                                attempts = 0;
                                break; // Break to restart username entry
                            }
                        }
                    }
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
                attempts++;
                if (attempts >= maxAttempts) {
                    cout << "*** Too many failed attempts. System locked for 1 minute. ***\n";
                    freezeSystem(60);
                    attempts = 0;
                }
            }
        }
        return "FAILED";
    }

    void freezeSystem(int seconds) {
        cout << "Please wait";
        for (int i = 0; i < seconds; i++) {
            this_thread::sleep_for(chrono::seconds(1));
            if (i % 10 == 0) cout << ".";
            cout.flush();
        }
        cout << "\n*** System unlocked. You may try again. ***\n\n";
    }

    void seedDepartments() {
        time_t rawtime;
        time(&rawtime);
        struct tm timeinfo;
        localtime_s(&timeinfo, &rawtime);
        int currentYear = timeinfo.tm_year + 1900;

        vector<string> deptNames = {"CS", "SE", "EE", "RIS", "AI"};
        for (auto& dept : deptNames) {
            for (int y = 2012; y <= currentYear; y++) {
                if (departments[dept].find(y) == departments[dept].end()) {
                    departments[dept][y] = Batch(y, false);
                }
            }
        }
    }

    string departmentDashboard() {
        cout << "\n============================================\n";
        cout << "          SELECT DEPARTMENT\n";
        cout << "============================================\n";
        cout << "1. Computer Science (CS)\n";
        cout << "2. Software Engineering (SE)\n";
        cout << "3. Electrical Engineering (EE)\n";
        cout << "4. Robotics & Intelligent Systems (RIS)\n";
        cout << "5. Artificial Intelligence (AI)\n";
        cout << "6. Exit System\n";
        cout << "--------------------------------------------\n";
        cout << "Choice: ";
        
        string choice;
        getline(cin, choice);
        
        if (choice == "1") return "CS";
        if (choice == "2") return "SE";
        if (choice == "3") return "EE";
        if (choice == "4") return "RIS";
        if (choice == "5") return "AI";
        if (choice == "6") return "EXIT";
        
        cout << "*** Invalid choice. Please try again. ***\n";
        return departmentDashboard();
    }

    void handleDepartment(const string& deptKey, const string& role) {
        try {
            if (role == "HOD") {
                // HOD goes directly to their dashboard
                hodDashboard(deptKey);
            } else {
                // Admin gets batch selection
                int year;
                cout << "\nEnter batch year (2012 - 2025): ";
                
                if (!(cin >> year)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    throw invalid_argument("Invalid input for year");
                }
                cin.ignore();
                
                if (departments[deptKey].find(year) == departments[deptKey].end()) {
                    throw out_of_range("Batch year not found");
                }
                
                Batch& batch = departments[deptKey][year];
                adminBatchDashboard(batch, year, deptKey);
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void adminBatchDashboard(Batch& batch, int year, const string& deptKey) {
        while (true) {
            try {
                cout << "\n============================================\n";
                cout << "  ADMIN - " << deptKey << " | BATCH " << year << "\n";
                cout << "============================================\n";
                cout << "1. Add New Student\n";
                cout << "2. View All Students\n";
                cout << "3. Search Particular Student\n";
                cout << "4. View Top Students by CGPA\n";
                cout << "5. View Graduates by Year\n";
                cout << "6. Update Student Details\n";
                cout << "7. Add New Graduate\n";
                cout << "8. Mark Batch as Graduated\n";
                cout << "0. Back to Departments\n";
                cout << "--------------------------------------------\n";
                cout << "Choice: ";
                
                string ch;
                getline(cin, ch);

                if (ch == "1") addStudent(batch, year, deptKey);
                else if (ch == "2") showAllStudentsAdmin(batch);
                else if (ch == "3") searchParticularStudent(batch);
                else if (ch == "4") showTopStudentsByCGPA(batch);
                else if (ch == "5") showGraduatesByYear();
                else if (ch == "6") updateStudentDetails(batch, deptKey);
                else if (ch == "7") addGraduate(batch, year, deptKey);
                else if (ch == "8") markBatchGraduated(batch, deptKey);
                else if (ch == "0") break;
                else throw invalid_argument("Invalid choice");
                
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    }

    void addStudent(Batch& batch, int year, const string& deptKey) {
        try {
            int currentYear = getCurrentYear();
            
            // Check if this is current batch or old batch
            bool isCurrentBatch = (year == currentYear);
            
            if (!isCurrentBatch) {
                cout << "\n*** This is not the current batch. ***\n";
                cout << "*** Only existing students can be added to old batches. ***\n";
                cout << "\nDo you want to add an existing student? (1=Yes, 0=No): ";
                int addExisting;
                if (!(cin >> addExisting)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    throw invalid_argument("Invalid input");
                }
                cin.ignore();
                
                if (addExisting != 1) {
                    return;
                }
            }
            
            Student s;
            s.batch = year; // Set batch to the selected year
            
            cout << "\n--- Add " << (isCurrentBatch ? "New" : "Existing") << " Student ---\n";
            
            // Name validation (alphabets only)
            while (true) {
                cout << "Name (alphabets only): ";
                getline(cin, s.name);
                if (isValidName(s.name)) break;
                cout << "*** Invalid name. Use only alphabets and spaces. ***\n";
            }
            
            // Enrollment validation (digits and dashes only)
            while (true) {
                cout << "Enrollment No (digits and dashes only): ";
                getline(cin, s.enrollmentNo);
                if (isValidEnrollment(s.enrollmentNo)) break;
                cout << "*** Invalid enrollment number. Use digits and dashes only. ***\n";
            }
            
            // Email validation
            while (true) {
                cout << "Email: ";
                getline(cin, s.email);
                if (isValidEmail(s.email)) break;
                cout << "*** Invalid email format. Use format: example@domain.com ***\n";
            }
            
            // CGPA validation
            while (true) {
                cout << "CGPA (0.0 - 4.0): ";
                if (cin >> s.cgpa && s.cgpa >= 0 && s.cgpa <= 4.0) {
                    cin.ignore();
                    break;
                }
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "*** Invalid CGPA. Enter value between 0.0 and 4.0 ***\n";
            }
            
            // Semester validation
            while (true) {
                cout << "Semester (1-8): ";
                if (cin >> s.semester && s.semester >= 1 && s.semester <= 8) {
                    cin.ignore();
                    break;
                }
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "*** Invalid semester. Enter value between 1 and 8 ***\n";
            }
            
            // Address validation
            while (true) {
                cout << "Address: ";
                getline(cin, s.address);
                if (isValidAddress(s.address)) break;
                cout << "*** Address cannot be empty ***\n";
            }
            
            // Phone validation
            while (true) {
                cout << "Phone (format: XXXX-XXXXXXX): ";
                getline(cin, s.phone);
                if (isValidPhone(s.phone)) break;
                cout << "*** Invalid phone format. Use format: 0300-1234567 ***\n";
            }
            
            // Check if frozen
            cout << "Is semester frozen? (1=Yes, 0=No): ";
            int frozen;
            while (!(cin >> frozen) || (frozen != 0 && frozen != 1)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "*** Invalid input. Enter 1 for Yes or 0 for No: ";
            }
            cin.ignore();
            
            s.isFrozen = (frozen == 1);
            if (s.isFrozen) {
                cout << "Reason for freezing: ";
                getline(cin, s.frozenReason);
            }
            
            // Check graduation status
            cout << "Has student graduated in their batch? (1=Yes, 0=No): ";
            int gradStatus;
            while (!(cin >> gradStatus) || (gradStatus != 0 && gradStatus != 1)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "*** Invalid input. Enter 1 for Yes or 0 for No: ";
            }
            cin.ignore();
            
            s.graduated = (gradStatus == 1);
            
            if (s.graduated) {
                s.graduationReason = "Graduated successfully";
                cout << "Year of Graduation: ";
                while (!(cin >> s.yearOfGraduation)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "*** Invalid year. Try again: ";
                }
                cin.ignore();
                
                // Check for Masters
                cout << "Doing Masters? (1=Yes, 0=No): ";
                int masters;
                while (!(cin >> masters) || (masters != 0 && masters != 1)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "*** Invalid input. Enter 1 for Yes or 0 for No: ";
                }
                cin.ignore();
                
                if (masters == 1) {
                    s.doingMasters = true;
                    while (true) {
                        cout << "Masters University (alphabets and dashes only): ";
                        getline(cin, s.mastersUniversity);
                        if (isValidMastersField(s.mastersUniversity)) break;
                        cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                    }
                    while (true) {
                        cout << "Masters Degree (alphabets and dashes only): ";
                        getline(cin, s.mastersDegree);
                        if (isValidMastersField(s.mastersDegree)) break;
                        cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                    }
                }
                
                // Check for Job
                cout << "Has Job? (1=Yes, 0=No): ";
                int job;
                while (!(cin >> job) || (job != 0 && job != 1)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "*** Invalid input. Enter 1 for Yes or 0 for No: ";
                }
                cin.ignore();
                
                if (job == 1) {
                    s.hasJob = true;
                    while (true) {
                        cout << "Company: ";
                        getline(cin, s.company);
                        if (isValidCompanyName(s.company)) break;
                        cout << "*** Invalid company name. ***\n";
                    }
                    while (true) {
                        cout << "Position (alphabets only): ";
                        getline(cin, s.jobPost);
                        if (isValidPosition(s.jobPost)) break;
                        cout << "*** Invalid. Use alphabets only. ***\n";
                    }
                    cout << "Salary: ";
                    while (!(cin >> s.salary) || !isValidSalary(s.salary)) {
                        cin.clear();
                        cin.ignore(10000, '\n');
                        cout << "*** Invalid salary. Enter positive number: ";
                    }
                    cin.ignore();
                }
                
                graduationYearQueue.push(s);
            } else {
                if (s.semester < 8) {
                    s.graduationReason = "Currently Enrolled";
                } else {
                    cout << "Reason for not graduating: ";
                    getline(cin, s.graduationReason);
                }
            }

            batch.students.push_back(s);
            batch.registrationQueue.push(s);
            batch.recentStack.push(s);
            
            saveBatchData(deptKey, year, batch);

            cout << "\n*** Student added successfully! ***\n";
        } catch (const exception& e) {
            cout << "Error adding student: " << e.what() << "\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    void showAllStudentsAdmin(const Batch& batch) {
        if (batch.students.empty()) {
            cout << "\n*** No students in this batch ***\n";
            return;
        }
        
        cout << "\n" << string(150, '=') << "\n";
        cout << "                                   ALL STUDENTS - BATCH " << batch.year << "\n";
        cout << string(150, '=') << "\n\n";
        
        cout << left << setw(20) << "Name"
             << setw(15) << "Enrollment"
             << setw(7) << "Batch"
             << setw(7) << "CGPA"
             << setw(10) << "Status"
             << setw(25) << "Email"
             << setw(15) << "Phone"
             << setw(20) << "Masters"
             << setw(20) << "Company"
             << setw(15) << "Position" << "\n";
        cout << string(150, '-') << "\n";
        
        for (const auto& st : batch.students) {
            string status = st.graduated ? "Graduated" : (st.isFrozen ? "Frozen" : "Active");
            string masters = st.doingMasters ? st.mastersUniversity.substr(0, 18) : "None";
            string company = st.hasJob ? st.company.substr(0, 18) : "None";
            string position = st.hasJob ? st.jobPost.substr(0, 13) : "None";
            
            cout << left << setw(20) << st.name
                 << setw(15) << st.enrollmentNo
                 << setw(7) << st.batch
                 << setw(7) << fixed << setprecision(2) << st.cgpa
                 << setw(10) << status
                 << setw(25) << st.email
                 << setw(15) << st.phone
                 << setw(20) << masters
                 << setw(20) << company
                 << setw(15) << position << "\n";
        }
        cout << string(150, '-') << "\n";
    }

    void searchParticularStudent(const Batch& batch) {
        try {
            cout << "\n--- Search Student ---\n";
            cout << "1. Search by Name\n";
            cout << "2. Search by Enrollment No\n";
            cout << "Choice: ";
            string choice;
            getline(cin, choice);

            if (choice != "1" && choice != "2") {
                throw invalid_argument("Invalid choice");
            }

            string searchKey;
            if (choice == "1") {
                cout << "Enter Name: ";
                getline(cin, searchKey);
            } else {
                cout << "Enter Enrollment No: ";
                getline(cin, searchKey);
            }

            bool found = false;
            for (const auto& st : batch.students) {
                if ((choice == "1" && st.name == searchKey) || 
                    (choice == "2" && st.enrollmentNo == searchKey)) {
                    displayStudentDetails(st);
                    found = true;
                    break;
                }
            }

            if (!found) {
                cout << "\n*** No matching student found ***\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void displayStudentDetails(const Student& st) {
        cout << "\n============================================\n";
        cout << "          STUDENT DETAILS\n";
        cout << "============================================\n";
        cout << "Name:           " << st.name << "\n";
        cout << "Enrollment:     " << st.enrollmentNo << "\n";
        cout << "Email:          " << st.email << "\n";
        cout << "CGPA:           " << fixed << setprecision(2) << st.cgpa << "\n";
        cout << "Batch:          " << st.batch << "\n";
        cout << "Semester:       " << st.semester << "\n";
        cout << "Graduated:      " << (st.graduated ? "Yes" : "No") << "\n";
        cout << "Status:         " << st.graduationReason << "\n";
        cout << "Address:        " << st.address << "\n";
        cout << "Phone:          " << st.phone << "\n";
        
        if (st.isFrozen) {
            cout << "\n--- Semester Status ---\n";
            cout << "Frozen:         Yes\n";
            cout << "Reason:         " << st.frozenReason << "\n";
        }
        
        if (st.doingMasters) {
            cout << "\n--- Masters Education ---\n";
            cout << "University:     " << st.mastersUniversity << "\n";
            cout << "Degree:         " << st.mastersDegree << "\n";
        }
        
        if (st.hasJob) {
            cout << "\n--- Employment ---\n";
            cout << "Company:        " << st.company << "\n";
            cout << "Position:       " << st.jobPost << "\n";
            cout << "Salary:         $" << st.salary << "\n";
        }
        
        if (st.graduated) {
            cout << "Grad Year:      " << st.yearOfGraduation << "\n";
        }
        cout << "============================================\n";
    }

    void showTopStudentsByCGPA(const Batch& batch) {
        if (batch.students.empty()) {
            cout << "\n*** No students in this batch ***\n";
            return;
        }

        priority_queue<Student, vector<Student>, CGPAComparator> pq;
        for (const auto& st : batch.students) {
            pq.push(st);
        }

        cout << "\n============================================\n";
        cout << "       TOP STUDENTS BY CGPA\n";
        cout << "============================================\n";
        
        int rank = 1;
        while (!pq.empty() && rank <= 10) {
            Student top = pq.top();
            pq.pop();
            cout << rank << ". " << left << setw(25) << top.name 
                 << "CGPA: " << fixed << setprecision(2) << top.cgpa 
                 << " | " << top.enrollmentNo << "\n";
            rank++;
        }
        cout << "============================================\n";
    }

    void showGraduatesByYear() {
        priority_queue<Student, vector<Student>, YearComparator> tempQueue = graduationYearQueue;
        
        if (tempQueue.empty()) {
            cout << "\n*** No graduated students yet ***\n";
            return;
        }

        cout << "\n============================================\n";
        cout << "     GRADUATES BY YEAR (Oldest First)\n";
        cout << "============================================\n";
        
        cout << left << setw(25) << "Name"
             << setw(15) << "Enrollment"
             << setw(12) << "Grad Year"
             << setw(8) << "CGPA" << "\n";
        cout << string(60, '-') << "\n";
        
        while (!tempQueue.empty()) {
            Student st = tempQueue.top();
            tempQueue.pop();
            cout << left << setw(25) << st.name
                 << setw(15) << st.enrollmentNo
                 << setw(12) << st.yearOfGraduation
                 << setw(8) << fixed << setprecision(2) << st.cgpa << "\n";
        }
        cout << "============================================\n";
    }

    void updateStudentDetails(Batch& batch, const string& deptKey) {
        try {
            cout << "\nEnter Enrollment No of student to update: ";
            string enr;
            getline(cin, enr);

            for (auto& st : batch.students) {
                if (st.enrollmentNo == enr) {
                    cout << "\n--- Update Student: " << st.name << " ---\n";
                    cout << "1. Update CGPA\n";
                    cout << "2. Update Semester\n";
                    cout << "3. Update Phone\n";
                    cout << "4. Update Email\n";
                    cout << "5. Update Address\n";
                    cout << "6. Update Employment\n";
                    cout << "7. Update Masters Info\n";
                    cout << "8. Update Frozen Status\n";
                    cout << "0. Cancel\n";
                    cout << "Choice: ";
                    
                    string choice;
                    getline(cin, choice);

                    if (choice == "1") {
                        cout << "New CGPA (0.0-4.0): ";
                        double newCGPA;
                        if (!(cin >> newCGPA) || newCGPA < 0 || newCGPA > 4.0) {
                            throw invalid_argument("Invalid CGPA");
                        }
                        cin.ignore();
                        st.cgpa = newCGPA;
                        cout << "*** CGPA updated ***\n";
                    } else if (choice == "2") {
                        cout << "New Semester (1-8): ";
                        int newSem;
                        if (!(cin >> newSem) || newSem < 1 || newSem > 8) {
                            throw invalid_argument("Invalid semester");
                        }
                        cin.ignore();
                        st.semester = newSem;
                        cout << "*** Semester updated ***\n";
                    } else if (choice == "3") {
                        while (true) {
                            cout << "New Phone (format: XXXX-XXXXXXX): ";
                            string newPhone;
                            getline(cin, newPhone);
                            if (isValidPhone(newPhone)) {
                                st.phone = newPhone;
                                cout << "*** Phone updated ***\n";
                                break;
                            }
                            cout << "*** Invalid format. Try again. ***\n";
                        }
                    } else if (choice == "4") {
                        while (true) {
                            cout << "New Email: ";
                            string newEmail;
                            getline(cin, newEmail);
                            if (isValidEmail(newEmail)) {
                                st.email = newEmail;
                                cout << "*** Email updated ***\n";
                                break;
                            }
                            cout << "*** Invalid email format. Try again. ***\n";
                        }
                    } else if (choice == "5") {
                        while (true) {
                            cout << "New Address: ";
                            string newAddress;
                            getline(cin, newAddress);
                            if (isValidAddress(newAddress)) {
                                st.address = newAddress;
                                cout << "*** Address updated ***\n";
                                break;
                            }
                            cout << "*** Address cannot be empty. Try again. ***\n";
                        }
                    } else if (choice == "6") {
                        cout << "Has Job? (1=Yes, 0=No): ";
                        int hasJob;
                        if (!(cin >> hasJob) || (hasJob != 0 && hasJob != 1)) {
                            throw invalid_argument("Invalid input");
                        }
                        cin.ignore();
                        st.hasJob = (hasJob == 1);
                        if (st.hasJob) {
                            while (true) {
                                cout << "Company: ";
                                getline(cin, st.company);
                                if (isValidCompanyName(st.company)) break;
                                cout << "*** Invalid company name. ***\n";
                            }
                            while (true) {
                                cout << "Position (alphabets only): ";
                                getline(cin, st.jobPost);
                                if (isValidPosition(st.jobPost)) break;
                                cout << "*** Invalid. Use alphabets only. ***\n";
                            }
                            cout << "Salary: ";
                            while (!(cin >> st.salary) || !isValidSalary(st.salary)) {
                                cin.clear();
                                cin.ignore(10000, '\n');
                                cout << "*** Invalid salary. Enter positive number: ";
                            }
                            cin.ignore();
                        }
                        cout << "*** Employment info updated ***\n";
                    } else if (choice == "7") {
                        cout << "Doing Masters? (1=Yes, 0=No): ";
                        int doingMasters;
                        if (!(cin >> doingMasters) || (doingMasters != 0 && doingMasters != 1)) {
                            throw invalid_argument("Invalid input");
                        }
                        cin.ignore();
                        st.doingMasters = (doingMasters == 1);
                        if (st.doingMasters) {
                            while (true) {
                                cout << "University (alphabets and dashes only): ";
                                getline(cin, st.mastersUniversity);
                                if (isValidMastersField(st.mastersUniversity)) break;
                                cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                            }
                            while (true) {
                                cout << "Degree (alphabets and dashes only): ";
                                getline(cin, st.mastersDegree);
                                if (isValidMastersField(st.mastersDegree)) break;
                                cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                            }
                        }
                        cout << "*** Masters info updated ***\n";
                    } else if (choice == "8") {
                        cout << "Is semester frozen? (1=Yes, 0=No): ";
                        int frozen;
                        if (!(cin >> frozen) || (frozen != 0 && frozen != 1)) {
                            throw invalid_argument("Invalid input");
                        }
                        cin.ignore();
                        st.isFrozen = (frozen == 1);
                        if (st.isFrozen) {
                            cout << "Reason for freezing: ";
                            getline(cin, st.frozenReason);
                        } else {
                            st.frozenReason = "";
                        }
                        cout << "*** Frozen status updated ***\n";
                    } else if (choice == "0") {
                        return;
                    } else {
                        throw invalid_argument("Invalid choice");
                    }
                    
                    saveBatchData(deptKey, batch.year, batch);
                    return;
                }
            }
            cout << "\n*** Student not found ***\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    void addGraduate(Batch& batch, int year, const string& deptKey) {
        try {
            cout << "\nEnter Enrollment No of graduating student: ";
            string enr;
            getline(cin, enr);

            for (auto& st : batch.students) {
                if (st.enrollmentNo == enr) {
                    st.graduated = true;
                    
                    cout << "Graduation Year: ";
                    if (!(cin >> st.yearOfGraduation)) {
                        throw invalid_argument("Invalid year");
                    }
                    cin.ignore();
                    
                    st.graduationReason = "Graduated successfully";
                    
                    cout << "Doing Masters? (1=Yes, 0=No): ";
                    int masters;
                    if (!(cin >> masters) || (masters != 0 && masters != 1)) {
                        throw invalid_argument("Invalid input");
                    }
                    cin.ignore();
                    
                    if (masters == 1) {
                        st.doingMasters = true;
                        while (true) {
                            cout << "Masters University (alphabets and dashes only): ";
                            getline(cin, st.mastersUniversity);
                            if (isValidMastersField(st.mastersUniversity)) break;
                            cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                        }
                        while (true) {
                            cout << "Masters Degree (alphabets and dashes only): ";
                            getline(cin, st.mastersDegree);
                            if (isValidMastersField(st.mastersDegree)) break;
                            cout << "*** Invalid. Use alphabets and dashes only. ***\n";
                        }
                    }
                    
                    cout << "Has Job? (1=Yes, 0=No): ";
                    int job;
                    if (!(cin >> job) || (job != 0 && job != 1)) {
                        throw invalid_argument("Invalid input");
                    }
                    cin.ignore();
                    
                    if (job == 1) {
                        st.hasJob = true;
                        while (true) {
                            cout << "Company: ";
                            getline(cin, st.company);
                            if (isValidCompanyName(st.company)) break;
                            cout << "*** Invalid company name. ***\n";
                        }
                        while (true) {
                            cout << "Position (alphabets only): ";
                            getline(cin, st.jobPost);
                            if (isValidPosition(st.jobPost)) break;
                            cout << "*** Invalid. Use alphabets only. ***\n";
                        }
                        cout << "Salary: ";
                        while (!(cin >> st.salary) || !isValidSalary(st.salary)) {
                            cin.clear();
                            cin.ignore(10000, '\n');
                            cout << "*** Invalid salary. Enter positive number: ";
                        }
                        cin.ignore();
                    }
                    
                    graduationYearQueue.push(st);
                    saveBatchData(deptKey, year, batch);
                    
                    cout << "\n*** Student marked as graduated! ***\n";
                    return;
                }
            }
            cout << "\n*** Student not found ***\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    void markBatchGraduated(Batch& batch, const string& deptKey) {
        try {
            batch.batchGraduated = true;
            for (auto& st : batch.students) {
                if (!st.graduated) {
                    st.graduationReason = "Failed/Dropped";
                }
            }
            
            saveBatchData(deptKey, batch.year, batch);
            
            cout << "\n*** Batch " << batch.year << " marked as graduated ***\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    // HOD Dashboard - View Only
    void hodDashboard(const string& deptKey) {
        while (true) {
            try {
                cout << "\n============================================\n";
                cout << "          HOD DASHBOARD - " << deptKey << "\n";
                cout << "============================================\n";
                cout << "1. View Department Statistics\n";
                cout << "2. View All Batches Overview\n";
                cout << "3. Generate Department Report\n";
                cout << "4. View Employment Statistics\n";
                cout << "5. View Masters Statistics\n";
                cout << "6. View All Students (View-Only)\n";
                cout << "0. Back\n";
                cout << "--------------------------------------------\n";
                cout << "Choice: ";
                
                string choice;
                getline(cin, choice);
                
                if (choice == "1") {
                    showDepartmentStats(deptKey);
                } else if (choice == "2") {
                    showAllBatchesOverview(deptKey);
                } else if (choice == "3") {
                    generateDepartmentReport(deptKey);
                } else if (choice == "4") {
                    showEmploymentStats(deptKey);
                } else if (choice == "5") {
                    showMastersStats(deptKey);
                } else if (choice == "6") {
                    viewAllStudentsHOD(deptKey);
                } else if (choice == "0") {
                    break;
                } else {
                    throw invalid_argument("Invalid choice");
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    }

    void viewAllStudentsHOD(const string& deptKey) {
        try {
            cout << "\nEnter batch year to view (2012 - 2025): ";
            int year;
            if (!(cin >> year)) {
                cin.clear();
                cin.ignore(10000, '\n');
                throw invalid_argument("Invalid year");
            }
            cin.ignore();
            
            if (departments[deptKey].find(year) == departments[deptKey].end()) {
                throw out_of_range("Batch year not found");
            }
            
            const Batch& batch = departments[deptKey][year];
            
            if (batch.students.empty()) {
                cout << "\n*** No students in this batch ***\n";
                return;
            }
            
            cout << "\n" << string(150, '=') << "\n";
            cout << "                          VIEW-ONLY - ALL STUDENTS - BATCH " << year << "\n";
            cout << string(150, '=') << "\n\n";
            
            cout << left << setw(20) << "Name"
                 << setw(15) << "Enrollment"
                 << setw(7) << "Batch"
                 << setw(7) << "CGPA"
                 << setw(10) << "Status"
                 << setw(25) << "Email"
                 << setw(15) << "Phone"
                 << setw(20) << "Masters"
                 << setw(20) << "Company"
                 << setw(15) << "Position" << "\n";
            cout << string(150, '-') << "\n";
            
            for (const auto& st : batch.students) {
                string status = st.graduated ? "Graduated" : (st.isFrozen ? "Frozen" : "Active");
                string masters = st.doingMasters ? st.mastersUniversity.substr(0, 18) : "None";
                string company = st.hasJob ? st.company.substr(0, 18) : "None";
                string position = st.hasJob ? st.jobPost.substr(0, 13) : "None";
                
                cout << left << setw(20) << st.name
                     << setw(15) << st.enrollmentNo
                     << setw(7) << st.batch
                     << setw(7) << fixed << setprecision(2) << st.cgpa
                     << setw(10) << status
                     << setw(25) << st.email
                     << setw(15) << st.phone
                     << setw(20) << masters
                     << setw(20) << company
                     << setw(15) << position << "\n";
            }
            cout << string(150, '-') << "\n";
            
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void showDepartmentStats(const string& deptKey) {
        try {
            int totalStudents = 0;
            int totalGraduates = 0;
            int totalFrozen = 0;
            double avgCGPA = 0.0;
            
            for (auto& batchPair : departments[deptKey]) {
                totalStudents += batchPair.second.students.size();
                for (const auto& st : batchPair.second.students) {
                    avgCGPA += st.cgpa;
                    if (st.graduated) totalGraduates++;
                    if (st.isFrozen) totalFrozen++;
                }
            }
            
            if (totalStudents > 0) avgCGPA /= totalStudents;
            
            cout << "\n============================================\n";
            cout << "  " << deptKey << " DEPARTMENT STATISTICS\n";
            cout << "============================================\n";
            cout << "Total Students:      " << totalStudents << "\n";
            cout << "Total Graduates:     " << totalGraduates << "\n";
            cout << "Frozen Semesters:    " << totalFrozen << "\n";
            cout << "Average CGPA:        " << fixed << setprecision(2) << avgCGPA << "\n";
            cout << "Graduation Rate:     " << fixed << setprecision(1) 
                 << (totalStudents > 0 ? (totalGraduates * 100.0 / totalStudents) : 0.0) << "%\n";
            cout << "============================================\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void showAllBatchesOverview(const string& deptKey) {
        try {
            cout << "\n============================================\n";
            cout << "  ALL BATCHES IN " << deptKey << "\n";
            cout << "============================================\n";
            cout << left << setw(10) << "Year" 
                 << setw(12) << "Students" 
                 << setw(12) << "Graduated"
                 << setw(10) << "Frozen" << "\n";
            cout << string(44, '-') << "\n";
            
            for (auto& batchPair : departments[deptKey]) {
                int graduated = 0;
                int frozen = 0;
                for (const auto& st : batchPair.second.students) {
                    if (st.graduated) graduated++;
                    if (st.isFrozen) frozen++;
                }
                cout << left << setw(10) << batchPair.first
                     << setw(12) << batchPair.second.students.size()
                     << setw(12) << graduated
                     << setw(10) << frozen << "\n";
            }
            cout << "============================================\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void showEmploymentStats(const string& deptKey) {
        try {
            int totalEmployed = 0;
            int totalGraduates = 0;
            double avgSalary = 0.0;
            int salaryCount = 0;
            
            cout << "\n============================================\n";
            cout << "  EMPLOYMENT STATISTICS - " << deptKey << "\n";
            cout << "============================================\n";
            
            for (auto& batchPair : departments[deptKey]) {
                for (const auto& st : batchPair.second.students) {
                    if (st.graduated) {
                        totalGraduates++;
                        if (st.hasJob) {
                            totalEmployed++;
                            if (st.salary > 0) {
                                avgSalary += st.salary;
                                salaryCount++;
                            }
                        }
                    }
                }
            }
            
            if (salaryCount > 0) avgSalary /= salaryCount;
            
            cout << "Total Graduates:     " << totalGraduates << "\n";
            cout << "Total Employed:      " << totalEmployed << "\n";
            cout << "Employment Rate:     " << fixed << setprecision(1) 
                 << (totalGraduates > 0 ? (totalEmployed * 100.0 / totalGraduates) : 0.0) << "%\n";
            cout << "Average Salary:      $" << fixed << setprecision(2) << avgSalary << "\n";
            cout << "============================================\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void showMastersStats(const string& deptKey) {
        try {
            int totalPursuingMasters = 0;
            int totalGraduates = 0;
            
            cout << "\n============================================\n";
            cout << "  MASTERS EDUCATION STATISTICS - " << deptKey << "\n";
            cout << "============================================\n";
            
            for (auto& batchPair : departments[deptKey]) {
                for (const auto& st : batchPair.second.students) {
                    if (st.graduated) {
                        totalGraduates++;
                        if (st.doingMasters) {
                            totalPursuingMasters++;
                        }
                    }
                }
            }
            
            cout << "Total Graduates:     " << totalGraduates << "\n";
            cout << "Pursuing Masters:    " << totalPursuingMasters << "\n";
            cout << "Masters Rate:        " << fixed << setprecision(1) 
                 << (totalGraduates > 0 ? (totalPursuingMasters * 100.0 / totalGraduates) : 0.0) << "%\n";
            cout << "============================================\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void generateDepartmentReport(const string& deptKey) {
        try {
            cout << "\n============================================\n";
            cout << "       COMPREHENSIVE DEPARTMENT REPORT\n";
            cout << "============================================\n";
            
            time_t now = time(0);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now);
            char dt[26];
            asctime_s(dt, sizeof(dt), &timeinfo);
            cout << "Department: " << deptKey << "\n";
            cout << "Generated:  " << dt;
            cout << "============================================\n\n";
            
            showDepartmentStats(deptKey);
            cout << "\n";
            showEmploymentStats(deptKey);
            cout << "\n";
            showMastersStats(deptKey);
            cout << "\n";
            showAllBatchesOverview(deptKey);
            
            cout << "\n*** Report Generated Successfully ***\n";
        } catch (const exception& e) {
            cout << "Error generating report: " << e.what() << "\n";
        }
    }
};

int main() {
    try {
        AlumniSystem app;
        app.run();
    } catch (const exception& e) {
        cout << "Critical Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}