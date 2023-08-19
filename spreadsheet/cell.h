#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>
#include <vector>
#include <stack>
#include <algorithm>

class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface& sheet);
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

    void Swap(Cell& other);
    bool Empty();

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    mutable std::optional<CellInterface::Value> cached_value_;
    std::vector<Position> referenced_cells_;
    std::unordered_set<Cell*> referenced_by;
    std::unordered_set<Cell*> reference_to;
    bool is_empty = false;

    void InvalidateCache();
};