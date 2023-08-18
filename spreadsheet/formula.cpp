#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

FormulaError::FormulaError(Category category)
  : category_(category)
  {}

FormulaError::Category FormulaError::GetCategory() const{
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const{
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const{
    switch (category_){
        case FormulaError::Category::Div0:
            return "#DIV/0!";
        break;
        case FormulaError::Category::Value:
            return "#VALUE!";
        break;
        case FormulaError::Category::Ref:
            return "#REF!";
        break;
    }
    return "";
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(const std::string& expression) try :
        ast_(ParseFormulaAST(expression))
        {}
    catch(const FormulaException& err){
        throw err;
    }

    Value Evaluate(const SheetInterface& sheet) const override {

        const SheetArgs args = [&sheet](const Position p) -> double {
            if (!p.IsValid()) { throw FormulaError(FormulaError::Category::Ref);}
            
            const auto* cell = sheet.GetCell(p);
            if (!cell) { return 0.0;}

            if (std::holds_alternative<double>(cell->GetValue())) {
                return std::get<double>(cell->GetValue());
            }

            if (std::holds_alternative<std::string>(cell->GetValue())) {
                auto value = std::get<std::string>(cell->GetValue());
                double result = 0;
                if (!value.empty()) {
                    std::istringstream in(value);
                    if (!(in >> result) || !in.eof()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                return result;
            }
            throw FormulaError(std::get<FormulaError>(cell->GetValue()));
        };

        try {
            return ast_.Execute(args);
        }
        catch (FormulaError& fe) {
            return FormulaError(fe.GetCategory());
        }
    }

    std::string GetExpression() const override {
        std::ostringstream os;
        ast_.PrintFormula(os);
        return os.str();
    }

    std::vector<Position> GetReferencedCells() const{
        std::vector<Position> result{};
        for (auto cell : ast_.GetCells()) {
            if (cell.IsValid()) {
                result.push_back(cell);
            }
        }
        result.resize(std::unique(result.begin(), result.end()) - result.begin());
        return result;
    }
private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("");
    }
}