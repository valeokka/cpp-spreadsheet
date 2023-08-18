#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;

    if(text.empty()){
        impl = std::make_unique<EmptyImpl>("");
    }else if(text[0] == FORMULA_SIGN && text.size() > 1){
        try{
            impl = std::make_unique<FormulaImpl>(text, sheet_);
        }
        catch(...){
            throw FormulaException("Incorrect formula format");
        }
    }else{
        impl = std::make_unique<TextImpl>(text);
    }

    impl_ = std::move(impl);
    referenced_cells_ = impl_-> GetReferencedCells();
}

void Cell::Clear() {
    impl_.reset();
}

Cell::Value Cell::GetValue() const {
    if(!cached_value_.has_value()){
        cached_value_ = std::make_optional<Value>(impl_->GetValue());
    }
    return cached_value_.value();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const{
    return referenced_cells_;
}

bool Cell::IsReferenced() const{
    return !referenced_cells_.empty();
}

std::unordered_set<Cell*>& Cell::GetReferencedBy(){
    return referenced_by;
}

std::unordered_set<Cell*>& Cell::GetReferenceTo(){
    return reference_to;
}

void Cell::InvalidateCache(){
    cached_value_.reset();
}

void Cell::InvalidateCacheChilds(){
    InvalidateCache();
    std::stack<Cell*> to_invalidate;
    for(Cell* ref_to : reference_to){
        to_invalidate.push(ref_to);
    }
    while (!to_invalidate.empty()){
        Cell* invalidate_cell = to_invalidate.top();
        to_invalidate.pop();
        if(invalidate_cell->cached_value_.has_value()){
            for(Cell* ref_to : invalidate_cell->reference_to){
                to_invalidate.push(ref_to);
            }
            invalidate_cell->InvalidateCache();
        }
    }
}

bool Cell::TestCyclicDependance(const CellInterface* cell, Position head) const{
    if(cell == nullptr) { return false;}

    bool res = false;
    for(const auto& next_cell_pos : cell->GetReferencedCells()) {
        if(next_cell_pos == head) {
            return true;
        }
        res = res || TestCyclicDependance(sheet_.GetCell(next_cell_pos), head);
    }
    return res;
}
