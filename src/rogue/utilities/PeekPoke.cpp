/**
 *-----------------------------------------------------------------------------
 * Title         : Register access peek and poke
 * ----------------------------------------------------------------------------
 * File          : PeekPoke.cpp
 * Author        : Ryan Herbst <rherbst@slac.stanford.edu>
 * Created       : 09/17/2016
 * Last update   : 09/17/2016
 *-----------------------------------------------------------------------------
 * Description :
 *    Interface to peek and poke arbitary registers.
 *-----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/
#include <rogue/utilities/PeekPoke.h>
#include <rogue/interfaces/memory/Block.h>
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

namespace ru  = rogue::utilities;
namespace rim = rogue::interfaces::memory;
namespace bp  = boost::python;

//! Class creation
ru::PeekPokePtr ru::PeekPoke::create (uint32_t size) {
   ru::PeekPokePtr p = boost::make_shared<ru::PeekPoke>(size);
   return(p);
}

//! Setup class in python
void ru::PeekPoke::setup_python() {
   bp::class_<ru::PeekPoke, bp::bases<rim::Master>, ru::PeekPokePtr, boost::noncopyable >("PeekPoke",bp::init<uint32_t>())
      .def("create",         &ru::PeekPoke::create)
      .staticmethod("create")
      .def("poke",           &ru::PeekPoke::poke)
      .def("peek",           &ru::PeekPoke::peek)
   ;
}

//! Creator 
ru::PeekPoke::PeekPoke(uint32_t size) {
   regSize_ = size;
}

//! Deconstructor
ru::PeekPoke::~PeekPoke() {}

//! Poke. Write to an address
bool ru::PeekPoke::poke ( uint32_t address, uint32_t value ) {
   rim::BlockPtr b = rim::Block::create(address,regSize_);

   if ( regSize_ == 8 ) b->setUInt8(0,value);
   else if ( regSize_ == 16 ) b->setUInt16(0,value);
   else if ( regSize_ == 32 ) b->setUInt32(0,value);

   return(reqWriteSingle(b));
}

//! Peek. Read from an address
uint32_t ru::PeekPoke::peek ( uint32_t address ) {
   uint32_t ret;

   rim::BlockPtr b = rim::Block::create(address,regSize_);

   ret = 0xFFFFFFFF;

   if ( reqReadSingle(b) ) {
      if ( regSize_ == 8 ) ret = b->getUInt8(0);
      else if ( regSize_ == 16 ) ret = b->getUInt16(0);
      else if ( regSize_ == 32 ) ret = b->getUInt32(0);
   }

   return(ret);
}

