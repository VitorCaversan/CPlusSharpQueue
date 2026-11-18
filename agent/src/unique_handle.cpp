#include "unique_handle.hpp"
#include <utility>

UniqueHandle::UniqueHandle(HANDLE h) noexcept : handle_(h)
{
}

UniqueHandle::~UniqueHandle()
{
    if (is_valid(handle_))
        CloseHandle(handle_);
}

UniqueHandle::UniqueHandle(UniqueHandle&& other) noexcept :
    handle_(std::exchange(other.handle_, nullptr))
{
}

UniqueHandle& UniqueHandle::operator =(UniqueHandle&& other) noexcept
{
    reset(other.release());
    return *this;
}

HANDLE UniqueHandle::get() const noexcept
{
    return handle_;
}

HANDLE UniqueHandle::release() noexcept
{
    return std::exchange(handle_, nullptr);
}

void UniqueHandle::reset(HANDLE handle) noexcept
{
    if (this->handle_ == handle)
        return;
    
    if (is_valid(handle_))
        CloseHandle(handle_);
        
    handle_ = handle;
}

UniqueHandle::operator bool() const noexcept
{
    return is_valid(handle_);
}

bool UniqueHandle::is_valid(HANDLE h) noexcept
{
    return (h != nullptr && h != INVALID_HANDLE_VALUE);
}