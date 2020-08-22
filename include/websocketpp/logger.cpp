/*
 * Copyright (c) 2014, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WEBSOCKETPP_LOGGER_BASIC_HPP
#define WEBSOCKETPP_LOGGER_BASIC_HPP

/* Need a way to print a message to the log
 *
 * - timestamps
 * - channels
 * - thread safe
 * - output to stdout or file
 * - selective output channels, both compile time and runtime
 * - named channels
 * - ability to test whether a log message will be printed at compile time
 *
 */


#include <websocketpp/common/cpp11.hpp>
#include <websocketpp/common/time.hpp>

#include <stdint.h>
#include <misc/syslog.h>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>

namespace logger {


/// Type of a channel package
typedef uint32_t level;

/// Package of values for hinting at the nature of a given logger.
/**
 * Used by the library to signal to the logging class a hint that it can use to
 * set itself up. For example, the `access` hint indicates that it is an access
 * log that might be suitable for being printed to an access log file or to cout
 * whereas `error` might be suitable for an error log file or cerr. 
 */
struct channel_type_hint {
    /// Type of a channel type hint value
    typedef uint32_t value;
    
    /// No information
    static value const none = 0;
    /// Access log
    static value const access = 1;
    /// Error log
    static value const error = 2;
};

/// Package of log levels for logging errors
struct elevel {
    /// Special aggregate value representing "no levels"
    static level const none = 0x0;
    /// Low level debugging information (warning: very chatty)
    static level const devel = 0x1;
    /// Information about unusual system states or other minor internal library
    /// problems, less chatty than devel.
    static level const library = 0x2;
    /// Information about minor configuration problems or additional information
    /// about other warnings.
    static level const info = 0x4;
    /// Information about important problems not severe enough to terminate
    /// connections.
    static level const warn = 0x8;
    /// Recoverable error. Recovery may mean cleanly closing the connection with
    /// an appropriate error code to the remote endpoint.
    static level const rerror = 0x10;
    /// Unrecoverable error. This error will trigger immediate unclean
    /// termination of the connection or endpoint.
    static level const fatal = 0x20;
    /// Special aggregate value representing "all levels"
    static level const all = 0xffffffff;

    /// Get the textual name of a channel given a channel id
    /**
     * The id must be that of a single channel. Passing an aggregate channel
     * package results in undefined behavior.
     *
     * @param channel The channel id to look up.
     *
     * @return The name of the specified channel.
     */
    static char const * channel_name(level channel) {
        switch(channel) {
            case devel:
                return "devel";
            case library:
                return "library";
            case info:
                return "info";
            case warn:
                return "warning";
            case rerror:
                return "error";
            case fatal:
                return "fatal";
            default:
                return "unknown";
        }
    }
};

/// Package of log levels for logging access events
struct alevel {
    /// Special aggregate value representing "no levels"
    static level const none = 0x0;
    /// Information about new connections
    /**
     * One line for each new connection that includes a host of information
     * including: the remote address, websocket version, requested resource,
     * http code, remote user agent
     */
    static level const connect = 0x1;
    /// One line for each closed connection. Includes closing codes and reasons.
    static level const disconnect = 0x2;
    /// One line per control frame
    static level const control = 0x4;
    /// One line per frame, includes the full frame header
    static level const frame_header = 0x8;
    /// One line per frame, includes the full message payload (warning: chatty)
    static level const frame_payload = 0x10;
    /// Reserved
    static level const message_header = 0x20;
    /// Reserved
    static level const message_payload = 0x40;
    /// Reserved
    static level const endpoint = 0x80;
    /// Extra information about opening handshakes
    static level const debug_handshake = 0x100;
    /// Extra information about closing handshakes
    static level const debug_close = 0x200;
    /// Development messages (warning: very chatty)
    static level const devel = 0x400;
    /// Special channel for application specific logs. Not used by the library.
    static level const app = 0x800;
    /// Access related to HTTP requests
    static level const http = 0x1000;
    /// One line for each failed WebSocket connection with details
    static level const fail = 0x2000;
    /// Aggregate package representing the commonly used core access channels
    /// Connect, Disconnect, Fail, and HTTP
    static level const access_core = 0x00003003;
    /// Special aggregate value representing "all levels"
    static level const all = 0xffffffff;

    /// Get the textual name of a channel given a channel id
    /**
     * Get the textual name of a channel given a channel id. The id must be that
     * of a single channel. Passing an aggregate channel package results in
     * undefined behavior.
     *
     * @param channel The channelid to look up.
     *
     * @return The name of the specified channel.
     */
    static char const * channel_name(level channel) {
        switch(channel) {
            case connect:
                return "connect";
            case disconnect:
                return "disconnect";
            case control:
                return "control";
            case frame_header:
                return "frame_header";
            case frame_payload:
                return "frame_payload";
            case message_header:
                return "message_header";
            case message_payload:
                return "message_payload";
            case endpoint:
                return "endpoint";
            case debug_handshake:
                return "debug_handshake";
            case debug_close:
                return "debug_close";
            case devel:
                return "devel";
            case app:
                return "application";
            case http:
                return "http";
            case fail:
                return "fail";
            default:
                return "unknown";
        }
    }
};

/// Basic logger that outputs to an ostream
template <typename concurrency, typename names>
class basic {
public:
    basic<concurrency,names>(channel_type_hint::value h =
        channel_type_hint::access)
      : m_static_channels(0xffffffff)
      , m_dynamic_channels(0)
      , m_out(h == channel_type_hint::error ? &std::cerr : &std::cout) {}

    basic<concurrency,names>(std::ostream * out)
      : m_static_channels(0xffffffff)
      , m_dynamic_channels(0)
      , m_out(out) {}

    basic<concurrency,names>(level c, channel_type_hint::value h =
        channel_type_hint::access)
      : m_static_channels(c)
      , m_dynamic_channels(0)
      , m_out(h == channel_type_hint::error ? &std::cerr : &std::cout) {}

    basic<concurrency,names>(level c, std::ostream * out)
      : m_static_channels(c)
      , m_dynamic_channels(0)
      , m_out(out) {}

    /// Destructor
    ~basic<concurrency,names>() {}

    /// Copy constructor
    basic<concurrency,names>(basic<concurrency,names> const & other)
     : m_static_channels(other.m_static_channels)
     , m_dynamic_channels(other.m_dynamic_channels)
     , m_out(other.m_out)
    {}
    
#ifdef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
    // no copy assignment operator because of const member variables
    basic<concurrency,names> & operator=(basic<concurrency,names> const &) = delete;
#endif // _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_

#ifdef _WEBSOCKETPP_MOVE_SEMANTICS_
    /// Move constructor
    basic<concurrency,names>(basic<concurrency,names> && other)
     : m_static_channels(other.m_static_channels)
     , m_dynamic_channels(other.m_dynamic_channels)
     , m_out(other.m_out)
    {}

#ifdef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
    // no move assignment operator because of const member variables
    basic<concurrency,names> & operator=(basic<concurrency,names> &&) = delete;
#endif // _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_

#endif // _WEBSOCKETPP_MOVE_SEMANTICS_

    void set_ostream(std::ostream * out = &std::cout) {
        m_out = out;
    }

    void set_channels(level channels) {
        if (channels == names::none) {
            clear_channels(names::all);
            return;
        }

        scoped_lock_type lock(m_lock);
        m_dynamic_channels |= (channels & m_static_channels);
    }

    void clear_channels(level channels) {
        scoped_lock_type lock(m_lock);
        m_dynamic_channels &= ~channels;
    }

    /// Write a string message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, std::string const & msg) {
        scoped_lock_type lock(m_lock);
        if (!this->dynamic_test(channel)) { return; }
        *m_out << "[" << timestamp << "] "
                  << "[" << names::channel_name(channel) << "] "
                  << msg << "\n";
        m_out->flush();
    }

    /// Write a cstring message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, char const * msg) {
        scoped_lock_type lock(m_lock);
        if (!this->dynamic_test(channel)) { return; }
        *m_out << "[" << timestamp << "] "
                  << "[" << names::channel_name(channel) << "] "
                  << msg << "\n";
        m_out->flush();
    }

    _WEBSOCKETPP_CONSTEXPR_TOKEN_ bool static_test(level channel) const {
        return ((channel & m_static_channels) != 0);
    }

    bool dynamic_test(level channel) {
        return ((channel & m_dynamic_channels) != 0);
    }

protected:
    typedef typename concurrency::scoped_lock_type scoped_lock_type;
    typedef typename concurrency::mutex_type mutex_type;
    mutex_type m_lock;

private:
    // The timestamp does not include the time zone, because on Windows with the
    // default registry settings, the time zone would be written out in full,
    // which would be obnoxiously verbose.
    //
    // TODO: find a workaround for this or make this format user settable
    static std::ostream & timestamp(std::ostream & os) {
        std::time_t t = std::time(NULL);
        std::tm lt = localtime(t);
        #ifdef _WEBSOCKETPP_PUTTIME_
            return os << std::put_time(&lt,"%Y-%m-%d %H:%M:%S");
        #else // Falls back to strftime, which requires a temporary copy of the string.
            char buffer[20];
            size_t result = std::strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",&lt);
            return os << (result == 0 ? "Unknown" : buffer);
        #endif
    }

    level const m_static_channels;
    level m_dynamic_channels;
    std::ostream * m_out;
};


/// Stub logger that ignores all input
class stub {
public:
    /// Construct the logger
    /**
     * @param hint A channel type specific hint for how to construct the logger
     */
    explicit stub(channel_type_hint::value) {}

    /// Construct the logger
    /**
     * @param default_channels A set of channels to statically enable
     * @param hint A channel type specific hint for how to construct the logger
     */
    stub(level, channel_type_hint::value) {}
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ stub() {}

    /// Dynamically enable the given list of channels
    /**
     * All operations on the stub logger are no-ops and all arguments are
     * ignored
     *
     * @param channels The package of channels to enable
     */
    void set_channels(level) {}

    /// Dynamically disable the given list of channels
    /**
     * All operations on the stub logger are no-ops and all arguments are
     * ignored
     *
     * @param channels The package of channels to disable
     */
    void clear_channels(level) {}

    /// Write a string message to the given channel
    /**
     * Writing on the stub logger is a no-op and all arguments are ignored
     *
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level, std::string const &) {}

    /// Write a cstring message to the given channel
    /**
     * Writing on the stub logger is a no-op and all arguments are ignored
     *
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level, char const *) {}

    /// Test whether a channel is statically enabled
    /**
     * The stub logger has no channels so all arguments are ignored and
     * `static_test` always returns false.
     *
     * @param channel The package of channels to test
     */
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ bool static_test(level) const {
        return false;
    }

    /// Test whether a channel is dynamically enabled
    /**
     * The stub logger has no channels so all arguments are ignored and
     * `dynamic_test` always returns false.
     *
     * @param channel The package of channels to test
     */
    bool dynamic_test(level) {
        return false;
    }
};


/// Basic logger that outputs to syslog
template <typename concurrency, typename names>
class syslog : public basic<concurrency, names> {
public:
    typedef basic<concurrency, names> base;

    /// Construct the logger
    /**
     * @param hint A channel type specific hint for how to construct the logger
     */
    syslog<concurrency,names>(channel_type_hint::value hint =
        channel_type_hint::access)
      : basic<concurrency,names>(hint), m_channel_type_hint(hint) {}

    /// Construct the logger
    /**
     * @param channels A set of channels to statically enable
     * @param hint A channel type specific hint for how to construct the logger
     */
    syslog<concurrency,names>(level channels, channel_type_hint::value hint =
        channel_type_hint::access)
      : basic<concurrency,names>(channels, hint), m_channel_type_hint(hint) {}

    /// Write a string message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, std::string const & msg) {
        write(channel, msg.c_str());
    }

    /// Write a cstring message to the given channel
    /**
     * @param channel The channel to write to
     * @param msg The message to write
     */
    void write(level channel, char const * msg) {
        scoped_lock_type lock(base::m_lock);
        if (!this->dynamic_test(channel)) { return; }
        ::syslog(syslog_priority(channel), "[%s] %s",
            names::channel_name(channel), msg);
    }
private:
    typedef typename base::scoped_lock_type scoped_lock_type;

    /// The default level is used for all access logs and any error logs that
    /// don't trivially map to one of the standard syslog levels.
    static int const default_level = LOG_INFO;

    /// retrieve the syslog priority code given a WebSocket++ channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    int syslog_priority(level channel) const {
        if (m_channel_type_hint == channel_type_hint::access) {
            return syslog_priority_access(channel);
        } else {
            return syslog_priority_error(channel);
        }
    }

    /// retrieve the syslog priority code given a WebSocket++ error channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    int syslog_priority_error(level channel) const {
        switch (channel) {
            case elevel::devel:
                return LOG_DEBUG;
            case elevel::library:
                return LOG_DEBUG;
            case elevel::info:
                return LOG_INFO;
            case elevel::warn:
                return LOG_WARNING;
            case elevel::rerror:
                return LOG_ERR;
            case elevel::fatal:
                return LOG_CRIT;
            default:
                return default_level;
        }
    }

    /// retrieve the syslog priority code given a WebSocket++ access channel
    /**
     * @param channel The level to look up
     * @return The syslog level associated with `channel`
     */
    _WEBSOCKETPP_CONSTEXPR_TOKEN_ int syslog_priority_access(level) const {
        return default_level;
    }

    channel_type_hint::value m_channel_type_hint;
};

} // log

#endif // WEBSOCKETPP_LOGGER_BASIC_HPP
