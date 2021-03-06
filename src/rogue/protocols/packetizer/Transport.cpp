/**
 *-----------------------------------------------------------------------------
 * Title      : Packetizer Transport Port
 * ----------------------------------------------------------------------------
 * File       : Transport.h
 * Created    : 2017-01-07
 * Last update: 2017-01-07
 * ----------------------------------------------------------------------------
 * Description:
 * Packetizer Transport Port
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
#include <rogue/protocols/packetizer/Controller.h>
#include <rogue/protocols/packetizer/Transport.h>
#include <rogue/GeneralError.h>
#include <boost/make_shared.hpp>
#include <rogue/GilRelease.h>
#include <rogue/Logging.h>

namespace rpp = rogue::protocols::packetizer;
namespace ris = rogue::interfaces::stream;

#ifndef NO_PYTHON
#include <boost/python.hpp>
namespace bp  = boost::python;
#endif

//! Class creation
rpp::TransportPtr rpp::Transport::create () {
   rpp::TransportPtr r = boost::make_shared<rpp::Transport>();
   return(r);
}

void rpp::Transport::setup_python() {
#ifndef NO_PYTHON

   bp::class_<rpp::Transport, rpp::TransportPtr, bp::bases<ris::Master,ris::Slave>, boost::noncopyable >("Transport",bp::init<>());

   bp::implicitly_convertible<rpp::TransportPtr, ris::MasterPtr>();
   bp::implicitly_convertible<rpp::TransportPtr, ris::SlavePtr>();
#endif
}

//! Creator
rpp::Transport::Transport () { }

//! Destructor
rpp::Transport::~Transport() { 
   thread_->interrupt();
   thread_->join();
}

//! Setup links
void rpp::Transport::setController( rpp::ControllerPtr cntl ) {
   cntl_ = cntl;

   // Start read thread
   thread_ = new boost::thread(boost::bind(&rpp::Transport::runThread, this));
}

//! Accept a frame from master
void rpp::Transport::acceptFrame ( ris::FramePtr frame ) {
   cntl_->transportRx(frame);
}

//! Thread background
void rpp::Transport::runThread() {
   Logging log("packetizer.Transport");
   log.logThreadId(rogue::Logging::Info);

   try {
      while(1) {
         sendFrame(cntl_->transportTx());
      }
   } catch (boost::thread_interrupted&) { }
}

