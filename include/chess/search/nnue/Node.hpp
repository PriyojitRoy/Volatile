#ifndef CHESS_SEARCH_NODE_H
#define CHESS_SEARCH_NODE_H

#include <vector>
#include <cmath>
#include "chess/core/Move.hpp"

namespace ChessCore {

    class Node {
    public:
        Move move;
        Node* parent;
        std::vector<Node*> children;

        float visits;
        float value;
        float priority;

        Node(Move m = Move(), Node* p = nullptr);
        ~Node();

        bool isLeaf() const;
        bool isExpanded() const;
        
        Node* getBestChild() const;
        float getUctScore(float totalVisits) const;

        void addChild(Move m);
    };
}

#endif // CHESS_SEARCH_NODE_H