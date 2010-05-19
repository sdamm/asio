//
// detail/impl/pipe_select_interrupter.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_PIPE_SELECT_INTERRUPTER_IPP
#define ASIO_DETAIL_IMPL_PIPE_SELECT_INTERRUPTER_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/push_options.hpp"

#include "asio/detail/pipe_select_interrupter.hpp"

#if !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include "asio/detail/push_options.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/throw_exception.hpp>
#include "asio/detail/pop_options.hpp"

#include "asio/error.hpp"
#include "asio/system_error.hpp"

namespace asio {
namespace detail {

pipe_select_interrupter::pipe_select_interrupter()
{
  int pipe_fds[2];
  if (pipe(pipe_fds) == 0)
  {
    read_descriptor_ = pipe_fds[0];
    ::fcntl(read_descriptor_, F_SETFL, O_NONBLOCK);
    write_descriptor_ = pipe_fds[1];
    ::fcntl(write_descriptor_, F_SETFL, O_NONBLOCK);
  }
  else
  {
    asio::error_code ec(errno,
        asio::error::get_system_category());
    asio::system_error e(ec, "pipe_select_interrupter");
    boost::throw_exception(e);
  }
}

pipe_select_interrupter::~pipe_select_interrupter()
{
  if (read_descriptor_ != -1)
    ::close(read_descriptor_);
  if (write_descriptor_ != -1)
    ::close(write_descriptor_);
}

void pipe_select_interrupter::interrupt()
{
  char byte = 0;
  int result = ::write(write_descriptor_, &byte, 1);
  (void)result;
}

bool pipe_select_interrupter::reset()
{
  for (;;)
  {
    char data[1024];
    int bytes_read = ::read(read_descriptor_, data, sizeof(data));
    if (bytes_read < 0 && errno == EINTR)
      continue;
    bool was_interrupted = (bytes_read > 0);
    while (bytes_read == sizeof(data))
      bytes_read = ::read(read_descriptor_, data, sizeof(data));
    return was_interrupted;
  }
}

} // namespace detail
} // namespace asio

#endif // !defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#include "asio/detail/pop_options.hpp"

#endif // ASIO_DETAIL_IMPL_PIPE_SELECT_INTERRUPTER_IPP