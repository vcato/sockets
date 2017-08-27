#include "objectset.hpp"

#include <set>


static void testAllocatingAndDeallocating()
{
  ObjectSet<int> s;
  ObjectSet<int>::ObjectHandle h1 = s.allocate();
  s[h1] = 1;
  ObjectSet<int>::ObjectHandle h2 = s.allocate();
  s[h2] = 2;
  assert(s.allocatedCount()==2);
  s.deallocate(h1);
  assert(s.allocatedCount()==1);
  h1 = s.allocate();
  assert(s.allocatedCount()==2);
  assert(s.unallocatedCount()==0);
}


static void testForEachHandle()
{
  using ObjectHandle = ObjectSet<int>::ObjectHandle;
  ObjectSet<int> s;
  ObjectHandle h = s.allocate();
  s.allocate();
  s.allocate();
  s.deallocate(h);

  std::set<ObjectHandle> handle_set;

  s.forEachHandle([&](ObjectHandle h){
    handle_set.insert(h);
  });

  assert(handle_set.size()==2);
}


int main()
{
  testAllocatingAndDeallocating();
  testForEachHandle();
}
