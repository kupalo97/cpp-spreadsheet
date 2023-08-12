#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>


Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string data) {
    std::unique_ptr<Impl> new_cell;

    if (data.empty()) {
        new_cell = std::make_unique<EmptyImpl>();
    }
    else if (data.size() >= 2 && data[0] == FORMULA_SIGN) {
        new_cell = std::make_unique<FormulaImpl>(std::move(data), sheet_);
    }
    else {
        new_cell = std::make_unique<TextImpl>(std::move(data));
    }

    if (CircDepControl(*new_cell)) {
        throw CircularDependencyException("New cell creates a circular dependency");
    }

    impl_ = std::move(new_cell);

    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));

        if (cell == nullptr) {
            sheet_.SetCell(pos, "");
            cell = static_cast<Cell*>(sheet_.GetCell(pos));
        }

        cell->heirs_.insert(this);
    }

    InvalCash();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}



void Cell::InvalCash() {
    impl_->InvalCash();
    for (Cell* cell : heirs_) {
        cell->InvalCash();
    }
}

bool Cell::CircDepControl(const Impl& new_cell) const {
    std::stack<const Cell*> cells_for_control;
    std::unordered_set<const Cell*> verified_cells;
    std::unordered_set<const Cell*> referenced_cells;
    cells_for_control.push(this);

    for (const auto& pos : new_cell.GetReferencedCells()) {
        const Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
        referenced_cells.insert(cell);
    }

    while (!cells_for_control.empty()) {
        const Cell* current_cell = cells_for_control.top();
        cells_for_control.pop();

        if (referenced_cells.count(current_cell)) {
            return true;
        }

        verified_cells.insert(current_cell);

        for (const Cell* cell : current_cell->heirs_) {
            if (!verified_cells.count(cell)) {
                cells_for_control.push(cell);
            }
        }
    }
    return false;
}




Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}



Cell::TextImpl::TextImpl(std::string text) : data_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const  {
    if (data_[0] == ESCAPE_SIGN) {
        return data_.substr(1);
    }
    return data_;
}
    
std::string Cell::TextImpl::GetText() const  {
    return data_;
}


    Cell::FormulaImpl::FormulaImpl(std::string expression, const SheetInterface& sheet)
    : sheet_(sheet) {   
    formula_ = ParseFormula(expression.substr(1));
}
    
Cell::Value Cell::FormulaImpl::GetValue() const  {
    FormulaInterface::Value value;
    if (!cache_.has_value()) {
        cache_ = formula_->Evaluate(sheet_);
    }
    value = cache_.value();
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    return std::get<FormulaError>(value);
}
    
std::string Cell::FormulaImpl::GetText() const  {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const  {
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::InvalCash()  {
    cache_.reset();
}


