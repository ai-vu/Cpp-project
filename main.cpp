#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>


// A program that keeps record of books that are in the library

using namespace std;

const string libraryFileName = "library.dat";
const string cardHolderFileName = "library_card_holders.dat";

const time_t borrowTime(60*60*24*14);

const int idLenth = 10;
const int dateStringLenth = 32;
const int maxStringLength = 25;
using libString = char[maxStringLength];

struct Book {
    libString name;
    int id;
    libString person;
    time_t endTime;
};

struct CardHolder {
    libString name;
};

class Library {
public:
    Library() {};
    void Initialize();
    void SaveToFile();
    void ReadFromFile();
    void AddBook();
    void BorrowBook();
    void ReturnBook();
    void Print();


private:
    bool trySetString(string from, libString& to);
    string formatTime(time_t t);
    tm getTimeInfo(time_t t);
    vector<Book> books;
    vector<CardHolder> cardHolders;
};

void Library::Initialize() {
    books.clear();
    cardHolders.clear();
}

void Library::SaveToFile()
{
    ofstream out(libraryFileName.c_str(), ios::binary);
    for_each(books.begin(), books.end(), [&](Book& book)
    {
        out.write(reinterpret_cast<char*>(&book), sizeof(Book));
    });

    out.close();

    ofstream out2(cardHolderFileName.c_str(), ios::binary);
    for_each(cardHolders.begin(), cardHolders.end(), [&](CardHolder& cardHolder)
    {
        out2.write(reinterpret_cast<char*>(&cardHolder), sizeof(CardHolder));
    });

    out2.close();
}

void Library::ReadFromFile()
{
    Initialize();

    ifstream in(libraryFileName.c_str(), ios::binary);

    while (true)
    {

        Book b;
        in.read(reinterpret_cast<char*>(&b), sizeof(Book));
        if (in) {
            books.push_back(b);
        }
        else {
            break;
        }

    }
    in.close();

    ifstream in2(cardHolderFileName.c_str(), ios::binary);

    while (true)
    {
        CardHolder c;
        in2.read(reinterpret_cast<char*>(&c), sizeof(CardHolder));
        if (in2) {
            cardHolders.push_back(c);
        }
        else {
            break;
        }

    }
    in2.close();
}

void Library::AddBook()
{
    Book book{};
    cout << "Add book" << endl;
    cout << "Name: " << endl;
    string val;
    cin.ignore();
    getline(cin, val);

    if (!trySetString(val, book.name)) {
        cout << "\nAdd book failed." << endl;
        return;
    }

    book.person[0] = '\0';
    book.id = static_cast<int>(books.size());
    book.endTime = 0;
    books.push_back(book);
}

void Library::BorrowBook()
{
    cout << "Borrow book" << endl;
    cout << "Give id of book:" << endl;

    int id = 0;
    while (!(cin >> id)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Error: Invalid input. Try again.";
    }

    auto it = find_if(books.begin(), books.end(), [id](Book& book)
    {
            return book.id == id && book.person[0] == 0;
    });

    if (it != books.end()) {
        cout << "Name of borrower: " << endl;
        string name;
        cin.ignore();
        getline(cin, name);

        if (!trySetString(name, it->person)) {
            cout << "Borrowing book failed." << endl;
            return;
        }

        time_t now(NULL);
        time(&now);
        it->endTime = now + borrowTime;


        // Find if this person is a cardholder already
        auto itCardHolder = find_if(cardHolders.begin(), cardHolders.end(), [it](CardHolder& c)
        {
             return c.name == it->person;
        });

        // If not add to list
        if (itCardHolder == cardHolders.end()) {

            CardHolder c;
            strcpy(c.name, it->person);
            cardHolders.push_back(c);
        }
    }
    else {
        cout << "Error: Could not find book. It might be borrowed already.";
        return;
    }
}

void Library::ReturnBook()
{
    cout << "Return book" << endl;
    cout << "Give id of book:" << endl;

    int id = 0;
    while (!(cin >> id)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Error: Invalid input. Try again.";
    }

    auto it = find_if(books.begin(), books.end(), [id](Book& book)
    {
        return book.id == id && book.person[0] != 0;
    });

    if (it != books.end()) {

        time_t now(NULL);
        time(&now);

        if (now > it->endTime) {
            cout << "You return the book to late." << endl;
        }

        it->person[0] = '\0';
        it->endTime = 0;
    }
    else {
        cout << "Error: Could not find book. It might not have been borrowed.";
        return;
    }
}

void Library::Print()
{
    // Print Borrowed books
    cout << "Borrowed books" << endl;
    cout <<
        setw(idLenth) << "ID" <<
        setw(maxStringLength) << "Book" <<
        setw(dateStringLenth) << "Return time " <<
        setw(maxStringLength) << "Borrower" <<
        endl;

    for_each(books.begin(), books.end(), [&](Book& book)
    {
        if (book.person[0] != 0) {
            cout <<
                setw(idLenth) << book.id <<
                setw(maxStringLength) << book.name <<
                setw(dateStringLenth) << formatTime(book.endTime) <<
                setw(maxStringLength) << book.person <<
                endl;
        }

    });

    // Print Available books
    cout << "Available books" << endl;
    cout <<
        setw(idLenth) << "ID" <<
        setw(maxStringLength) << "Book" <<
        endl;
    for_each(books.begin(), books.end(), [&](Book& book)
    {
        if (book.person[0] == 0) {
            cout <<
                setw(idLenth) << book.id <<
                setw(maxStringLength) << book.name <<
                endl;
        }

    });

    // Print card owners
    cout << "Library card owners" << endl;
    cout <<
        setw(maxStringLength) << "Name" <<
        endl;
    for_each(cardHolders.begin(), cardHolders.end(), [&](CardHolder& cardHolder)
    {

        cout <<
            setw(maxStringLength) << cardHolder.name <<
            endl;
    });
}

// Validate string and copy value to libstring
bool Library::trySetString(string from, libString& to)
{
    if (from.length() > maxStringLength) {
        cout << "Error: String too long.";
        return false;
    }

    if (from.length() == 0) {
        cout << "Error: String is empty.";
        return false;
    }

    from.copy(to, maxStringLength);
    return true;
}

string Library::formatTime(time_t t)
{
    if (t == 0) {
        return " ";
    }
    const auto timeInfo = getTimeInfo(t);
    char buffer[dateStringLenth];
    strftime(buffer, dateStringLenth, "%a, %d.%m.%Y %H:%M:%S", &timeInfo);
    return buffer;
}

tm Library::getTimeInfo(time_t t)
{
    tm bt{};
#if defined(__unix__)
    localtime_r(&t, &bt);
#elif defined(_MSC_VER)
    localtime_s(&bt, &t);
#else
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bt = *std::localtime(&t);
#endif
    return bt;
}

int main() {

    auto lib = Library();

    while (true) {
        cout << "\nLibrary" << endl;
        cout << "1) Save Books to File " << endl;
        cout << "2) Read Books from File " << endl;
        cout << "3) Add Book to Library " << endl;
        cout << "4) Borrow Book " << endl;
        cout << "5) Return book " << endl;
        cout << "6) Print report " << endl;
        cout << "7) Initialize " << endl;
        cout << "8) Exit " << endl;

        int val = 0;
        while (!(cin >> val)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Invalid input. Try again.";
        }

        if (val == 1) {
            lib.SaveToFile();
        }
        else if (val == 2) {
            lib.ReadFromFile();
        }
        else if (val == 3) {
            lib.AddBook();
        }
        else if (val == 4) {
            lib.BorrowBook();
        }
        else if (val == 5) {
            lib.ReturnBook();
        }
        else if (val == 6) {
            lib.Print();
        }
        else if (val == 7) {
            lib.Initialize();
        }
        else if (val == 8) {
            break;
        }
    }

    return 0;
}

