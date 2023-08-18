#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <cmath>
#include <vector>

using Cell_ptr = std::unique_ptr<Cell>;
using Cols = std::unordered_map<int, Cell_ptr>;

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::unordered_map<Position, Cell_ptr, PositionHash> cells_;
    mutable Size printable_size_;
    mutable std::vector<int> rows_count_;
    mutable std::vector<int> cols_count_;
    
    struct CellValuePrinter {
        std::string operator()(double value) const {
            double int_check;
            if(std::modf(value, &int_check) == 0.0){
                return std::to_string(int(value));
            }else{
               return std::to_string(value); 
            }
        }
        std::string operator()(const std::string& value) const {
            return value;
        }
        std::string operator()(const FormulaError& value) const {
            std::string result = "#";
            result = result + value.what() + "!";
            return result;
        }
        
    };

    bool IsValid(Position pos) const;
    void RefreshPrintableSize() const;
    void TestPosition(Position pos) const;
};