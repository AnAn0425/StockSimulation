#ifndef STOCK_EXCHANGE_H
#define STOCK_EXCHANGE_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "FinancialProduct.h"

using namespace std;

class StockExchange {
private:
    vector<unique_ptr<FinancialProduct>> marketProducts;
    map<string, int> userPortfolio;
    double userBalance;
    int currentDay;
    const string filename = "stock_saveData.txt";

public:
    StockExchange();
    void initMarket();
    void updateMarketPricesPeriodically();
    void showMarket() const;
    void buyStock();
    void sellStock();
    void saveToFile() const;
    void loadFromFile();
};

#endif
