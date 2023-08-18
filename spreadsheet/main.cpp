#include <limits>
#include <iostream>
#include "common.h"
#include "formula.h"
#include "test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

void PrintSheet(const std::unique_ptr<SheetInterface>& sheet, std::ostream& out) {
    out << sheet->GetPrintableSize() << std::endl;
    sheet->PrintTexts(out);
    out << std::endl;
    sheet->PrintValues(out);
    out << std::endl;
}

void TestClearPrint() {
    auto sheet = CreateSheet();
    for (int i = 0; i <= 5; ++i) {
        sheet->SetCell(Position{i, i}, std::to_string(i));
    }

    sheet->ClearCell(Position{3, 3});

    for (int i = 5; i >= 0; --i) {
        sheet->ClearCell(Position{i, i});
        PrintSheet(sheet, std::cout);
    }
}
int main() {
    TestClearPrint();
}
