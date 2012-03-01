#ifndef DM_EVENTBASE_HPP
#define DM_EVENTBASE_HPP
#include <event2/event.h>
namespace dm {

/**
 * Wrapper class for setting up an event base. Once constructed, pass the event
 * base to any libevent calls with eventbasename.getBase().
 *
 * @author Dean Morin
 */
class EventBase
{
private:
    /** The structure needed by many libevent calls. */
    struct event_base* base_;

public:
    /**
     * Creates a libevent base of the type specified by method.
     *
     * @author Dean Morin
     * @param method The event method to use (select, epoll, kqueue, etc.).
     * @throws BadBaseException method is not available on this system.
     * @throws exception pthreads are not available on this system. 
     */
    EventBase(const char* method);
    ~EventBase();

    /**
     * Get the name of the event base (select, epoll, kqueue, etc.).
     *
     * @author Dean Morin
     * @return The name of the event handling method being used.
     */
    const char* getMethod();
    /**
     * Get the event base. This is needed by many calls in the libevent library.
     *
     * @author Dean Morin
     * @return The event base structure.
     */
    struct event_base* getBase();
    /**
     * Get a list of the event bases for this system (select, epoll, kqueue, 
     * etc.).
     *
     * @author Dean Morin
     * @return All available event handling methods for this system.
     */
    static const char** getAvailableMethods();
};

} // namespace dm
#endif
