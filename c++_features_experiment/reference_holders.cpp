

template <class T> class reference_wrapper {
  public:
    using type = T;

    // Constructor from T&
    explicit reference_wrapper(T &t) noexcept : ptr(std::addressof(t)) {}

    // Implicit conversion to T&
    operator T &() const noexcept { return *ptr; }

    // Access the underlying object
    T &get() const noexcept { return *ptr; }

  private:
    T *ptr;
};

template <class T> reference_wrapper<T> ref(T &t) noexcept {
    return reference_wrapper<T>(t);
}
template <class T> void ref(const T &&) = delete; // disable r-value cases.

template <class T> reference_wrapper<const T> cref(const T &t) noexcept {
    return reference_wrapper<const T>(t);
}
template <class T> void cref(const T &&) = delete;
