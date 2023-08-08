#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl;

    class EmptyImpl : public Impl;

    class TextImpl : public Impl;

    class FormulaImpl : public Impl;

    std::unique_ptr<Impl> impl_;

    SheetInterface& sheet_;

};