#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;


class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string data);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;



private:
    //можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const {
            return {};
        }
        virtual void InvalCash() {}
    };


    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };


    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);
        Cell::Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string data_;
    };


    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string expression, const SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalCash() override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };

    std::unique_ptr<Impl> impl_;

    SheetInterface& sheet_;

    std::unordered_set<Cell*> heirs_;

    void InvalCash();

    bool CircDepControl(const Impl& new_cell) const;
};


