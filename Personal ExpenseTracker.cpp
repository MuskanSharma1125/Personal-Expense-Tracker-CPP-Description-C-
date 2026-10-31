#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <map>
#include <limits>
#include <algorithm>

using namespace std;

class Expense {
private:
    int id;
    string date;
    string category;
    string description;
    double amount;

public:
    Expense() : id(0), amount(0.0) {}

    Expense(int expenseId, const string& expenseDate,
            const string& expenseCategory, const string& expenseDescription,
            double expenseAmount)
        : id(expenseId), date(expenseDate), category(expenseCategory),
          description(expenseDescription), amount(expenseAmount) {}

    int getId() const { return id; }
    string getDate() const { return date; }
    string getCategory() const { return category; }
    string getDescription() const { return description; }
    double getAmount() const { return amount; }

    void display() const {
        cout << left << setw(6) << id
             << setw(14) << date
             << setw(16) << category
             << setw(28) << description
             << right << fixed << setprecision(2) << amount << '\n';
    }

    string toFileLine() const {
        ostringstream out;
        out << id << '|' << date << '|' << category << '|'
            << description << '|' << fixed << setprecision(2) << amount;
        return out.str();
    }

    static bool fromFileLine(const string& line, Expense& result) {
        stringstream ss(line);
        string idText, date, category, description, amountText;

        if (!getline(ss, idText, '|') ||
            !getline(ss, date, '|') ||
            !getline(ss, category, '|') ||
            !getline(ss, description, '|') ||
            !getline(ss, amountText)) {
            return false;
        }

        try {
            int id = stoi(idText);
            double amount = stod(amountText);
            result = Expense(id, date, category, description, amount);
            return true;
        } catch (...) {
            return false;
        }
    }
};

class ExpenseTracker {
private:
    vector<Expense> expenses;
    const string expenseFile = "expenses.txt";
    const string budgetFile = "budget.txt";
    double monthlyBudget = 0.0;
    int nextId = 1;

    static string getLineInput(const string& prompt) {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    static double getPositiveAmount(const string& prompt) {
        while (true) {
            cout << prompt;
            string input;
            getline(cin, input);

            try {
                size_t pos = 0;
                double value = stod(input, &pos);
                if (pos == input.size() && value > 0.0) {
                    return value;
                }
            } catch (...) {
                // Ask again when the input is not a valid number.
            }
            cout << "Please enter a valid amount greater than zero.\n";
        }
    }

    static int getMenuChoice() {
        while (true) {
            cout << "\nEnter your choice: ";
            string input;
            getline(cin, input);

            try {
                size_t pos = 0;
                int choice = stoi(input, &pos);
                if (pos == input.size()) return choice;
            } catch (...) {
                // Invalid input; show the message below.
            }
            cout << "Invalid input. Please enter a number.\n";
        }
    }

    static bool containsSeparator(const string& value) {
        return value.find('|') != string::npos;
    }

    void loadExpenses() {
        ifstream file(expenseFile);
        string line;
        int largestId = 0;

        while (getline(file, line)) {
            Expense expense;
            if (Expense::fromFileLine(line, expense)) {
                expenses.push_back(expense);
                largestId = max(largestId, expense.getId());
            }
        }
        nextId = largestId + 1;
    }

    void saveExpenses() const {
        ofstream file(expenseFile);
        if (!file) {
            cout << "Error: Could not save expense records.\n";
            return;
        }

        for (const Expense& expense : expenses) {
            file << expense.toFileLine() << '\n';
        }
    }

    void loadBudget() {
        ifstream file(budgetFile);
        if (file) file >> monthlyBudget;
    }

    void saveBudget() const {
        ofstream file(budgetFile);
        if (file) file << fixed << setprecision(2) << monthlyBudget;
    }

    static bool isValidDateFormat(const string& date) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
        for (int i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) continue;
            if (!isdigit(static_cast<unsigned char>(date[i]))) return false;
        }
        int month = stoi(date.substr(5, 2));
        int day = stoi(date.substr(8, 2));
        return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }

    static string chooseCategory() {
        const vector<string> categories = {
            "Food", "Travel", "Shopping", "Education",
            "Bills", "Health", "Entertainment", "Other"
        };

        cout << "\nExpense Categories\n";
        for (size_t i = 0; i < categories.size(); ++i) {
            cout << i + 1 << ". " << categories[i] << '\n';
        }

        while (true) {
            cout << "Choose category (1-" << categories.size() << "): ";
            string input;
            getline(cin, input);
            try {
                size_t pos = 0;
                int choice = stoi(input, &pos);
                if (pos == input.size() && choice >= 1 &&
                    choice <= static_cast<int>(categories.size())) {
                    return categories[choice - 1];
                }
            } catch (...) {}
            cout << "Invalid category. Try again.\n";
        }
    }

    static void printTableHeading() {
        cout << '\n' << left << setw(6) << "ID"
             << setw(14) << "Date"
             << setw(16) << "Category"
             << setw(28) << "Description"
             << "Amount (Rs.)\n";
        cout << string(78, '-') << '\n';
    }

    double totalForMonth(const string& month) const {
        double total = 0.0;
        for (const Expense& expense : expenses) {
            if (expense.getDate().size() >= 7 &&
                expense.getDate().substr(0, 7) == month) {
                total += expense.getAmount();
            }
        }
        return total;
    }

public:
    ExpenseTracker() {
        loadExpenses();
        loadBudget();
    }

    void addExpense() {
        cout << "\n--- Add New Expense ---\n";

        string date;
        while (true) {
            date = getLineInput("Enter date (YYYY-MM-DD): ");
            if (isValidDateFormat(date)) break;
            cout << "Please use a valid date format, for example 2026-10-30.\n";
        }

        string category = chooseCategory();

        string description;
        while (true) {
            description = getLineInput("Enter description: ");
            if (!description.empty() && !containsSeparator(description)) break;
            cout << "Description cannot be empty or contain the | symbol.\n";
        }

        double amount = getPositiveAmount("Enter amount (Rs.): ");

        expenses.emplace_back(nextId++, date, category, description, amount);
        saveExpenses();

        cout << "\nExpense added successfully!\n";
        checkBudgetWarning(date.substr(0, 7));
    }

    void viewExpenses() const {
        cout << "\n--- All Expense Records ---\n";
        if (expenses.empty()) {
            cout << "No expenses found. Add your first expense!\n";
            return;
        }

        printTableHeading();
        for (const Expense& expense : expenses) expense.display();
    }

    void searchExpense() const {
        cout << "\n--- Search Expenses ---\n";
        cout << "1. Search by date (YYYY-MM-DD)\n";
        cout << "2. Search by category\n";
        cout << "3. Search by description\n";
        int choice = getMenuChoice();

        string query;
        if (choice == 1) query = getLineInput("Enter date: ");
        else if (choice == 2) query = getLineInput("Enter category: ");
        else if (choice == 3) query = getLineInput("Enter description keyword: ");
        else {
            cout << "Invalid search option.\n";
            return;
        }

        bool found = false;
        printTableHeading();
        for (const Expense& expense : expenses) {
            bool match = false;

            if (choice == 1) match = expense.getDate() == query;
            if (choice == 2) {
                string category = expense.getCategory();
                string search = query;
                transform(category.begin(), category.end(), category.begin(), ::tolower);
                transform(search.begin(), search.end(), search.begin(), ::tolower);
                match = category == search;
            }
            if (choice == 3) {
                string description = expense.getDescription();
                string search = query;
                transform(description.begin(), description.end(), description.begin(), ::tolower);
                transform(search.begin(), search.end(), search.begin(), ::tolower);
                match = description.find(search) != string::npos;
            }

            if (match) {
                expense.display();
                found = true;
            }
        }

        if (!found) cout << "No matching expenses found.\n";
    }

    void categorySummary() const {
        cout << "\n--- Category-wise Expense Summary ---\n";
        map<string, double> totals;

        for (const Expense& expense : expenses) {
            totals[expense.getCategory()] += expense.getAmount();
        }

        if (totals.empty()) {
            cout << "No expenses available.\n";
            return;
        }

        cout << left << setw(20) << "Category" << "Total (Rs.)\n";
        cout << string(38, '-') << '\n';
        for (const auto& item : totals) {
            cout << left << setw(20) << item.first
                 << right << fixed << setprecision(2) << item.second << '\n';
        }
    }

    void monthlySummary() const {
        cout << "\n--- Monthly Expense Summary ---\n";
        string month;

        while (true) {
            month = getLineInput("Enter month (YYYY-MM): ");
            if (month.size() == 7 && month[4] == '-') {
                bool valid = true;
                for (int i = 0; i < 7; ++i) {
                    if (i == 4) continue;
                    if (!isdigit(static_cast<unsigned char>(month[i]))) valid = false;
                }
                int m = valid ? stoi(month.substr(5, 2)) : 0;
                if (valid && m >= 1 && m <= 12) break;
            }
            cout << "Please enter the month in YYYY-MM format, e.g. 2026-10.\n";
        }

        double total = totalForMonth(month);
        cout << "\nTotal spending for " << month << ": Rs. "
             << fixed << setprecision(2) << total << '\n';

        if (monthlyBudget > 0.0) {
            cout << "Monthly budget: Rs. " << monthlyBudget << '\n';
            cout << "Remaining budget: Rs. " << monthlyBudget - total << '\n';
            if (total > monthlyBudget) {
                cout << "Warning: You have exceeded your monthly budget!\n";
            } else {
                cout << "You are within your monthly budget.\n";
            }
        } else {
            cout << "No monthly budget has been set yet.\n";
        }
    }

    void setBudget() {
        cout << "\n--- Set Monthly Budget ---\n";
        monthlyBudget = getPositiveAmount("Enter your monthly budget (Rs.): ");
        saveBudget();
        cout << "Monthly budget saved successfully.\n";
    }

    void deleteExpense() {
        cout << "\n--- Delete Expense ---\n";
        if (expenses.empty()) {
            cout << "There are no records to delete.\n";
            return;
        }

        viewExpenses();
        int id;
        while (true) {
            cout << "\nEnter the ID of the expense to delete: ";
            string input;
            getline(cin, input);
            try {
                size_t pos = 0;
                id = stoi(input, &pos);
                if (pos == input.size()) break;
            } catch (...) {}
            cout << "Please enter a valid ID.\n";
        }

        auto it = find_if(expenses.begin(), expenses.end(),
                          [id](const Expense& expense) {
                              return expense.getId() == id;
                          });

        if (it == expenses.end()) {
            cout << "Expense ID not found.\n";
            return;
        }

        expenses.erase(it);
        saveExpenses();
        cout << "Expense deleted successfully.\n";
    }

    void checkBudgetWarning(const string& month) const {
        if (monthlyBudget <= 0.0) return;

        double total = totalForMonth(month);
        if (total > monthlyBudget) {
            cout << "\n*** Budget Alert! ***\n";
            cout << "Your spending for " << month << " is Rs. "
                 << fixed << setprecision(2) << total
                 << ", which is above your budget of Rs. "
                 << monthlyBudget << ".\n";
        } else {
            cout << "Remaining budget for " << month << ": Rs. "
                 << fixed << setprecision(2) << monthlyBudget - total << '\n';
        }
    }

    void run() {
        cout << "============================================\n";
        cout << "       PERSONAL EXPENSE TRACKER (C++)\n";
        cout << "============================================\n";

        while (true) {
            cout << "\n--------------- MAIN MENU -----------------\n";
            cout << "1. Add New Expense\n";
            cout << "2. View All Expenses\n";
            cout << "3. Search Expense\n";
            cout << "4. Category-wise Summary\n";
            cout << "5. Monthly Expense Summary\n";
            cout << "6. Set Monthly Budget\n";
            cout << "7. Delete Expense\n";
            cout << "8. Exit\n";
            cout << "--------------------------------------------\n";

            int choice = getMenuChoice();

            switch (choice) {
                case 1: addExpense(); break;
                case 2: viewExpenses(); break;
                case 3: searchExpense(); break;
                case 4: categorySummary(); break;
                case 5: monthlySummary(); break;
                case 6: setBudget(); break;
                case 7: deleteExpense(); break;
                case 8:
                    saveExpenses();
                    saveBudget();
                    cout << "\nThank you for using Personal Expense Tracker!\n";
                    cout << "Your records have been saved.\n";
                    return;
                default:
                    cout << "Invalid choice. Please select from 1 to 8.\n";
            }
        }
    }
};

int main() {
    ExpenseTracker tracker;
    tracker.run();
    return 0;
}
