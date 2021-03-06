/** @file

  Fast-Allocators

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Provides three classes
    - Allocator for allocating memory blocks of fixed size
    - ClassAllocator for allocating objects
    - SpaceClassAllocator for allocating sparce objects (most members uninitialized)

  These class provides a efficient way for handling dynamic allocation.
  The fast allocator maintains its own freepool of objects from
  which it doles out object. Allocated objects when freed go back
  to the free pool.

  @note Fast allocators could accumulate a lot of objects in the
  free pool as a result of bursty demand. Memory used by the objects
  in the free pool never gets freed even if the freelist grows very
  large.

 */

#ifndef _Allocator_h_
#define _Allocator_h_

#include <stdlib.h>
#include "ink_queue.h"
#include "ink_defs.h"
#include "ink_resource.h"

#define RND16(_x)               (((_x)+15)&~15)

/** Allocator for fixed size memory blocks. */
class Allocator
{
public:
  /**
    Allocate a block of memory (size specified during construction
    of Allocator.
  */
  void *
  alloc_void()
  {
    return ink_freelist_new(this->fl);
  }

  /** Deallocate a block of memory allocated by the Allocator. */
  void
  free_void(void *ptr)
  {
    ink_freelist_free(this->fl, ptr);
  }

  Allocator()
  {
    fl = NULL;
  }

  /**
    Creates a new allocator.

    @param name identification tag used for mem tracking .
    @param element_size size of memory blocks to be allocated.
    @param chunk_size number of units to be allocated if free pool is empty.
    @param alignment of objects must be a power of 2.
  */
  Allocator(const char *name, unsigned int element_size,
            unsigned int chunk_size = 128, unsigned int alignment = 8)
  {
    ink_freelist_init(&fl, name, element_size, chunk_size, alignment);
  }

  /** Re-initialize the parameters of the allocator. */
  void
  re_init(const char *name, unsigned int element_size,
          unsigned int chunk_size, unsigned int alignment)
  {
    ink_freelist_init(&this->fl, name, element_size, chunk_size, alignment);
  }

protected:
  InkFreeList *fl;
};

/**
  Allocator for Class objects. It uses a prototype object to do
  fast initialization. Prototype of the template class is created
  when the fast allocator is created. This is instantiated with
  default (no argument) constructor. Constructor is not called for
  the allocated objects. Instead, the prototype is just memory
  copied onto the new objects. This is done for performance reasons.

*/
template<class C> class ClassAllocator: public Allocator {
public:
  /** Allocates objects of the templated type. */
  C*
  alloc()
  {
    void *ptr = ink_freelist_new(this->fl);

    memcpy(ptr, (void *)&this->proto.typeObject, sizeof(C));
    return (C *) ptr;
  }

  /**
    Deallocates objects of the templated type.

    @param ptr pointer to be freed.
  */
  void
  free(C * ptr)
  {
    ink_freelist_free(this->fl, ptr);
  }

  /**
    Allocate objects of the templated type via the inherited interface
    using void pointers.
  */
  void*
  alloc_void()
  {
    return (void *) alloc();
  }

  /**
    Deallocate objects of the templated type via the inherited
    interface using void pointers.

    @param ptr pointer to be freed.
  */
  void
  free_void(void *ptr)
  {
    free((C *) ptr);
  }

  /**
    Create a new class specific ClassAllocator.

    @param name some identifying name, used for mem tracking purposes.
    @param chunk_size number of units to be allocated if free pool is empty.
    @param alignment of objects must be a power of 2.
  */
  ClassAllocator(const char *name, unsigned int chunk_size = 128,
                 unsigned int alignment = 16)
  {
    ink_freelist_init(&this->fl, name, RND16(sizeof(C)), chunk_size, RND16(alignment));
  }

  struct
  {
    C typeObject;
    int64_t space_holder;
  } proto;
};

/**
  Allocator for space class, a class with a lot of uninitialized
  space/members. It uses an instantiate fucntion do initialization
  of objects. This is particulary useful if most of the space in
  the objects does not need to be intialized. The inifunction passed
  can be used to intialize a few fields selectively. Using
  ClassAllocator for space objects would unnecessarily initialized
  all of the members.

*/
template<class C> class SparceClassAllocator:public ClassAllocator<C> {
public:

  /** Allocates objects of the templated type. */
  C*
  alloc()
  {
    void *ptr = ink_freelist_new(this->fl);

    if (!_instantiate) {
      memcpy(ptr, (void *)&this->proto.typeObject, sizeof(C));
    } else
      (*_instantiate) ((C *) &this->proto.typeObject, (C *) ptr);
    return (C *) ptr;
  }


  /**
    Create a new class specific SparceClassAllocator.

    @param name some identifying name, used for mem tracking purposes.
    @param chunk_size number of units to be allocated if free pool is empty.
    @param alignment of objects must be a power of 2.
    @param instantiate_func

  */
  SparceClassAllocator(const char *name, unsigned int chunk_size = 128,
                       unsigned int alignment = 16,
                       void (*instantiate_func) (C * proto, C * instance) = NULL)
    : ClassAllocator<C>(name, chunk_size, alignment)
  {
    _instantiate = instantiate_func;       // NULL by default
  }

private:
  void (*_instantiate) (C* proto, C* instance);
};

#endif  // _Allocator_h_
