/**
 *-----------------------------------------------------------------------------
 * Title      : RSSI Transport Port
 * ----------------------------------------------------------------------------
 * File       : Transport.h
 * Created    : 2017-01-07
 * Last update: 2017-01-07
 * ----------------------------------------------------------------------------
 * Description:
 * RSSI Transport Port
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
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/Buffer.h>
#include <rogue/protocols/rssi/Controller.h>
#include <rogue/protocols/rssi/Transport.h>
#include <rogue/GeneralError.h>
#include <boost/make_shared.hpp>
#include <rogue/GilRelease.h>
#include <rogue/Logging.h>

namespace rpr = rogue::protocols::rssi;
namespace ris = rogue::interfaces::stream;
namespace bp  = boost::python;

//! Class creation
rpr::TransportPtr rpr::Transport::create () {
   rpr::TransportPtr r = boost::make_shared<rpr::Transport>();
   return(r);
}

void rpr::Transport::setup_python() {

   bp::class_<rpr::Transport, rpr::TransportPtr, bp::bases<ris::Master,ris::Slave>, boost::noncopyable >("Transport",bp::init<>());

   bp::implicitly_convertible<rpr::TransportPtr, ris::MasterPtr>();
   bp::implicitly_convertible<rpr::TransportPtr, ris::SlavePtr>();
}

//! Creator
rpr::Transport::Transport () { }

//! Destructor
rpr::Transport::~Transport() { }

//! Setup links
void rpr::Transport::setController( rpr::ControllerPtr cntl ) {
   cntl_ = cntl;
   rxQueue_.setMax(4);

   // Start read thread
   thread_ = new boost::thread(boost::bind(&rpr::Transport::runThread, this));
}

//! Accept a frame from master
void rpr::Transport::acceptFrame ( ris::FramePtr frame ) {
   //cntl_->transportRx(frame);
   rxQueue_.push(frame);
}

//! Thread background
void rpr::Transport::runThread() {
   Logging log("rssi.Transport");
   log.logThreadId(rogue::Logging::Info);

   try {
      while(1) {
         cntl_->transportRx(rxQueue_.pop());
      }
   } catch (boost::thread_interrupted&) { }
}

