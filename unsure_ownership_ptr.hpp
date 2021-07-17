#include <memory>
#include <cstdint>
#include <cassert>

template <typename T>
class unsure_ownership_ptr
{
    std::unique_ptr<T> pointer;

#ifndef __x86_64__
    bool ownership_flag;
#endif

private:
    //in x86_64/arm64, the 48th-63th bits should equals to 47th bit
    //by this, we can hidden the ownership in the 63th bit
#ifdef __x86_64__
    static bool get_47th_bit(T *address) noexcept
    {
        static_assert(sizeof(size_t) == 8); //check if is 64bit platform
        return reinterpret_cast<size_t>(address) & (0x0000800000000000);
    }
#endif
    static T *unzip_ptr(T *address) noexcept
    {
#ifdef __x86_64__
        return reinterpret_cast<T *>(((static_cast<size_t>(get_47th_bit(address)) << 63) | (SIZE_MAX >> 1)) & reinterpret_cast<size_t>(address));
#else
        return address;
#endif
    }
#ifdef __x86_64__ 
    static T *zip_ptr(bool is_owned, T *address) noexcept
    {
        return reinterpret_cast<T *>((static_cast<size_t>(!is_owned) << 63) ^ reinterpret_cast<size_t>(address));
    }
#else
    T *zip_ptr(bool is_owned, T *address) noexcept
    {
        ownership_flag=is_owned;
        return address;
    }
#endif

    // if the pointers' 48-63th bits no equals to 47th bit,this func will throw error in debug mode
    static bool check_ptr(T *address) noexcept
    {
#ifdef __x86_64__
        for (int i = 48; i < 64; ++i)
        {
            if ((reinterpret_cast<size_t>(address) & ((static_cast<size_t>(1) << i))) != get_47th_bit(address))
                return false;
        }
#endif
        return true;
    }

public:

    bool has_ownership() const noexcept
    {
#ifdef __x86_64__
        return !((reinterpret_cast<size_t>(pointer.get()) & 0x8000000000000000) ^ get_47th_bit(pointer.get()));
#else 
        return ownership_flag;
#endif
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
    void reset(std::unique_ptr<T> &&another_pointer = std::unique_ptr<T>()) noexcept
    {
        assert(check_ptr(another_pointer.get()));

        if (!pointer.has_ownership())
        {   
            pointer.release();
        }
        pointer.reset(std::move(another_pointer));
    }
    void reset(unsure_ownership_ptr &&another) noexcept
    {
        *this = std::move(another);
    }
    void reset(bool is_owned, T *another_pointer) noexcept
    {
        assert(check_ptr(another_pointer));

        if (!has_ownership())
        {
            pointer.release();
        }
        pointer.reset(zip_ptr(is_owned, another_pointer));
    }

    unsure_ownership_ptr() : pointer() {}
    unsure_ownership_ptr(const unsure_ownership_ptr &another) = delete;
    unsure_ownership_ptr(unsure_ownership_ptr &&another) : pointer(std::move(another.pointer)) {}
    unsure_ownership_ptr(bool is_owned, T *another_pointer) : pointer(zip_ptr(is_owned, another_pointer))
    {
        assert(check_ptr(another_pointer));
    }
    unsure_ownership_ptr(std::unique_ptr<T> &&another_pointer) : pointer(std::move(another_pointer))
    {
        assert(check_ptr(another_pointer.get()));
    }

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
