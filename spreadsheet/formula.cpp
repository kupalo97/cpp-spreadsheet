#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>


namespace {
    
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {
        }
        catch (...) {
            throw FormulaException("formula syntax error");
        }
        
        Value Evaluate(const SheetInterface& sheet) const override {
            const std::function<double(Position)> arguments = [&sheet](const Position p) -> double {
                if (!p.IsValid()) {
                    throw FormulaError(FormulaError::Category::Ref);
                }
                const auto* cell = sheet.GetCell(p);
                if (!cell) {
                    return 0;
                }
                if (std::holds_alternative<double>(cell->GetValue())) {
                    return std::get<double>(cell->GetValue());
                } else if (std::holds_alternative<std::string>(cell->GetValue())) {
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
                return ast_.Execute(arguments);
            }
            catch (FormulaError& error) {
                return error;
            }
        }
        

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> cells;
            for (auto cell : ast_.GetCells()) {
                if (cell.IsValid()) {
                    cells.push_back(cell);
                }
            }
          
            auto last = cells.end();
            for (auto first = cells.begin(); first < last; ++first) last = remove(std::next(first), last, *first);
            cells.erase(last, cells.end());
            return cells;
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

    private:
        const FormulaAST ast_;        
    };
    
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

FormulaError::FormulaError(Category category) : category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!";
    case Category::Value:
        return "#VALUE!";
    case Category::Div0:
        return "#DIV/0!";
    }
    return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}