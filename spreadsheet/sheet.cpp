#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;
Sheet::Sheet(){
    rows_count_.resize(Position::MAX_ROWS, 0);
    cols_count_.resize(Position::MAX_COLS, 0);
}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()){
        throw InvalidPositionException("invalid position");
    }

    bool IsExists = cells_.count(pos);
    if(!IsExists){ cells_[pos] = std::make_unique<Cell>(*this); }

    Cell* cell = cells_.at(pos).get();
    cell->Set(text, pos);

    if(cell->IsReferenced()){
        for(Position reference_pos : cell->GetReferencedCells()){
            if(!cells_.count(reference_pos)){SetCell(reference_pos, "");}
            Cell* reference_cell = cells_.at(reference_pos).get();
            if(!cell->GetReferencedBy().count(reference_cell)){
                cell->GetReferencedBy().insert(reference_cell);
            }
            if(!reference_cell->GetReferenceTo().count(cell)){
                reference_cell->GetReferenceTo().insert(cell);
            }
        }
    }
    

    if(!IsExists){
        if(int(rows_count_.size()) < pos.row) { rows_count_.push_back(0);} 
        ++rows_count_[pos.row];
        printable_size_.rows = printable_size_.rows < pos.row + 1 ? pos.row +1 : printable_size_.rows;

        if(int(cols_count_.size()) < pos.col) { cols_count_.push_back(0);} 
        ++cols_count_[pos.col];
        printable_size_.cols = printable_size_.cols < pos.col + 1 ? pos.col +1 : printable_size_.cols;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(IsValid(pos)){
        return cells_.at(pos).get(); 
    } else {
        return nullptr;
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    //в случае если ячейка была очищена должен возвращать nullptr,
    //поэтому если cell = EmptyImpl возвращает nullptr
    if(IsValid(pos) && !cells_.at(pos)->Empty()){
        return cells_.at(pos).get(); 
    } else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    if(IsValid(pos)){
        cells_.at(pos)->Clear(); 
        --rows_count_[pos.row];
        --cols_count_[pos.col];
    } 
}

Size Sheet::GetPrintableSize() const {
    RefreshPrintableSize();

    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
        for (int row = 0; row < printable_size.rows; ++row) {

        for (int col = 0; col < printable_size.cols; ++col) {
            if (col > 0) {
                output << "\t";
            }
            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                std::visit([&](const auto value) {output << value; }, it->second->GetValue());
            }
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    Size printable_size = GetPrintableSize();
    for (int row = 0; row < printable_size.rows; ++row) {

        for (int col = 0; col < printable_size.cols; ++col) {
            if (col > 0) {
                output << "\t";
            }
            const auto& it = cells_.find({ row, col });
            if (it != cells_.end() && it->second != nullptr && !it->second->GetText().empty()) {
                output << it->second->GetText();
            }
        }
        output << "\n";
    }
}

bool Sheet::IsValid(Position pos) const {
    TestPosition(pos);
    return cells_.count(pos);
}

void Sheet::RefreshPrintableSize() const {
    while(rows_count_[printable_size_.rows - 1] == 0  && printable_size_.rows != 0){
        --printable_size_.rows;
    }
    while(cols_count_[printable_size_.cols - 1] == 0 && printable_size_.cols != 0){
        --printable_size_.cols;
    }
}

void Sheet::TestPosition(Position pos) const{
    if(!pos.IsValid()){
        throw InvalidPositionException("Invalid Position"s);
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}