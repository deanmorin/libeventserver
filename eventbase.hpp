#ifndef DM_EVENTBASE_HPP
#define DM_EVENTBASE_HPP
#include <event2/event.h>
namespace dm {

/**
 * Wrapper class for setting up an event base.
 * @author Dean Morin
 */
class EventBase
{
private:
    struct event_base* base_;

public:
    /**
     * @author Dean Morin
     * @param method The event method to use.
     * @throws BadBaseException method is not available on this system.
     * @throws exception pthreads are not available on this system. 
     */
    EventBase(const char* method);
    ~EventBase();

    /**
     * @author Dean Morin
     * @return The name of the event handling method being used.
     */
    const char* getMethod();
    /**
     * @author Dean Morin
     * @return The event base structure.
     */
    struct event_base* getBase();
    /**
     * @author Dean Morin
     * @return All available event handling methods for this system.
     */
    static const char** getAvailableMethods();
};

} // namespace dm
#endif
