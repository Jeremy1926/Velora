#pragma once

#include "HAL/UnrealMemory.h"
#include "Templates/UnrealTemplate.h"

template<typename T>
class TQueue
{
public:
    typedef T FElementType;

    TQueue()
    {
        Head = Tail = new(FMemory::Realloc(nullptr, sizeof(TNode))) TNode();
    }

    ~TQueue()
    {
        while (Tail)
        {
            TNode* Node = Tail;
            Tail = Tail->NextNode;

            Node->~TNode();
            FMemory::Free(Node);
        }
    }

    bool Dequeue(FElementType& OutItem)
    {
        TNode* Popped = Tail->NextNode;

        if (!Popped)
            return false;

        OutItem = MoveTemp(Popped->Item);

        TNode* OldTail = Tail;
        Tail = Popped;
        Tail->Item = FElementType();

        OldTail->~TNode();
        FMemory::Free(OldTail);

        return true;
    }

    bool Enqueue(const FElementType& Item)
    {
        TNode* NewNode = new(FMemory::Realloc(nullptr, sizeof(TNode))) TNode(Item);
        TNode* OldHead = Head;

        Head = NewNode;
        OldHead->NextNode = NewNode;

        return true;
    }

    bool IsEmpty() const
    {
        return !Tail->NextNode;
    }

private:
    struct TNode
    {
        TNode()
            : NextNode(nullptr)
        {
        }

        explicit TNode(const FElementType& InItem)
            : NextNode(nullptr)
            , Item(InItem)
        {
        }

        TNode* volatile NextNode;
        FElementType Item;
    };

    alignas(16) TNode* volatile Head;
    TNode* Tail;
};
