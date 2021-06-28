#include <memory>
#include <cstdint>

template <typename T>
class unsure_ownership_ptr
{
    std::unique_ptr<T> pointer;

private:
    static T *unzip_ptr(T *address) noexcept
    {
        return reinterpret_cast<T *>(reinterpret_cast<size_t>(address) & (SIZE_MAX - 1));
    }
    static T *zip_ptr(bool is_owned, T *address) noexcept
    {
        return reinterpret_cast<T *>((!is_owned) | reinterpret_cast<size_t>(address));
    }

public:
    bool has_ownership() const noexcept
    {
        return !(reinterpret_cast<size_t>(pointer.get()) & 0x1);
    };
    T *get() const noexcept
    {
        return unzip_ptr(pointer.get());
    }
    T *release() noexcept
    {
        return unzip_ptr(pointer.release());
    }
    decltype(auto) get_deleter() noexcept
    {
        return pointer.get_deleter();
    }
    void swap(unsure_ownership_ptr &another) noexcept
    {
        pointer.swap(another.pointer);
    }
    void reset(std::unique_ptr<T> &&pointer = std::unique_ptr<T>()) noexcept
    {
        if (!this->has_ownership())
        {
            pointer.release();
        }
        pointer.reset(std::move(pointer));
    }
    void reset(unsure_ownership_ptr &&another) noexcept
    {
        *this = std::move(another);
    }
    void reset(bool is_owned, T *pointer) noexcept
    {
        if (!has_ownership())
        {
            pointer.release();
        }
        pointer.reset(zip_ptr(is_owned, pointer));
    }

    unsure_ownership_ptr() : pointer() {}
    unsure_ownership_ptr(const unsure_ownership_ptr &another) = delete;
    unsure_ownership_ptr(unsure_ownership_ptr &&another) : pointer(std::move(another.pointer)) {}
    unsure_ownership_ptr(bool is_owned, T *pointer) : pointer(zip_ptr(is_owned, pointer)) {}
    unsure_ownership_ptr(std::unique_ptr<T> &&pointer) : pointer(std::move(pointer)) {}

    unsure_ownership_ptr &operator=(const unsure_ownership_ptr &another) = delete;
    unsure_ownership_ptr &operator=(unsure_ownership_ptr &&another) noexcept
    {
        if (!has_ownership())
        {
            pointer.release();
        }
        pointer = std::move(another.pointer);
        return *this;
    }

    T *operator->() const noexcept
    {
        return get();
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    T &operator*() const
    {
        return *get();
    }

    ~unsure_ownership_ptr()
    {
        if (!has_ownership())
        {
            pointer.release();
        }
    }
};
