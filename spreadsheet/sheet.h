#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <optional>

struct Hasher {
    size_t operator()(const Position& pos) const;
};


class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos)  override;
    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
private:
    std::unordered_map<Position, std::unique_ptr<Cell>, Hasher> cells_;

    CellInterface* GetCellUni(Position pos) const;
    std::pair<Position, Position> GetWorkSpace() const;
};
