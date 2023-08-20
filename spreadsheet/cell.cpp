#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

class Cell::Impl{
public:
explicit Impl(const std::string raw) 
: raw_text_(raw) {}
virtual Value GetValue() = 0;
    virtual std::string GetText() = 0;
    virtual std::vector<Position> GetReferencedCells() { return {};}
protected:
    std::string raw_text_;
};

class Cell::EmptyImpl final : public Cell::Impl{
public:
    explicit EmptyImpl(const std::string raw)
    : Cell::Impl(raw) {}

    Value GetValue() override{ return "";}
    std::string GetText() override{ return "";}
};

class Cell::TextImpl : public Cell::Impl{
public:
    explicit TextImpl(const std::string raw)
    : Cell::Impl(raw) {}

    Value GetValue() override{ return raw_text_[0] == ESCAPE_SIGN ? raw_text_.substr(1) : raw_text_;}
    std::string GetText() override{ return raw_text_;}
};

class Cell::FormulaImpl : public Cell::Impl{
public:
    explicit FormulaImpl(const std::string& raw, SheetInterface& sheet)
    : Cell::Impl(raw),
    sheet_(sheet),
    formula_(ParseFormula(raw.substr(1))) {}

    Value GetValue() override{
        FormulaInterface::Value result = formula_->Evaluate(sheet_);

        if(std::holds_alternative<double>(result)){
            return std::get<double>(result);
        }else if(std::holds_alternative<FormulaError>(result)){
            return std::get<FormulaError>(result);
        }
        return raw_text_;
    }

    std::string GetText() override{return FORMULA_SIGN + formula_->GetExpression();}
    std::vector<Position> GetReferencedCells() override { return formula_->GetReferencedCells();}

private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet),
    impl_(std::make_unique<EmptyImpl>("")) {}

Cell::~Cell() = default;

void Cell::Set(std::string text, Position pos) {
    std::unique_ptr<Impl> impl;

    bool NeedCyclicTest = false;

    if(text.empty()){
        impl = std::make_unique<EmptyImpl>("");
    }else if(text[0] == FORMULA_SIGN && text.size() > 1){
        try{
            impl = std::make_unique<FormulaImpl>(text, sheet_);
            NeedCyclicTest = true;
        }
        catch(...){
            throw FormulaException("Incorrect formula format");
        }
    }else{
        impl = std::make_unique<TextImpl>(text);
    }
    if(NeedCyclicTest && TestCyclicDependance(impl->GetReferencedCells(), pos)){
        throw CircularDependencyException("Circular dependency"); 
    }
    for (Cell* cell: referenced_by){
        cell->GetReferenceTo().erase(this);
    }
    referenced_by.clear();
    // for (Cell* cell: reference_to){
    //     cell->GetReferencedBy().erase(this);
    // }
    // reference_to.clear();
    

    impl_ = std::move(impl);
    referenced_cells_ = impl_-> GetReferencedCells();
    
    InvalidateCacheChilds();
    is_empty = false;
}

void Cell::Clear() {
    Set("", {-1,-1});
    is_empty = true;
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

bool Cell::TestCyclicDependance(const std::vector<Position>& referenced_cells, Position head) const{
    if(referenced_cells.empty()) { return false;}

    bool res = false;
    for(const auto& next_cell_pos : referenced_cells) {
        if(next_cell_pos == head) {
            return true;
        }
        res = res || TestCyclicDependance(sheet_.GetCell(next_cell_pos), head);
    }
    return res;
}

bool Cell::Empty(){
    return is_empty;
}