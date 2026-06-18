#include <iostream>
#include <thread>
#include <mutex>
#include <conio.h>
#include "StockExchange.h"

using namespace std;

mutex mtx;
bool isRunning = true;

int main() {
    StockExchange exchange;
    exchange.loadFromFile();
    exchange.showMarket();

    // 啟動背景更新執行緒
    thread updateThread(&StockExchange::updateMarketPricesPeriodically, &exchange);

    char input = '\0';
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

    cout << "\n已安全存檔並退出系統。祝您投資順利，期末歐趴！\n";
    return 0;
}
