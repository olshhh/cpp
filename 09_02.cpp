#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>

class Tree {
public:
    struct Node {
        int value{0};
        std::shared_ptr<Node> left{};
        std::shared_ptr<Node> right{};
        std::weak_ptr<Node> parent{};

        explicit Node(const int node_value) noexcept : value(node_value) {
        }

        ~Node() noexcept {
            std::cout << "Node::~Node value=" << value << '\n';
        }
    };

    std::shared_ptr<Node> root{};

    void traverse_v1(std::ostream& os = std::cout) const {
        std::queue<std::shared_ptr<Node>> nodes;
        std::shared_ptr<Node> current = nullptr;
        bool first = true;

        if (root == nullptr) {
            os << '\n';
            return;
        }

        nodes.push(root);
        while (!nodes.empty()) {
            current = nodes.front();
            nodes.pop();

            if (!first) {
                os << ' ';
            }
            os << current->value;
            first = false;

            if (current->left != nullptr) {
                nodes.push(current->left);
            }
            if (current->right != nullptr) {
                nodes.push(current->right);
            }
        }

        os << '\n';
    }

    void traverse_v2(std::ostream& os = std::cout) const {
        bool first = true;

        traverse_dfs(root, os, first);
        os << '\n';
    }

private:
    static void traverse_dfs(
        const std::shared_ptr<Node>& node,
        std::ostream& os,
        bool& first) {
        if (node == nullptr) {
            return;
        }

        if (!first) {
            os << ' ';
        }
        os << node->value;
        first = false;

        traverse_dfs(node->left, os, first);
        traverse_dfs(node->right, os, first);
    }
};

Tree BuildDemoTree() {
    Tree tree;
    std::shared_ptr<Tree::Node> root = nullptr;
    std::shared_ptr<Tree::Node> left = nullptr;
    std::shared_ptr<Tree::Node> right = nullptr;

    tree.root = std::make_shared<Tree::Node>(1);
    root = tree.root;

    root->left = std::make_shared<Tree::Node>(2);
    root->right = std::make_shared<Tree::Node>(3);
    left = root->left;
    right = root->right;

    left->parent = root;
    right->parent = root;

    left->left = std::make_shared<Tree::Node>(4);
    left->right = std::make_shared<Tree::Node>(5);
    right->left = std::make_shared<Tree::Node>(6);
    right->right = std::make_shared<Tree::Node>(7);

    left->left->parent = left;
    left->right->parent = left;
    right->left->parent = right;
    right->right->parent = right;

    return tree;
}

void TestTreeStructure() {
    Tree tree = BuildDemoTree();

    assert(tree.root != nullptr);
    assert(tree.root->value == 1);
    assert(tree.root->left != nullptr);
    assert(tree.root->right != nullptr);
    assert(tree.root->left->value == 2);
    assert(tree.root->right->value == 3);
    assert(tree.root->left->parent.lock().get() == tree.root.get());
    assert(tree.root->right->parent.lock().get() == tree.root.get());
}

void TestTraverseBreadthFirst() {
    Tree tree = BuildDemoTree();
    std::stringstream buffer;

    tree.traverse_v1(buffer);
    assert(buffer.str() == "1 2 3 4 5 6 7\n");
}

void TestTraverseDepthFirst() {
    Tree tree = BuildDemoTree();
    std::stringstream buffer;

    tree.traverse_v2(buffer);
    assert(buffer.str() == "1 2 4 5 3 6 7\n");
}

void TestNoOwnershipCycle() {
    std::weak_ptr<Tree::Node> weak_root;
    std::weak_ptr<Tree::Node> weak_leaf;

    {
        Tree tree = BuildDemoTree();

        weak_root = tree.root;
        weak_leaf = tree.root->left->left;

        assert(!weak_root.expired());
        assert(!weak_leaf.expired());
    }

    assert(weak_root.expired());
    assert(weak_leaf.expired());
}

void RunTests() {
    TestTreeStructure();
    TestTraverseBreadthFirst();
    TestTraverseDepthFirst();
    TestNoOwnershipCycle();
}

int main() {
    RunTests();

    std::cout << "BFS: ";
    {
        Tree tree = BuildDemoTree();

        tree.traverse_v1();

        std::cout << "DFS: ";
        tree.traverse_v2();

        std::cout << "left parent value="
                  << tree.root->left->parent.lock()->value
                  << '\n';
    }

    std::cout << "All nodes were destroyed without ownership cycles.\n";
    return 0;
}