#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <cmath>
#include <vector>

using Cell_ptr = std::unique_ptr<Cell>;

struct PositionHash {
    size_t operator()(const Position& pos) const {
        size_t hash_row = std::hash<int>{}(pos.row);
        size_t hash_col = std::hash<int>{}(pos.col);
        return hash_row ^ (hash_col << 1);
    }
};

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    Cell* GetOrCreateCell(Position pos);

private:
    std::unordered_map<Position, Cell_ptr, PositionHash> cells_;
    mutable Size printable_size_;
    std::vector<int> rows_count_;
    std::vector<int> cols_count_;
    
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
            return std::string{value.ToString()};
        }
        
    };

    bool IsValid(Position pos) const;
    void RefreshPrintableSize() const;
    void TestPosition(Position pos) const;
};
