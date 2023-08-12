#include "sheet.h"




size_t Hasher::operator() (const Position& pos) const {
    return std::hash<int>()(pos.row) * 777 + 777 * std::hash<int>()(pos.col);
}

Sheet::~Sheet() {
}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Position is incorrect");
    }
    if (!cells_.count(pos)) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    cells_.at(pos)->Set(std::move(text));
}


CellInterface* Sheet::GetCellUni(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is incorrect");
    }
    if (cells_.count(pos)) return  cells_.at(pos).get();
    else return nullptr;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellUni(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellUni(pos);
}

void Sheet::ClearCell(Position pos) {
   
    if(!pos.IsValid()) {
        throw InvalidPositionException("Position is incorrect");
    }
    cells_.erase(pos);
}



void Sheet::PrintValues(std::ostream& output) const {
    if(cells_.empty()) {
        return;
    }
    auto [left_top, right_down] = GetWorkSpace();
    for(int row = left_top.row; row <= right_down.row; ++row) {
        for(int col = left_top.col; col <= right_down.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto ptr_cell = GetCell(Position{row, col});
            if(nullptr != ptr_cell) {
                std::visit([&](const auto value) {output << value;}, ptr_cell->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if(cells_.empty()) {
        return;
    }
    auto [left_top, right_down] = GetWorkSpace();
    for(int row = left_top.row; row <= right_down.row; ++row) {
        for(int col = left_top.col; col <= right_down.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto ptr_cell = GetCell(Position{row, col});
            if(nullptr != ptr_cell) {
                output << ptr_cell->GetText();
            }
        }
        output << '\n';
    }
}

std::pair<Position, Position> Sheet::GetWorkSpace() const {
    Position left_top{0, 0};
    Position right_down{0, 0};

    if(cells_.empty()) {
        return { left_top, right_down };
    }
    else {
        for (const auto& [pos, cell] : cells_) {
            if (nullptr != cell) {
                right_down.row = std::max(right_down.row, pos.row);
                right_down.col = std::max(right_down.col, pos.col);
            }
        }
        return { left_top, right_down };
    }
}

Size Sheet::GetPrintableSize() const {
    if (cells_.empty()) {
        return Size{ 0, 0 };
    }
    auto [left_top, right_down] = GetWorkSpace();
    return Size{ right_down.row - left_top.row + 1,
                right_down.col - left_top.col + 1 };
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
