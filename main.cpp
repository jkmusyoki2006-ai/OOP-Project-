#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;

// ============================================================
//  UTILITIES
// ============================================================
string currentTime() {
    time_t t = time(nullptr);
    tm* tm   = localtime(&t);
    ostringstream o;
    o << setw(2)<<setfill('0')<<tm->tm_hour<<":"
      << setw(2)<<setfill('0')<<tm->tm_min <<":"
      << setw(2)<<setfill('0')<<tm->tm_sec;
    return o.str();
}
void line(char c = '-', int n = 52) { cout << string(n, c) << "\n"; }
void header(const string& s) { line('='); cout << "  " << s << "\n"; line('='); }

// Ticket generator -- encapsulates its own counters (Encapsulation)
map<string, int> _ticketCounters;
string genTicket(const string& prefix) {
    ostringstream o;
    o << prefix << setw(3) << setfill('0') << ++_ticketCounters[prefix];
    return o.str();
}

// ============================================================
//  INTERFACES  (Abstraction)
// ============================================================
class Notifiable {
public:
    virtual void sendNotification(const string& msg) = 0;
    virtual ~Notifiable() {}
};

class Reportable {
public:
    virtual void generateReport() const = 0;
    virtual ~Reportable() {}
};

// ============================================================
//  PERSON  --  Abstract Base Class  (Abstraction + Inheritance)
// ============================================================
class Person : public Notifiable {
protected:
    string name, id, phone;
public:
    Person(string n, string i, string p) : name(n), id(i), phone(p) {}
    virtual ~Person() {}
    string getName()  const { return name; }
    string getID()    const { return id;   }
    string getPhone() const { return phone;}
    virtual void displayInfo() const = 0;  // pure virtual
    void sendNotification(const string& msg) override {
        cout << "  [NOTIFICATION -> " << name << "]: " << msg << "\n";
    }
};

// ============================================================
//  CUSTOMER  (Inherits Person)
// ============================================================
class Customer : public Person {
    bool   priority;   // elderly / disabled
    string accountNo;
public:
    Customer(string n, string i, string p, bool pr = false, string acc = "")
        : Person(n, i, p), priority(pr), accountNo(acc) {}
    bool   isPriority()   const { return priority; }
    string getAccountNo() const { return accountNo; }
    void displayInfo() const override {
        cout << "  Name    : " << name    << "\n"
             << "  ID      : " << id      << "\n"
             << "  Phone   : " << phone   << "\n"
             << "  Account : " << (accountNo.empty() ? "N/A" : accountNo) << "\n"
             << "  Priority: " << (priority ? "YES -- Priority Customer" : "No") << "\n";
    }
};

// ============================================================
//  SERVICE REQUEST  --  Abstract  (Abstraction + Polymorphism)
// ============================================================
enum class Status { WAITING, SERVING, COMPLETED };

class ServiceRequest {
protected:
    string ticket, customerName, issuedAt;
    Status status;
    int    estMinutes;
public:
    ServiceRequest(string t, string c, int m)
        : ticket(t), customerName(c), issuedAt(currentTime()),
          status(Status::WAITING), estMinutes(m) {}
    virtual ~ServiceRequest() {}

    string getTicket()   const { return ticket; }
    string getCustomer() const { return customerName; }
    int    getEst()      const { return estMinutes; }
    void   setStatus(Status s) { status = s; }
    Status getStatus()   const { return status; }

    string statusStr() const {
        if(status==Status::WAITING)   return "Waiting";
        if(status==Status::SERVING)   return "Serving";
        return "Completed";
    }

    virtual string serviceType() const = 0;  // pure virtual
    virtual void   process()           = 0;  // pure virtual -- POLYMORPHISM

    void showTicket() const {
        line();
        cout << "  Ticket No : " << ticket        << "\n"
             << "  Customer  : " << customerName  << "\n"
             << "  Service   : " << serviceType() << "\n"
             << "  Issued At : " << issuedAt      << "\n"
             << "  Est. Wait : " << estMinutes     << " min\n"
             << "  Status    : " << statusStr()    << "\n";
        line();
    }
};

// -- Bank Request Subclasses (Inheritance + Polymorphism) ------

class WithdrawalRequest : public ServiceRequest {
    double amount;
public:
    WithdrawalRequest(string t, string c, double a)
        : ServiceRequest(t, c, 5), amount(a) {}
    string serviceType() const override { return "Withdrawal"; }
    void process() override {
        cout << "  Processing withdrawal of KES " << fixed << setprecision(2)
             << amount << " for " << customerName << "...\n";
        status = Status::COMPLETED;
    }
};

class DepositRequest : public ServiceRequest {
    double amount;
public:
    DepositRequest(string t, string c, double a)
        : ServiceRequest(t, c, 4), amount(a) {}
    string serviceType() const override { return "Deposit"; }
    void process() override {
        cout << "  Processing deposit of KES " << fixed << setprecision(2)
             << amount << " for " << customerName << "...\n";
        status = Status::COMPLETED;
    }
};

class LoanInquiryRequest : public ServiceRequest {
public:
    LoanInquiryRequest(string t, string c) : ServiceRequest(t, c, 15) {}
    string serviceType() const override { return "Loan Inquiry"; }
    void process() override {
        cout << "  Conducting loan inquiry session for " << customerName << "...\n";
        status = Status::COMPLETED;
    }
};

class AccountOpeningRequest : public ServiceRequest {
public:
    AccountOpeningRequest(string t, string c) : ServiceRequest(t, c, 20) {}
    string serviceType() const override { return "Account Opening"; }
    void process() override {
        cout << "  Opening new bank account for " << customerName << "...\n";
        status = Status::COMPLETED;
    }
};

class MiniStatementRequest : public ServiceRequest {
public:
    MiniStatementRequest(string t, string c) : ServiceRequest(t, c, 3) {}
    string serviceType() const override { return "Mini Statement"; }
    void process() override {
        cout << "  Printing mini statement for " << customerName << "...\n";
        status = Status::COMPLETED;
    }
};

// ============================================================
//  QUEUE  --  Abstract Base  (Abstraction)
// ============================================================
class Queue {
protected:
    vector<ServiceRequest*> requests;
    int totalServed = 0, totalWaitMins = 0;
public:
    virtual ~Queue() {}
    virtual void push(ServiceRequest* r) = 0;

    ServiceRequest* pop() {
        if (requests.empty()) return nullptr;
        ServiceRequest* r = requests.front();
        requests.erase(requests.begin());
        totalServed++;
        totalWaitMins += r->getEst();
        return r;
    }
    bool   empty()    const { return requests.empty(); }
    int    size()     const { return (int)requests.size(); }
    int    getServed()const { return totalServed; }
    double avgWait()  const { return totalServed ? (double)totalWaitMins / totalServed : 0; }

    void display() const {
        if (requests.empty()) { cout << "  Queue is currently empty.\n"; return; }
        for (int i = 0; i < (int)requests.size(); i++)
            cout << "  " << i+1 << ". [" << requests[i]->getTicket() << "]  "
                 << requests[i]->getCustomer() << "  --  "
                 << requests[i]->serviceType() << "\n";
    }
};

// Standard FIFO Queue
class StandardQueue : public Queue {
public:
    void push(ServiceRequest* r) override { requests.push_back(r); }
};

// Priority Queue -- priority tickets inserted before standard ones
class PriorityQueue : public Queue {
public:
    void push(ServiceRequest* r) override {
        bool isPrio = (r->getTicket()[0] == 'P');
        auto it = requests.begin();
        if (isPrio)
            while (it != requests.end() && (*it)->getTicket()[0] == 'P') ++it;
        requests.insert(it, r);
    }
};

// ============================================================
//  STAFF  (Inherits Person, implements Reportable)
// ============================================================
class Staff : public Person, public Reportable {
protected:
    string role;
    int    servedToday = 0;
    bool   onBreak     = false;
public:
    Staff(string n, string i, string p, string r)
        : Person(n, i, p), role(r) {}
    string getRole()      const { return role; }
    bool   isOnBreak()    const { return onBreak; }
    void   setBreak(bool b)     { onBreak = b; }
    void   incServed()          { servedToday++; }
    int    getServedToday()const{ return servedToday; }

    void displayInfo() const override {
        cout << "  Name   : " << name   << "\n"
             << "  ID     : " << id     << "\n"
             << "  Role   : " << role   << "\n"
             << "  Status : " << (onBreak ? "On Break" : "Available") << "\n"
             << "  Served : " << servedToday << " customers today\n";
    }
    void generateReport() const override {
        line();
        cout << "  Staff Report  --  " << name << " (" << role << ")\n"
             << "  Customers served today : " << servedToday << "\n"
             << "  Current status         : " << (onBreak?"On Break":"Available") << "\n";
        line();
    }
};

// Subclasses of Staff (Inheritance + Polymorphism on generateReport)
class Teller : public Staff {
public:
    Teller(string n, string i, string p) : Staff(n, i, p, "Teller") {}
    void generateReport() const override {
        Staff::generateReport();
        cout << "  Speciality: Cash transactions & account services\n";
        line();
    }
};

class Manager : public Staff {
public:
    Manager(string n, string i, string p) : Staff(n, i, p, "Manager") {}
    void generateReport() const override {
        Staff::generateReport();
        cout << "  Access: Full system oversight & queue override\n";
        line();
    }
};

// ============================================================
//  QUEUE MANAGER  (Encapsulation -- hides all internal queues)
// ============================================================
class QueueManager {
    PriorityQueue bankQueue;
    vector<Staff*> staffList;
    int totalToday = 0;
public:
    ~QueueManager() { for (auto s : staffList) delete s; }
    void addStaff(Staff* s)  { staffList.push_back(s); }

    void enqueue(ServiceRequest* r) {
        r->setStatus(Status::WAITING);
        bankQueue.push(r);
        totalToday++;
        cout << "\n  Ticket issued  : " << r->getTicket()
             << "\n  Estimated wait : " << (r->getEst() * bankQueue.size())
             << " min  |  Position: " << bankQueue.size() << "\n";
        r->showTicket();
    }

    void serveNext() {
        if (bankQueue.empty()) { cout << "  No customers in queue.\n"; return; }
        // Find an available teller
        Teller* teller = nullptr;
        for (auto s : staffList)
            if (s->getRole() == "Teller" && !s->isOnBreak())
                { teller = dynamic_cast<Teller*>(s); break; }
        if (!teller) { cout << "  No available teller at the moment.\n"; return; }

        ServiceRequest* r = bankQueue.pop();
        r->setStatus(Status::SERVING);
        cout << "\n  Teller " << teller->getName() << " is now serving:\n";
        r->showTicket();
        r->process();       // <-- POLYMORPHISM: correct process() runs per request type
        teller->incServed();
        teller->sendNotification("Customer " + r->getCustomer() + " served successfully.");
        delete r;
    }

    void showQueue()  const { cout << "\n  -- Current Queue --\n"; bankQueue.display(); }
    void showStaff()  const {
        header("STAFF LIST");
        for (auto s : staffList) { s->displayInfo(); line('-', 35); }
    }
    void toggleBreak(const string& staffID) {
        for (auto s : staffList) {
            if (s->getID() == staffID) {
                s->setBreak(!s->isOnBreak());
                cout << "  " << s->getName() << " is now "
                     << (s->isOnBreak() ? "ON BREAK" : "BACK & AVAILABLE") << "\n";
                return;
            }
        }
        cout << "  Staff ID not found.\n";
    }
    void systemReport() const {
        header("SYSTEM REPORT  --  " + currentTime());
        cout << "  Total customers today : " << totalToday     << "\n"
             << "  Currently in queue    : " << bankQueue.size()  << "\n"
             << "  Total served          : " << bankQueue.getServed() << "\n"
             << "  Average wait time     : " << fixed << setprecision(1)
             << bankQueue.avgWait() << " min\n\n";
        cout << "  -- Individual Staff Reports --\n";
        for (auto s : staffList) s->generateReport();  // POLYMORPHISM
    }
    void checkAlerts() {
        if (bankQueue.size() >= 5) {
            for (auto s : staffList)
                if (s->getRole() == "Manager")
                    s->sendNotification("Queue alert! " +
                        to_string(bankQueue.size()) + " customers waiting.");
        }
    }
};

// ============================================================
//  SETUP & MENUS
// ============================================================
QueueManager qm;

void setup() {
    qm.addStaff(new Teller("Alice Mwangi",  "T01", "0712-100001"));
    qm.addStaff(new Teller("Brian Ochieng", "T02", "0712-100002"));
    qm.addStaff(new Teller("Carol Wanjiru", "T03", "0712-100003"));
    qm.addStaff(new Manager("David Kamau",  "M01", "0712-100004"));
}

void customerCheckIn() {
    header("CUSTOMER CHECK-IN");
    string name, id, phone, accNo;
    char pr;
    cout << "  Full Name   : "; getline(cin, name);
    cout << "  National ID : "; getline(cin, id);
    cout << "  Phone       : "; getline(cin, phone);
    cout << "  Account No  : "; getline(cin, accNo);
    cout << "  Priority customer? (elderly/disabled) [y/n]: "; cin >> pr; cin.ignore();

    bool prio  = (pr == 'y' || pr == 'Y');
    string pfx = prio ? "P" : "B";

    Customer cust(name, id, phone, prio, accNo);
    cout << "\n  Select Bank Service:\n"
         << "  1. Withdrawal\n"
         << "  2. Deposit\n"
         << "  3. Loan Inquiry\n"
         << "  4. Account Opening\n"
         << "  5. Mini Statement\n"
         << "  Choice: ";
    int c; cin >> c; cin.ignore();

    string ticket = genTicket(pfx);
    ServiceRequest* req = nullptr;
    switch(c) {
        case 1: { double a; cout<<"  Amount (KES): "; cin>>a; cin.ignore();
                  req = new WithdrawalRequest(ticket, name, a); break; }
        case 2: { double a; cout<<"  Amount (KES): "; cin>>a; cin.ignore();
                  req = new DepositRequest(ticket, name, a); break; }
        case 3:   req = new LoanInquiryRequest(ticket, name);   break;
        case 4:   req = new AccountOpeningRequest(ticket, name); break;
        default:  req = new MiniStatementRequest(ticket, name);  break;
    }
    qm.enqueue(req);
    cust.sendNotification("Your ticket " + ticket + " has been issued. Please wait to be called.");
    qm.checkAlerts();
}

void staffPortal() {
    header("STAFF PORTAL");
    cout << "  1. Serve Next Customer\n"
         << "  2. View Queue\n"
         << "  3. Toggle Break (by Staff ID)\n"
         << "  4. Back\n"
         << "  Choice: ";
    int c; cin >> c; cin.ignore();
    switch(c) {
        case 1: qm.serveNext(); break;
        case 2: qm.showQueue(); break;
        case 3: { string sid; cout<<"  Enter Staff ID: "; getline(cin,sid);
                  qm.toggleBreak(sid); break; }
        default: break;
    }
}

void managerPortal() {
    header("MANAGER DASHBOARD");
    cout << "  1. View Queue\n"
         << "  2. View All Staff\n"
         << "  3. System Report\n"
         << "  4. Back\n"
         << "  Choice: ";
    int c; cin >> c; cin.ignore();
    switch(c) {
        case 1: qm.showQueue();    break;
        case 2: qm.showStaff();    break;
        case 3: qm.systemReport(); break;
        default: break;
    }
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    setup();
    header("BANK QUEUE MANAGEMENT SYSTEM");
    cout << "  Welcome!  System Time: " << currentTime() << "\n";

    for (bool run = true; run;) {
        cout << "\n  MAIN MENU\n"; line('-', 30);
        cout << "  1. Customer Check-In\n"
             << "  2. Staff Portal\n"
             << "  3. Manager Dashboard\n"
             << "  4. Exit\n"
             << "  Choice: ";
        int choice; cin >> choice; cin.ignore();
        cout << "\n";
        switch(choice) {
            case 1: customerCheckIn(); break;
            case 2: staffPortal();     break;
            case 3: managerPortal();   break;
            default:
                cout << "  System closed at " << currentTime() << ". Goodbye!\n";
                run = false;
        }
    }
    return 0;
}
