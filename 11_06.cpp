#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

namespace tsp {

using Graph = boost::adjacency_matrix<
    boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, int>>;

using Edge = boost::graph_traits<Graph>::edge_descriptor;
using EdgeInsertResult = std::pair<Edge, bool>;
using WeightMatrix = std::vector<std::vector<int>>;
using Path = std::vector<std::size_t>;

struct GraphData
{
    Graph graph;
    WeightMatrix matrix;
};

struct TspResult
{
    Path path;
    int cost;
};

std::size_t default_vertex_count()
{
    const std::size_t value = 10U;

    return value;
}

std::size_t default_start_vertex()
{
    const std::size_t value = 0U;

    return value;
}

int min_edge_weight()
{
    const int value = 1;

    return value;
}

int max_edge_weight()
{
    const int value = 10;

    return value;
}

WeightMatrix make_empty_matrix(std::size_t vertex_count)
{
    const int zero = 0;
    WeightMatrix matrix(vertex_count, std::vector<int>(vertex_count, zero));

    return matrix;
}

void add_weighted_edge(Graph& graph, std::size_t left, std::size_t right,
    int weight)
{
    EdgeInsertResult inserted = boost::add_edge(left, right, graph);

    assert(inserted.second);
    boost::put(boost::edge_weight, graph, inserted.first, weight);
}

bool is_valid_matrix(WeightMatrix const& matrix)
{
    const std::size_t vertex_count = matrix.size();
    const int diagonal_weight = 0;
    std::size_t row = 0U;
    std::size_t column = 0U;
    bool valid = vertex_count != 0U;

    for (row = 0U; valid && row < vertex_count; ++row) {
        valid = matrix[row].size() == vertex_count;
    }

    for (row = 0U; valid && row < vertex_count; ++row) {
        valid = matrix[row][row] == diagonal_weight;

        for (column = row + 1U; valid && column < vertex_count; ++column) {
            valid = matrix[row][column] == matrix[column][row]
                && min_edge_weight() <= matrix[row][column]
                && matrix[row][column] <= max_edge_weight();
        }
    }

    return valid;
}

GraphData build_complete_graph_from_matrix(WeightMatrix const& matrix)
{
    const std::size_t vertex_count = matrix.size();
    GraphData data = {Graph(vertex_count), matrix};
    std::size_t left = 0U;
    std::size_t right = 0U;

    assert(is_valid_matrix(matrix));

    for (left = 0U; left < vertex_count; ++left) {
        for (right = left + 1U; right < vertex_count; ++right) {
            add_weighted_edge(data.graph, left, right, matrix[left][right]);
        }
    }

    return data;
}

GraphData generate_random_complete_graph(std::size_t vertex_count,
    std::default_random_engine& generator)
{
    WeightMatrix matrix = make_empty_matrix(vertex_count);
    std::uniform_int_distribution<int> distribution(min_edge_weight(),
        max_edge_weight());
    std::size_t left = 0U;
    std::size_t right = 0U;
    int weight = min_edge_weight();

    assert(vertex_count != 0U);

    for (left = 0U; left < vertex_count; ++left) {
        for (right = left + 1U; right < vertex_count; ++right) {
            weight = distribution(generator);
            matrix[left][right] = weight;
            matrix[right][left] = weight;
        }
    }

    return build_complete_graph_from_matrix(matrix);
}

int edge_weight(Graph const& graph, std::size_t from, std::size_t to)
{
    const int missing_weight = 0;
    auto edge = boost::edge(from, to, graph);

    assert(edge.second);
    if (!edge.second) {
        return missing_weight;
    }

    return boost::get(boost::edge_weight, graph, edge.first);
}

int calculate_cycle_cost(Graph const& graph, Path const& path)
{
    int cost = 0;
    std::size_t index = 0U;

    assert(path.size() >= 2U);

    for (index = 0U; index + 1U < path.size(); ++index) {
        cost += edge_weight(graph, path[index], path[index + 1U]);
    }

    return cost;
}

bool is_hamiltonian_cycle(Path const& path, std::size_t vertex_count,
    std::size_t start_vertex)
{
    std::vector<bool> used(vertex_count, false);
    std::size_t index = 0U;
    std::size_t vertex = 0U;
    bool valid = vertex_count != 0U
        && path.size() == vertex_count + 1U
        && path.front() == start_vertex
        && path.back() == start_vertex;

    for (index = 0U; valid && index < vertex_count; ++index) {
        vertex = path[index];
        valid = vertex < vertex_count && !used[vertex];

        if (valid) {
            used[vertex] = true;
        }
    }

    return valid && std::all_of(used.begin(), used.end(), [](bool value) {
        return value;
    });
}

TspResult solve_tsp(Graph const& graph, std::size_t start_vertex)
{
    const std::size_t vertex_count = boost::num_vertices(graph);
    Path permutation = {};
    Path current_path = {};
    TspResult best = {{}, std::numeric_limits<int>::max()};
    std::size_t vertex = 0U;
    int current_cost = 0;

    assert(vertex_count != 0U);
    assert(start_vertex < vertex_count);

    permutation.reserve(vertex_count - 1U);
    current_path.reserve(vertex_count + 1U);

    for (vertex = 0U; vertex < vertex_count; ++vertex) {
        if (vertex != start_vertex) {
            permutation.push_back(vertex);
        }
    }

    do {
        current_path.clear();
        current_path.push_back(start_vertex);
        current_path.insert(current_path.end(), permutation.begin(),
            permutation.end());
        current_path.push_back(start_vertex);

        current_cost = calculate_cycle_cost(graph, current_path);

        if (current_cost < best.cost) {
            best.path = current_path;
            best.cost = current_cost;
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));

    assert(is_hamiltonian_cycle(best.path, vertex_count, start_vertex));

    return best;
}

void print_weight_matrix(WeightMatrix const& matrix)
{
    const int output_width = 4;
    std::size_t row = 0U;
    std::size_t column = 0U;

    std::cout << "Weight matrix:\n";

    for (row = 0U; row < matrix.size(); ++row) {
        for (column = 0U; column < matrix[row].size(); ++column) {
            std::cout << std::setw(output_width) << matrix[row][column];
        }

        std::cout << '\n';
    }
}

void print_path(Path const& path)
{
    std::size_t index = 0U;

    std::cout << "Best route: ";

    for (index = 0U; index < path.size(); ++index) {
        if (index != 0U) {
            std::cout << " -> ";
        }

        std::cout << path[index];
    }

    std::cout << '\n';
}

void print_result(GraphData const& graph_data, TspResult const& result)
{
    print_weight_matrix(graph_data.matrix);
    print_path(result.path);
    std::cout << "Best cost: " << result.cost << '\n';
}

WeightMatrix make_test_matrix_small()
{
    const WeightMatrix matrix = {
        {0, 2, 9, 10},
        {2, 0, 6, 4},
        {9, 6, 0, 8},
        {10, 4, 8, 0}
    };

    return matrix;
}

WeightMatrix make_test_matrix_ring(std::size_t vertex_count)
{
    WeightMatrix matrix = make_empty_matrix(vertex_count);
    std::size_t left = 0U;
    std::size_t right = 0U;
    std::size_t next = 0U;

    for (left = 0U; left < vertex_count; ++left) {
        for (right = left + 1U; right < vertex_count; ++right) {
            matrix[left][right] = max_edge_weight();
            matrix[right][left] = max_edge_weight();
        }
    }

    for (left = 0U; left < vertex_count; ++left) {
        next = (left + 1U) % vertex_count;
        matrix[left][next] = min_edge_weight();
        matrix[next][left] = min_edge_weight();
    }

    return matrix;
}

void test_matrix_validation()
{
    const WeightMatrix matrix = make_test_matrix_small();

    assert(is_valid_matrix(matrix));
}

void test_graph_construction()
{
    const WeightMatrix matrix = make_test_matrix_small();
    const GraphData data = build_complete_graph_from_matrix(matrix);

    assert(boost::num_vertices(data.graph) == 4U);
    assert(boost::num_edges(data.graph) == 6U);
    assert(edge_weight(data.graph, 0U, 1U) == 2);
    assert(edge_weight(data.graph, 1U, 3U) == 4);
    assert(edge_weight(data.graph, 2U, 3U) == 8);
}

void test_cycle_cost()
{
    const WeightMatrix matrix = make_test_matrix_small();
    const GraphData data = build_complete_graph_from_matrix(matrix);
    const Path path = {0U, 1U, 3U, 2U, 0U};
    const int expected_cost = 23;

    assert(calculate_cycle_cost(data.graph, path) == expected_cost);
}

void test_tsp_small()
{
    const WeightMatrix matrix = make_test_matrix_small();
    const GraphData data = build_complete_graph_from_matrix(matrix);
    const int expected_cost = 23;
    const TspResult result = solve_tsp(data.graph, default_start_vertex());

    assert(result.cost == expected_cost);
    assert(is_hamiltonian_cycle(result.path, matrix.size(),
        default_start_vertex()));
}

void test_tsp_ring()
{
    const std::size_t vertex_count = default_vertex_count();
    const WeightMatrix matrix = make_test_matrix_ring(vertex_count);
    const GraphData data = build_complete_graph_from_matrix(matrix);
    const int expected_cost = static_cast<int>(vertex_count) * min_edge_weight();
    const TspResult result = solve_tsp(data.graph, default_start_vertex());

    assert(result.cost == expected_cost);
    assert(is_hamiltonian_cycle(result.path, vertex_count,
        default_start_vertex()));
}

void test_random_complete_graph()
{
    const std::size_t vertex_count = 5U;
    const unsigned int seed = 42U;
    const std::size_t expected_edges = vertex_count * (vertex_count - 1U) / 2U;
    std::default_random_engine generator(seed);
    const GraphData data = generate_random_complete_graph(vertex_count, generator);

    assert(boost::num_edges(data.graph) == expected_edges);
    assert(is_valid_matrix(data.matrix));
}

void test_all()
{
    test_matrix_validation();
    test_graph_construction();
    test_cycle_cost();
    test_tsp_small();
    test_tsp_ring();
    test_random_complete_graph();
}

} // namespace tsp

int main()
{
    const std::size_t vertex_count = tsp::default_vertex_count();
    const std::size_t start_vertex = tsp::default_start_vertex();
    std::random_device random_device = {};
    std::default_random_engine generator(random_device());
    tsp::GraphData graph_data = {tsp::Graph(0U), tsp::WeightMatrix{}};
    tsp::TspResult result = {{}, 0};

    tsp::test_all();

    graph_data = tsp::generate_random_complete_graph(vertex_count, generator);
    result = tsp::solve_tsp(graph_data.graph, start_vertex);

    std::cout << "Self-check: OK\n";
    tsp::print_result(graph_data, result);

    return 0;
}