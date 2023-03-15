#pragma once


struct RgbHandler
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    struct Allocator
    {
        virtual void returnBuffer(RgbHandler &handler) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    RgbHandler() = default;

    RgbHandler(uint8_t *start, Allocator *allocator) : mStart(start), mAllocator(allocator) {}

    void returnBuffer()
    {
        if (mAllocator)
        {
            mAllocator->returnBuffer(*this);
        }
        mStart = nullptr;
        mSize = 0;
        mIsTick = false;
        mAllocator = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FIELDS
    ///////////////////////////////////////////////////////////////////////////

    uint8_t *mStart{nullptr};
    size_t mSize{0U};
    bool mIsTick{false};
    Allocator *mAllocator{nullptr};
};
