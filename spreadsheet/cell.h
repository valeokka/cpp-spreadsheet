#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>
#include <vector>
#include <stack>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet)
    : sheet_(sheet),
      impl_(std::make_unique<EmptyImpl>("")) {}
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;

    std::unordered_set<Cell*>& GetReferencedBy();
    std::unordered_set<Cell*>& GetReferenceTo();

    bool TestCyclicDependance(const CellInterface* cell, Position head) const;
    void InvalidateCacheChilds();

private:
    class Impl;
    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    mutable std::optional<CellInterface::Value> cached_value_;
    std::vector<Position> referenced_cells_;
    std::unordered_set<Cell*> referenced_by;
    std::unordered_set<Cell*> reference_to;

    void InvalidateCache();

    class Impl{
    public:
        explicit Impl(const std::string raw) 
        : raw_text_(raw) {}
        virtual Value GetValue() = 0;
        virtual std::string GetText() = 0;
        virtual std::vector<Position> GetReferencedCells() { return {};}
    protected:
        std::string raw_text_;
    };

    class EmptyImpl final : public Impl{
    public:
        explicit EmptyImpl(const std::string raw)
        : Impl(raw)
        {}

        Value GetValue() override{ return "";}
        std::string GetText() override{ return "";}
    };

    class TextImpl : public Impl{
    public:
        explicit TextImpl(const std::string raw)
        : Impl(raw)
        {}

        Value GetValue() override{ return raw_text_[0] == ESCAPE_SIGN ? raw_text_.substr(1) : raw_text_;}
        std::string GetText() override{ return raw_text_;}
    };

    class FormulaImpl : public Impl{
    public:
        explicit FormulaImpl(const std::string& raw, SheetInterface& sheet)
        : Impl(raw),
          sheet_(sheet),
          formula_(ParseFormula(raw.substr(1))) 
          {}

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
};