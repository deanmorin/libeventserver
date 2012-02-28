#include "eventbase.hpp"
#include <event2/thread.h>
#include <string.h>
#include "badbaseexception.hpp"
namespace dm {

EventBase::EventBase(const char* method)
{
    // enable locking for libevent structure
    if (evthread_use_pthreads())
    {
        throw std::exception();
    }

#ifdef DEBUG
    evthread_enable_lock_debuging();
    event_enable_debug_mode();
#endif

    struct event_config *config;
    config = event_config_new();

    int i = 0;
    const char** availMethods = event_get_supported_methods();

    for (i = 0; availMethods[i] != NULL; i++)
    {
        if (strcmp(availMethods[i], method)) {
            event_config_avoid_method(config, availMethods[i]);
        }
    }
    base_ = event_base_new_with_config(config);
    event_base_get_method(base_);
    event_config_free(config);

    if (!base_)
    {
        throw BadBaseException();
    }
}


EventBase::~EventBase()
{
    event_base_free(base_);
}


const char* 
EventBase::getMethod()
{
    return event_base_get_method(base_);
}


struct event_base*
EventBase::getBase()
{
    return base_;
}


const char**
EventBase::getAvailableMethods()
{
    return event_get_supported_methods();
}

} // namespace dm
