# Alumni Management System 🎓

A console-based **Alumni Management System** developed in **C++** using **Data Structures and Algorithms (DSA)** concepts.  
The project manages student and alumni records for different departments and batches while providing secure authentication, data persistence, statistics generation, and academic tracking features.

---

# 📌 Project Overview

The Alumni Management System is designed to maintain and manage records of students and graduates in a university environment.  

The system supports:
- Student record management
- Graduation tracking
- Employment details
- Masters education records
- Department statistics
- Secure login system
- File handling for permanent data storage

The project demonstrates the practical implementation of DSA concepts such as:
- Maps
- Queues
- Stacks
- Priority Queues
- Searching
- Sorting
- File Handling

---

# ✨ Features

-  Secure Admin & HOD Login System
-  Student Record Management
-  Department-wise Data Organization
-  CGPA-Based Student Ranking
-  Graduation Tracking
-  Employment Information Management
-  Masters Education Tracking
-  File-Based Data Persistence
-  Department Statistics & Reports
-  Semester Freeze Management
-  Search Student by Name or Enrollment
-  Batch-wise Student Management
-  View Graduates by Year

---

# 🏢 Departments Included

- Computer Science (CS)
- Software Engineering (SE)
- Electrical Engineering (EE)
- Robotics & Intelligent Systems (RIS)
- Artificial Intelligence (AI)

---

# 🛠️ Technologies Used

- C++
- Object-Oriented Programming (OOP)
- Data Structures & Algorithms
- File Handling
- STL Containers

---

# 📂 Data Structures Used

| Data Structure | Purpose |
|----------------|---------|
| `map` | Department and batch management |
| `vector` | Student storage |
| `queue` | Registration tracking |
| `stack` | Recently added students |
| `priority_queue` | Top CGPA students & graduation sorting |

---

# 🔐 Authentication System

The system contains two roles:

## Admin
Admin has complete control over:
- Adding students
- Updating student records
- Managing graduates
- Viewing statistics
- Marking batches as graduated

## HOD
HOD has view-only access:
- Department statistics
- Employment reports
- Masters statistics
- Batch overviews
- Student records

Passwords are stored securely using file handling.

---

# 👨‍🎓 Student Information Managed

The system stores:
- Name
- Enrollment Number
- Email
- CGPA
- Semester
- Batch
- Address
- Phone Number
- Graduation Status
- Employment Details
- Masters Information
- Frozen Semester Status

---

# 📊 System Functionalities

## Student Management
- Add new students
- Update student details
- Search students
- Display all students

## Alumni Tracking
- Mark students as graduated
- Track graduation year
- Store employment records
- Maintain masters education details

## Statistical Reports
- Department statistics
- Employment statistics
- Masters statistics
- Graduation reports
- Top students by CGPA

---

# 💾 File Handling

The system stores data permanently using text files.

Generated files include:
```txt
admin_credentials.txt
hod_credentials.txt
CS_2025_data.txt
AI_2024_data.txt
