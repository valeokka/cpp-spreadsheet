#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface& sheet_);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::optional<CellInterface::Value> cached_value_;

    class Impl{
    public:
        explicit Impl(const std::string raw);
        virtual Value GetValue() = 0;
        virtual std::string GetText() = 0;
        virtual GetReference() = 0;
    protected:
        std::string raw_text_;
        std::vector<Position> parents_;
        std::vector<Position> childs_;
    };

    class EmptyImpl final : public Impl{
    public:
        explicit EmptyImpl(const std::string raw);
        Value GetValue() override;
        std::string GetText() override;
    };

    class TextImpl : public Impl{
    public:
        explicit TextImpl(const std::string raw);
        Value GetValue() override;
        std::string GetText() override;
    }

    class FormulaImpl : public Impl{
    public:
        explicit FormulaImpl(const std::string& raw, SheetInterface& sheet_);
        Value GetValue() override;
        std::string GetText() override;
        std::forward_list<Position> GetReferencedCells() const;  
        void InvalidateCache();      

    private:
        std::unique_ptr<FormulaInterface> formula_;
        std::optional<CellInterface::Value> cached_value_;
        SheetInterface& sheet_;
    };

};