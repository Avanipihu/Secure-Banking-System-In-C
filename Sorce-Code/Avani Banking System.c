#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- STRUCTURES (CO4) ---
typedef struct {
    int acc_no;
    char name[50];
    int pin;
    float balance;
} Account;

typedef struct {
    int acc_no;
    char type[15];
    float amount;
    float balance_after;
    char timestamp[30];
} Transaction;

// --- FUNCTION PROTOTYPES ---
void loadAccounts(Account **acc, int *count);
void saveAccounts(Account *acc, int count);
void createAccount(Account **acc, int *count);
Account* login(Account *acc, int count);
void deposit(Account *curr);
void withdraw(Account *curr);
void recordTransaction(int acc_no, char type[], float amount, float balance);
void viewTransactions(int acc_no);
void getTimestamp(char *buffer);

// --- MAIN SYSTEM FLOW ---
int main() {
    Account *accounts = NULL;
    int accountCount = 0;
    int choice;

    loadAccounts(&accounts, &accountCount);

    while (1) {
        printf("\n=== SECURE BANKING SYSTEM ===");
        printf("\n1. Create Account\n2. Login\n3. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1:
                createAccount(&accounts, &accountCount);
                saveAccounts(accounts, accountCount);
                break;
            case 2: {
                Account *loggedInUser = login(accounts, accountCount);
                if (loggedInUser != NULL) {
                    int subChoice;
                    do {
                        printf("\n--- Welcome, %s ---", loggedInUser->name);
                        printf("\n1. Deposit\n2. Withdraw\n3. Check Balance\n4. Transaction History\n5. Logout\nChoice: ");
                        scanf("%d", &subChoice);

                        if (subChoice == 1) deposit(loggedInUser);
                        else if (subChoice == 2) withdraw(loggedInUser);
                        else if (subChoice == 3) printf("\nCurrent Balance: $%.2f\n", loggedInUser->balance);
                        else if (subChoice == 4) viewTransactions(loggedInUser->acc_no);
                        
                        if (subChoice >= 1 && subChoice <= 2) saveAccounts(accounts, accountCount);
                    } while (subChoice != 5);
                }
                break;
            }
            case 3:
                printf("Saving data and exiting...\n");
                saveAccounts(accounts, accountCount);
                free(accounts);
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}

// --- FILE HANDLING (CO4/CO2) ---
void loadAccounts(Account **acc, int *count) {
    FILE *fp = fopen("accounts.dat", "rb");
    if (fp == NULL) return;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    *count = size / sizeof(Account);
    if (*count > 0) {
        *acc = (Account*)malloc(size);
        fread(*acc, sizeof(Account), *count, fp);
    }
    fclose(fp);
}

void saveAccounts(Account *acc, int count) {
    FILE *fp = fopen("accounts.dat", "wb");
    if (fp != NULL) {
        fwrite(acc, sizeof(Account), count, fp);
        fclose(fp);
    }
}

// --- ACCOUNT OPERATIONS (CO3/CO4) ---
void createAccount(Account **acc, int *count) {
    *acc = (Account*)realloc(*acc, (*count + 1) * sizeof(Account));
    Account *newAcc = &((*acc)[*count]);

    newAcc->acc_no = 1000 + (*count + 1); // Auto-increment logic
    printf("Enter Name: ");
    scanf(" %[^\n]s", newAcc->name);
    printf("Set 4-Digit PIN: ");
    scanf("%d", &newAcc->pin);
    
    float initialDep;
    do {
        printf("Initial Deposit (Min $500): ");
        scanf("%f", &initialDep);
    } while (initialDep < 500);

    newAcc->balance = initialDep;
    (*count)++;
    
    printf("\nAccount Created Successfully! Your Acc No: %d\n", newAcc->acc_no);
    recordTransaction(newAcc->acc_no, "INITIAL", initialDep, newAcc->balance);
}

// --- SECURITY LOGIC (CO1/CO2) ---
Account* login(Account *acc, int count) {
    int accNo, pin, attempts = 0;
    while (attempts < 3) {
        printf("\nEnter Account Number: ");
        scanf("%d", &accNo);
        printf("Enter PIN: ");
        scanf("%d", &pin);

        for (int i = 0; i < count; i++) {
            if (acc[i].acc_no == accNo && acc[i].pin == pin) {
                printf("Login Successful!\n");
                return &acc[i]; // Return pointer to original record
            }
        }
        attempts++;
        printf("Invalid credentials. Attempts left: %d\n", 3 - attempts);
    }
    printf("Security Alert: Multiple failed attempts.\n");
    return NULL;
}

// --- BANKING LOGIC ---
void deposit(Account *curr) {
    float amt;
    printf("Enter amount to deposit: ");
    scanf("%f", &amt);
    if (amt > 0) {
        curr->balance += amt;
        recordTransaction(curr->acc_no, "DEPOSIT", amt, curr->balance);
        printf("Deposit Successful.\n");
    } else {
        printf("Invalid amount.\n");
    }
}

void withdraw(Account *curr) {
    float amt;
    printf("Enter amount to withdraw: ");
    scanf("%f", &amt);
    if (amt > 0 && amt <= curr->balance) {
        curr->balance -= amt;
        recordTransaction(curr->acc_no, "WITHDRAW", amt, curr->balance);
        printf("Withdrawal Successful.\n");
    } else {
        printf("Insufficient balance or invalid amount.\n");
    }
}

// --- AUDIT TRAIL (FILE APPENDING) ---
void recordTransaction(int acc_no, char type[], float amount, float balance) {
    FILE *fp = fopen("transactions.dat", "ab");
    if (fp == NULL) return;

    Transaction t;
    t.acc_no = acc_no;
    strcpy(t.type, type);
    t.amount = amount;
    t.balance_after = balance;
    getTimestamp(t.timestamp);

    fwrite(&t, sizeof(Transaction), 1, fp);
    fclose(fp);
}

void viewTransactions(int acc_no) {
    FILE *fp = fopen("transactions.dat", "rb");
    if (fp == NULL) {
        printf("No transaction history found.\n");
        return;
    }

    Transaction t;
    printf("\n--- TRANSACTION HISTORY (%d) ---\n", acc_no);
    printf("%-20s | %-10s | %-10s | %-10s\n", "Timestamp", "Type", "Amount", "Balance");
    
    int found = 0;
    while (fread(&t, sizeof(Transaction), 1, fp)) {
        if (t.acc_no == acc_no) {
            printf("%-20s | %-10s | %-10.2f | %-10.2f\n", t.timestamp, t.type, t.amount, t.balance_after);
            found = 1;
        }
    }
    if (!found) printf("No records found for this account.\n");
    fclose(fp);
}

void getTimestamp(char *buffer) {
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", info);
}



