#include "chess/search/nnue/Node.hpp"
#include <algorithm>
#include <limits>

namespace ChessCore {

    Node::Node(Move m, Node* p) {
        move = m;
        parent = p;
        visits = 0.0f;
        value = 0.0f;
        priority = 0.0f;
        children.clear();
    }

    Node::~Node() {
        for (Node* child : children) delete child;
    }

    bool Node::isLeaf() const { return children.empty(); }
    bool Node::isExpanded() const { return !children.empty(); }

    void Node::addChild(Move m) {
        children.push_back(new Node(m, this));
    }

    Node* Node::getBestChild() const {
        Node* best = nullptr;
        float maxVisits = -1.0f;

        for (Node* child : children) {
            // Choose most visited, not highest value
            if (child->visits > maxVisits) {
                maxVisits = child->visits;
                best = child;
            }
        }
        return best;
    }
}