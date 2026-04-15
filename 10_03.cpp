#include <boost/multi_array.hpp>

#include <cassert>
#include <cstddef>
#include <iostream>

using Field = boost::multi_array<int, 2>;

Field make_field(const std::size_t rows, const std::size_t cols)
{
    Field field(boost::extents[rows][cols]);
    std::size_t row = 0U;
    std::size_t col = 0U;

    for (row = 0U; row < rows; ++row)
    {
        for (col = 0U; col < cols; ++col)
        {
            field[row][col] = 0;
        }
    }

    return field;
}

void set_cell(Field& field, const std::size_t row, const std::size_t col, const int value)
{
    field[row][col] = value;
}

int count_neighbors(const Field& field, const std::size_t row, const std::size_t col)
{
    const std::size_t rows = field.shape()[0];
    const std::size_t cols = field.shape()[1];
    const int offsets[3] = {-1, 0, 1};
    int neighbors = 0;
    int dr = 0;
    int dc = 0;
    int next_row = 0;
    int next_col = 0;

    for (dr = 0; dr < 3; ++dr)
    {
        for (dc = 0; dc < 3; ++dc)
        {
            if (offsets[dr] == 0 && offsets[dc] == 0)
            {
                continue;
            }

            next_row = static_cast<int>(row) + offsets[dr];
            next_col = static_cast<int>(col) + offsets[dc];

            if (next_row >= 0 && next_col >= 0 &&
                static_cast<std::size_t>(next_row) < rows &&
                static_cast<std::size_t>(next_col) < cols)
            {
                neighbors += field[static_cast<std::size_t>(next_row)][static_cast<std::size_t>(next_col)];
            }
        }
    }

    return neighbors;
}

Field next_generation(const Field& current)
{
    const std::size_t rows = current.shape()[0];
    const std::size_t cols = current.shape()[1];
    Field next = make_field(rows, cols);
    std::size_t row = 0U;
    std::size_t col = 0U;
    int neighbors = 0;

    for (row = 0U; row < rows; ++row)
    {
        for (col = 0U; col < cols; ++col)
        {
            neighbors = count_neighbors(current, row, col);

            if (current[row][col] == 1)
            {
                next[row][col] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            }
            else
            {
                next[row][col] = (neighbors == 3) ? 1 : 0;
            }
        }
    }

    return next;
}

bool fields_equal(const Field& left, const Field& right)
{
    const std::size_t rows = left.shape()[0];
    const std::size_t cols = left.shape()[1];
    std::size_t row = 0U;
    std::size_t col = 0U;

    if (rows != right.shape()[0] || cols != right.shape()[1])
    {
        return false;
    }

    for (row = 0U; row < rows; ++row)
    {
        for (col = 0U; col < cols; ++col)
        {
            if (left[row][col] != right[row][col])
            {
                return false;
            }
        }
    }

    return true;
}

void print_field(const Field& field, const std::size_t iteration)
{
    const std::size_t rows = field.shape()[0];
    const std::size_t cols = field.shape()[1];
    const char alive = '#';
    const char dead = '.';
    std::size_t row = 0U;
    std::size_t col = 0U;

    std::cout << "Iteration " << iteration << '\n';

    for (row = 0U; row < rows; ++row)
    {
        for (col = 0U; col < cols; ++col)
        {
            std::cout << (field[row][col] == 1 ? alive : dead);
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}

void test_blinker()
{
    const std::size_t size = 10U;
    Field initial = make_field(size, size);
    Field expected_after_one = make_field(size, size);
    Field expected_after_two = make_field(size, size);
    Field after_one = make_field(size, size);
    Field after_two = make_field(size, size);

    set_cell(initial, 4U, 3U, 1);
    set_cell(initial, 4U, 4U, 1);
    set_cell(initial, 4U, 5U, 1);

    set_cell(expected_after_one, 3U, 4U, 1);
    set_cell(expected_after_one, 4U, 4U, 1);
    set_cell(expected_after_one, 5U, 4U, 1);

    set_cell(expected_after_two, 4U, 3U, 1);
    set_cell(expected_after_two, 4U, 4U, 1);
    set_cell(expected_after_two, 4U, 5U, 1);

    after_one = next_generation(initial);
    after_two = next_generation(after_one);

    assert(fields_equal(after_one, expected_after_one));
    assert(fields_equal(after_two, expected_after_two));
}

int main()
{
    const std::size_t size = 10U;
    const std::size_t iterations = 10U;
    Field field = make_field(size, size);
    std::size_t step = 0U;

    test_blinker();

    set_cell(field, 4U, 3U, 1);
    set_cell(field, 4U, 4U, 1);
    set_cell(field, 4U, 5U, 1);

    for (step = 0U; step <= iterations; ++step)
    {
        print_field(field, step);

        if (step < iterations)
        {
            field = next_generation(field);
        }
    }
}