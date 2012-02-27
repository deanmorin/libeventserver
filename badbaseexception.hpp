#ifndef DM_BADBASEEXCEPTION_HPP
#define DM_BADBASEEXCEPTION_HPP
namespace dm {

/**
 * @author Dean Morin
 */
class BadBaseException : public std::exception
{
public:
    virtual const char* what() const throw()
    {
        return "Event base does not exist on this system.";
    }
};

} // namespace dm
#endif
