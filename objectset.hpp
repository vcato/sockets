#ifndef OBJECTSET_HPP_
#define OBJECTSET_HPP_

#include <vector>
#include <memory>
#include <cassert>


using ObjectSetHandle = size_t;

template <typename T>
struct ObjectSet {
  std::vector<std::unique_ptr<T> > object_ptrs;
  using ObjectHandle = ObjectSetHandle;

  ObjectHandle allocate()
  {
    for (size_t i=0, n=object_ptrs.size(); i!=n; ++i) {
      if (!object_ptrs[i]) {
        object_ptrs[i] = std::make_unique<T>();
        return i;
      }
    }

    object_ptrs.push_back(std::make_unique<T>());
    return object_ptrs.size()-1;
  }

  size_t allocatedCount() const
  {
    size_t count = 0;

    for (size_t i=0, n=object_ptrs.size(); i!=n; ++i) {
      if (object_ptrs[i]) {
        ++count;
      }
    }

    return count;
  }

  size_t unallocatedCount() const
  {
    return object_ptrs.size()-allocatedCount();
  }

  void deallocate(ObjectHandle i)
  {
    assert(object_ptrs[i]);
    object_ptrs[i].reset();
    assert(!object_ptrs[i]);
  }

  T& operator[](ObjectHandle i)
  {
    assert(i<object_ptrs.size());
    assert(object_ptrs[i]);
    return *object_ptrs[i];
  }

  template <typename F>
  void forEachHandle(const F& f) const
  {
    for (size_t i=0, n=object_ptrs.size(); i!=n; ++i) {
      if (object_ptrs[i]) {
        f(i);
      }
    }
  }
};

#endif /* OBJECTSET_HPP_ */
