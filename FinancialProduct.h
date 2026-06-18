#ifndef FINANCIAL_PRODUCT_H
#define FINANCIAL_PRODUCT_H

#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

// 金融商品基底類別
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

// 普通股票衍生類別
class Stock : public FinancialProduct {
public:
    Stock(string c, string n, double p) : FinancialProduct(c, n, p) {}
    void updatePrice() override {
        double changePercent = ((rand() % 300) - 150) / 10000.0; 
        currentPrice = currentPrice * (1.0 + changePercent);
        if (currentPrice < 1.0) currentPrice = 1.0;
        priceHistory.push_back(currentPrice);
    }
    string getProductType() const override { return "普通股票"; }
};

#endif
