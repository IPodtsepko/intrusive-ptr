#pragma once

#include <atomic>

template <typename T>
struct intrusive_ptr {
  template <typename Y>
  friend struct intrusive_ptr;

  using element_type = T;

  intrusive_ptr() noexcept = default;

  intrusive_ptr(T* p, bool add_ref = true) : m_pointer(p) {
    if (m_pointer && add_ref) {
      intrusive_ptr_add_ref(p);
    }
  }

  intrusive_ptr(intrusive_ptr const& r) : m_pointer(r.get()) {
    if (m_pointer) {
      intrusive_ptr_add_ref(m_pointer);
    }
  }

  template <class Y>
  intrusive_ptr(intrusive_ptr<Y> const& r)
      : m_pointer(static_cast<element_type*>(r.get())) {
    if (m_pointer) {
      intrusive_ptr_add_ref(m_pointer);
    }
  }

  intrusive_ptr(intrusive_ptr&& r) : m_pointer(r.get()) {
    r.m_pointer = nullptr;
  }

  template <class Y>
  intrusive_ptr(intrusive_ptr<Y>&& r)
      : m_pointer(static_cast<element_type*>(r.get())) {
    r.m_pointer = nullptr;
  }

  ~intrusive_ptr() {
    if (m_pointer) {
      intrusive_ptr_release(m_pointer);
    }
  }

  intrusive_ptr& operator=(intrusive_ptr const& r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }

  template <class Y>
  intrusive_ptr& operator=(intrusive_ptr<Y> const& r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(T* r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(intrusive_ptr&& r) {
    intrusive_ptr(std::move(r)).swap(*this);
    return *this;
  }

  template <class Y>
  intrusive_ptr& operator=(intrusive_ptr<Y>&& r) {
    intrusive_ptr(std::move(r)).swap(*this);
    return *this;
  }

  void reset() {
    intrusive_ptr().swap(*this);
  }

  void reset(T* r) {
    intrusive_ptr(r).swap(*this);
  }

  void reset(T* r, bool add_ref) {
    intrusive_ptr(r, add_ref).swap(*this);
  }

  T& operator*() const noexcept {
    return *m_pointer;
  }

  T* operator->() const noexcept {
    return m_pointer;
  }

  T* get() const noexcept {
    return m_pointer;
  }

  T* detach() noexcept {
    auto pointer = m_pointer;
    m_pointer = nullptr;
    return pointer;
  }

  explicit operator bool() const noexcept {
    return get() != nullptr;
  }

  void swap(intrusive_ptr& b) noexcept {
    std::swap(m_pointer, b.m_pointer);
  }

private:
  element_type* m_pointer = nullptr;
};

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
  return a.get() == b.get();
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
  return a.get() != b.get();
}

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, U* b) noexcept {
  return a.get() == b;
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, U* b) noexcept {
  return a.get() != b;
}

template <class T, class U>
bool operator==(T* a, intrusive_ptr<U> const& b) noexcept {
  return a == b.get();
}

template <class T, class U>
bool operator!=(T* a, intrusive_ptr<U> const& b) noexcept {
  return a != b.get();
}

template <class T>
bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept {
  return a.get() < b.get();
}

template <class T>
bool operator>(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept {
  return a.get() > b.get();
}

template <class T>
void swap(intrusive_ptr<T>& a, intrusive_ptr<T>& b) noexcept {
  a.swap(b);
}

template <typename T>
struct intrusive_ref_counter {

  intrusive_ref_counter() noexcept = default;
  intrusive_ref_counter(const intrusive_ref_counter&) noexcept {};

  intrusive_ref_counter& operator=(const intrusive_ref_counter& v) noexcept {}

  unsigned int use_count() const noexcept {
    return m_reference_count;
  }

  template <typename Derived>
  friend void
  intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept;

  template <typename Derived>
  friend void
  intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept;

protected:
  ~intrusive_ref_counter() = default;

private:
  mutable std::atomic<unsigned int> m_reference_count = 0;
};

template <class Derived>
void intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept {
  p->m_reference_count.fetch_add(1, std::memory_order::memory_order_relaxed);
}

template <class Derived>
void intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept {
  if (p->m_reference_count.fetch_sub(1, std::memory_order::memory_order_acq_rel) == 1) {
    delete static_cast<const Derived*>(p);
  }
}
