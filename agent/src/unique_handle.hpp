#pragma once
#include <windows.h>

class UniqueHandle
{
public:
    UniqueHandle() noexcept = default;
    explicit UniqueHandle(HANDLE h) noexcept;
    ~UniqueHandle();

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator =(const UniqueHandle&) = delete;
    UniqueHandle(UniqueHandle&& other) noexcept;
    UniqueHandle& operator =(UniqueHandle&& other) noexcept;

    [[nodiscard]] HANDLE get() const noexcept;
    [[nodiscard]] HANDLE release() noexcept;
    void reset(HANDLE handle = nullptr) noexcept;
    explicit operator bool() const noexcept;

private:
    HANDLE handle_ = nullptr;
    static bool is_valid(HANDLE h) noexcept;
};