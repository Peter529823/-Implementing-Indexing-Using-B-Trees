# -Implementing-Indexing-Using-B-Trees

This program implements a student database system that utilizes B-Trees for efficient indexing and retrieval of records. The system processes a data file containing student records and a command file that specifies operations such as searching, adding, and dumping records. The primary data structure used in the program is the B-Tree which ensures efficient searching and insertion operations. The student records are stored in a structured format with fixed-length fields including an ID, last name, first name, year, major, and email address. The database is built by reading the student records from the input file and writing them to a binary file while simultaneously indexing them using two B-Trees: one for student IDs and another for last names. The B-Tree nodes contain keys (IDs or last names) positions of records in the file and pointers to child nodes making the retrieval process efficient.

The program supports several operations including finding a student record by ID or last name. The program traverses the corresponding Tree to locate the record's position in the database file and then retrieves and displays the relevant information. A message is returned if a record is not found. The insertion operation allows new student records to be added to the database while maintaining the B-Tree structure to ensure efficient future searches. A splitting operation is performed to maintain balance and optimize retrieval time when a node becomes full. Additionally, the program includes a dump operation that prints all student records sorted either by ID or last name. This feature enables users to view the database in a structured manner. The program processes the command file sequentially, executing each command and writing the results to an output file.

The implementation is structured to handle file input and output efficiently ensuring data persistence across multiple executions. The use of binary files for storing student records minimizes storage overhead and improves speed. The program achieves efficient search, insert, and sort operations, making it well-suited for handling large datasets while maintaining optimal performance.

The datafile1.txt and cmdfile1.txt files serve as inputs to the program enabling it to build a student database and process various commands to generate outfile1.txt.
Role of datafile1.txt. The datafile1.txt file contains student records each formatted with six fields:
•	ID (9 digits)
•	Last Name (up to 15 characters)
•	First Name (up to 15 characters)
•	Year (1 digit)
•	Major (up to 4 characters)
•	Email (up to 20 characters)
Each record is read from datafile1.txt, converted into a fixed-length format (64 bytes), and stored in the database file (student1.dat). The records are indexed using two b-trees. 
•	ID-based B-Tree (student1.ix1) - Uses the student ID as the key.
•	Last Name-based B-Tree (student1.ix2) - Uses the last name as the key.
Each record's byte position in student1.dat is stored, allowing efficient retrieval based on either ID or last name.
Role of cmdfile1.txt. The cmdfile1.txt file contains commands that the program processes sequentially. The five types of commands and their impact on the output file (outfile1.txt) are:
•	find ID <id>: Searches for a student record using the ID index (student1.ix1). If found, the program outputs the record; otherwise, it outputs "NOT FOUND".
•	find name <lastname>: Searches for all student records with a given last name using the name index (student1.ix2). If records exist, they are listed otherwise "NOT FOUND" is printed.
•	add <id> <lastname> <firstname> <year> <major> <email>: Inserts a new student record into student1.dat and updates both B-Trees. The output includes confirmation of the addition along with the byte position and splits that occurred.
•	dump by ID: Lists all student records sorted by ID. The program retrieves records using the ID-based B-Tree (student1.ix1).
•	dump by name: Lists all student records sorted by last name, retrieved using the name-based B-Tree (student1.ix2).
