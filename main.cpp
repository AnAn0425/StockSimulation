#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <thread>   
#include <chrono>   
#include <mutex>    
#include <conio.h>  

using namespace std;

mutex mtx;
bool isRunning = true;

// ==========================================
// 類別繼承功能 (Inheritance)
// ==========================================
class FinancialProduct {
protected:
    string code;
    string name;
    double currentPrice;
    vector<double> priceHistory;

public:
    FinancialProduct(string c, string n, double p) : code(c), name(n), currentPrice(p) {
        priceHistory.push_back(p);
    }
    virtual ~FinancialProduct() {}

    string getCode() const { return code; }
    string getName() const { return name; }
    double getCurrentPrice() const { return currentPrice; }
    const vector<double>& getPriceHistory() const { return priceHistory; }

    virtual void updatePrice() = 0;
    virtual string getProductType() const = 0;
};

// 普通股票
class Stock : public FinancialProduct {
public:
    Stock(string c, string n, double p) : FinancialProduct(c, n, p) {}
    void updatePrice() override {
        // 每秒跳動範圍約 -1.5% ~ +1.5%
        double changePercent = ((rand() % 300) - 150) / 10000.0;
        currentPrice = currentPrice * (1.0 + changePercent);
        if (currentPrice < 1.0) currentPrice = 1.0;
        priceHistory.push_back(currentPrice);
    }
    string getProductType() const override { return "普通股票"; }
};

// ==========================================
// 交易所管理系統
// ==========================================
class StockExchange {
private:
    vector<unique_ptr<FinancialProduct>> marketProducts;
    map<string, int> userPortfolio;
    double userBalance;
    int currentDay;
    const string filename = "stock_saveData.txt";

public:
    // 初始資金：5000 萬
    StockExchange() : userBalance(50000000.0), currentDay(1) {
        srand(time(0));
    }

    // 確實修正：初始化你指定的三大高價股
    void initMarket() {
        marketProducts.clear();
        marketProducts.push_back(make_unique<Stock>("2330", "台積電", 2400.0));
        marketProducts.push_back(make_unique<Stock>("2383", "台光電", 5600.0));
        marketProducts.push_back(make_unique<Stock>("2308", "台達電", 2150.0));
    }

    void updateMarketPricesPeriodically() {
        while (isRunning) {
            this_thread::sleep_for(chrono::milliseconds(1500));
            {
                lock_guard<mutex> lock(mtx);
                for (auto& prod : marketProducts) {
                    prod->updatePrice();
                }
                system("cls");
                showMarket();
            }
        }
    }

    void showMarket() const {
        cout << "======================= 終端機股市觀測站 (5000萬資金大戶操作中) =======================\n";
        cout << left << setw(10) << "商品代號" << setw(16) << "商品名稱" << setw(16) << "類別" << setw(14) << "目前價格" << "今日走勢\n";
        cout << "--------------------------------------------------------------------------------\n";
        for (const auto& prod : marketProducts) {
            double lastPrice = (prod->getPriceHistory().size() > 1) ? prod->getPriceHistory()[prod->getPriceHistory().size() - 2] : prod->getCurrentPrice();
            double changePercent = ((prod->getCurrentPrice() - lastPrice) / lastPrice) * 100.0;

            cout << left << setw(12) << prod->getCode()
                << setw(14) << prod->getName()
                << setw(16) << prod->getProductType()
                << "$" << fixed << setprecision(2) << setw(12) << prod->getCurrentPrice();

            if (changePercent >= 0) {
                cout << "+" << setprecision(2) << changePercent << "% ▲\n";
            }
            else {
                cout << setprecision(2) << changePercent << "% ▼\n";
            }
        }
        cout << "--------------------------------------------------------------------------------\n";
        cout << "[我的帳戶資產] 可用現金餘額: $" << fixed << setprecision(1) << userBalance << "\n";
        cout << "[目前持股庫存]: ";
        bool hasStock = false;
        for (const auto& pair : userPortfolio) {
            if (pair.second > 0) {
                cout << pair.first << ": " << pair.second << " 股 | ";
                hasStock = true;
            }
        }
        if (!hasStock) cout << "暫無持股";
        cout << "\n================================================================================\n";
        cout << "操作選單: [B] 買進股票 | [S] 賣出股票 | [Q] 儲存並離開系統\n";
        cout << "請直接在鍵盤按下對應英文按鈕開始下單...";
        cout.flush();
    }

    void buyStock() {
        lock_guard<mutex> lock(mtx);
        cout << "\n\n>>> 進入買進模式 <<<\n";
        string code;
        int shares;
        cout << "請輸入商品代號: "; cin >> code;

        FinancialProduct* prod = nullptr;
        for (const auto& p : marketProducts) {
            if (p->getCode() == code) prod = p.get();
        }
        if (!prod) { cout << "[錯誤] 找不到此代號！\n"; system("pause"); return; }

        cout << "目前價格: $" << prod->getCurrentPrice() << "，欲買進股數: "; cin >> shares;
        if (shares <= 0) { cout << "[錯誤] 股數無效！\n"; system("pause"); return; }

        double totalCost = prod->getCurrentPrice() * shares;
        if (userBalance >= totalCost) {
            userBalance -= totalCost;
            userPortfolio[code] += shares;
            cout << "[成功] 買進成功！本次花費 $" << totalCost << "\n";
        }
        else {
            cout << "[失敗] 現金餘額不足！\n";
        }
        system("pause");
    }

    void sellStock() {
        lock_guard<mutex> lock(mtx);
        cout << "\n\n>>> 進入賣出模式 <<<\n";
        string code;
        int shares;
        cout << "請輸入商品代號: "; cin >> code;

        if (userPortfolio[code] <= 0) { cout << "[錯誤] 您未持有此股票！\n"; system("pause"); return; }

        FinancialProduct* prod = nullptr;
        for (const auto& p : marketProducts) {
            if (p->getCode() == code) prod = p.get();
        }

        cout << "持有 " << userPortfolio[code] << " 股。欲賣出股數: "; cin >> shares;
        if (shares <= 0 || shares > userPortfolio[code]) { cout << "[錯誤] 股數無效！\n"; system("pause"); return; }

        double revenue = prod->getCurrentPrice() * shares;
        userBalance += revenue;
        userPortfolio[code] -= shares;
        cout << "[成功] 賣出成功！獲得現金 $" << revenue << "\n";
        system("pause");
    }

    void saveToFile() const {
        ofstream outFile(filename);
        outFile << userBalance << " " << currentDay << "\n";
        outFile << marketProducts.size() << "\n";
        for (const auto& prod : marketProducts) {
            outFile << prod->getProductType() << " " << prod->getCode() << " " << prod->getName() << " " << prod->getCurrentPrice() << "\n";
        }
        outFile << userPortfolio.size() << "\n";
        for (const auto& pair : userPortfolio) {
            outFile << pair.first << " " << pair.second << "\n";
        }
        outFile.close();
    }

    void loadFromFile() {
        ifstream inFile(filename);
        // 強制重置：不論有沒有舊檔案，都用全新指定的股票清單
        if (!inFile) {
            initMarket();
            return;
        }

        inFile >> userBalance >> currentDay;
        // 如果偵測到舊檔案的錢不是 5000 萬，直接強制覆蓋回 5000 萬
        if (userBalance < 40000000.0) {
            userBalance = 50000000.0;
        }

        initMarket(); // 呼叫你指定的新股票
        userPortfolio.clear();
        inFile.close();
    }
};

// ==========================================
// 主程式
// ==========================================
int main() {
    StockExchange exchange;
    exchange.loadFromFile();

    exchange.showMarket();

    thread updateThread(&StockExchange::updateMarketPricesPeriodically, &exchange);

    char input = '\0'; // 完美修正你的安全檢查錯誤！
    do {
        if (_kbhit()) {
            input = _getch();
            if (input == 'b' || input == 'B') {
                exchange.buyStock();
            }
            else if (input == 's' || input == 'S') {
                exchange.sellStock();
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    } while (input != 'q' && input != 'Q');

    isRunning = false;
    if (updateThread.joinable()) {
        updateThread.join();
    }
    exchange.saveToFile();

    cout << "\n已安全存檔並退出系統。\n";
    return 0;
}
