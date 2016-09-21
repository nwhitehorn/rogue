/**
 *-----------------------------------------------------------------------------
 * Title      : RCE Memory Mapped Access
 * ----------------------------------------------------------------------------
 * File       : RceMemory.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Class for interfacing to RCE mapped memory space.
 * ----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 * ----------------------------------------------------------------------------
**/
#include <rogue/hardware/rce/RceMemory.h>
#include <rogue/interfaces/memory/Block.h>
#include <rogue/interfaces/memory/BlockVector.h>
#include <boost/make_shared.hpp>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

namespace rhr = rogue::hardware::rce;
namespace rim = rogue::interfaces::memory;
namespace bp  = boost::python;

//! Class creation
rhr::RceMemoryPtr rhr::RceMemory::create () {
   rhr::RceMemoryPtr r = boost::make_shared<rhr::RceMemory>();
   return(r);
}

//! Creator
rhr::RceMemory::RceMemory() {
   fd_      = -1;
}

//! Destructor
rhr::RceMemory::~RceMemory() {
   this->close();
}

//! Open the device. Pass destination.
bool rhr::RceMemory::open ( ) {
   bool ret;

   ret = true;

   mapMtx_.lock();

   if ( fd_ > 0 ) ret = false;
   else {
      fd_ = ::open("/dev/mem", O_RDWR | O_SYNC);
      if ( fd_ > 0 ) ret = true;
   }
   mapMtx_.unlock();
   return(ret);
}

//! Close the device
void rhr::RceMemory::close() {
   uint32_t x;

   mapMtx_.lock();
   if ( fd_ < 0 ) return;

   for ( x=0; x < maps_.size(); x++ )
      munmap(maps_[x].ptr,maps_[x].size);

   maps_.clear();
   mapMtx_.unlock();

   ::close(fd_);
}

//! Add a memory space
void rhr::RceMemory::addMap(uint32_t address, uint32_t size) {
   rhr::RceMemoryMap map;

   map.base = address;
   map.size = size;

   mapMtx_.lock();

   if ( fd_ >= 0 ) {

      map.ptr = (uint8_t *)mmap(NULL, map.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, map.base);

      if ( map.ptr == NULL ) return;

      maps_.push_back(map);
   }
   mapMtx_.unlock();
}

// Find matching address space, lock before use
uint8_t * rhr::RceMemory::findSpace (uint32_t base, uint32_t size) {
   uint32_t x;

   for (x=0; x < maps_.size(); x++) {
      if ( (base > maps_[x].base) && (((base - maps_[x].base) + size) < maps_[x].size) ) {
         return(maps_[x].ptr + (base-maps_[x].base));
      }
   }
   return(NULL);
}

//! Issue a set of write transactions
bool rhr::RceMemory::doWrite (boost::shared_ptr<rogue::interfaces::memory::BlockVector> blocks) {
   rim::BlockPtr b;
   uint8_t *     ptr;
   uint32_t      x;
   bool          ret;

   ret = true;

   for(x=0; x < blocks->count(); x++) {
      b = blocks->getBlock(x);

      if ((ptr = findSpace(b->getAddress(),b->getSize())) == NULL) {
         ret = false;
         b->setError(1);
      }
      else {
         memcpy(ptr,b->getData(),b->getSize());
         b->setStale(false);
      }
   }
   return(ret);
}

//! Issue a set of read transactions
bool rhr::RceMemory::doRead  (boost::shared_ptr<rogue::interfaces::memory::BlockVector> blocks) {
   rim::BlockPtr b;
   uint8_t *     ptr;
   uint32_t      x;
   bool          ret;

   ret = true;

   for(x=0; x < blocks->count(); x++) {
      b = blocks->getBlock(x);

      if ((ptr = findSpace(b->getAddress(),b->getSize())) == NULL) {
         ret = false;
         b->setError(1);
      }
      else {
         memcpy(b->getData(),ptr,b->getSize());
         b->setStale(false);
      }
   }
   return(ret);
}

void rhr::RceMemory::setup_python () {

   bp::class_<rhr::RceMemory, bp::bases<rim::Slave>, rhr::RceMemoryPtr, boost::noncopyable >("RceMemory",bp::init<>())
      .def("create",         &rhr::RceMemory::create)
      .staticmethod("create")
      .def("open",           &rhr::RceMemory::open)
      .def("close",          &rhr::RceMemory::close)
      .def("addMap",         &rhr::RceMemory::addMap)
   ;
}

