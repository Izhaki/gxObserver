#ifndef gxEvent_h
#define gxEvent_h

/**
 * @author Izhaki - https://github.com/Izhaki/gxObserver
 */

#include "FastDelegate.h"
#include <vector>

/**
 * @brief A class wrapping callbacks - the combination of object pointers and
 * member function pointers.
 *
 * This particular callback implementation is nothing more than a 
 * DelegateMemento - an object that wraps the callback object and a member 
 * function pointer. DelegateMementos can be retrieved from and set into
 * fastdelegates, which execute the callback.
 */
class gxCallback : public fastdelegate::DelegateMemento
{
public:
    /**
     * @brief The default constructor represents an empty callback.
     */
    gxCallback() { }
    
    /**
     * @brief A copy constructor from DelegateMemento class.
     * @param aMemento The DelegateMemento.
     */
    gxCallback( const fastdelegate::DelegateMemento &aMemento )
        :fastdelegate::DelegateMemento( aMemento ) { }
    
    /**
     * For some odd reason (and a opposed to the documentation) DelegateMemento
     * does not implement the equality operator. So we do (although we don't
     * really need this, mods might).
     *
     * @param aCallback 
     *     The callback we are comparing to.
     */
	bool operator == (const gxCallback &aCallback) const
    {
		return IsEqual( aCallback );
    }
    
    /**
     * @brief Retrieves the object pointer from the callback.
     *
     * We need this when checking whether the same object tries to register
     * with an event twice.
     * @param aMemento The DelegateMemento.
     * @return A pointer to the callback object.
     */
    fastdelegate::detail::GenericClass *GetObj() { return m_pthis; }
};


/**
 * @brief Base class for {@link gxSubject subject} events. Events are 
 * defined within subjects using the #gxDefineEvent() or #gxDefineBoundEvent() 
 * macros.
 *
 * An event allows observers to subscribe to it by providing a {@link gxCallback
 * callback}. It maintain a list of ovserver
 * callbacks, and facilitates firing to all observers.
 *
 */
class gxEvent
{
public:
    
    virtual ~gxEvent();
    
    /**
     * @brief Adds a callback to the subscribers list of this object.
     *
     * @param aCallback The callback to be added.
     */
    void Subscribe( gxCallback &aCallback );

    /**
     * @brief Removes an object from this event subscribers list.
     *
     * @param aCallback The callback to be added.
     */
    void Unsubscribe( void *aObj );

    /**
     * @brief Itterates through all callbacks and fire them.
     */
    virtual void Fire();
    
    /**
     * @brief Fires a specific callback.
     *
     * This is an abstract method that will be implemented by the specialised
     * derived class.
     * @param aCallback The callback to fire.
     */
    virtual void Fire( gxCallback &aCallback ) = 0;
protected:
    
    /**
     * @brief Checks if a callback is already in the subscription list.
     *
     * @param aCallback The callback to check.
     * @return true if the callback is already in the subscription list.
     */
    bool AlreadySubscribed( gxCallback &aCallback );
private:
    typedef std::vector< gxCallback > gxCallbacks;
    /**
     * @brief The list of callback subscribed to this event.
     */
    gxCallbacks mCallbacks;
};


/**
 * @brief A specialized event class for events with no parameters.
 */
class gxEvent0 : public gxEvent
{
public:
    typedef fastdelegate::FastDelegate0<> gxDelegate;

    /**
     * @brief Calls the actual callback.
     *
     * The base class gxEvent will call this for each callback, or the subject
     * will call this directly upon subscription of bound events.
     */
    virtual void Fire( gxCallback &aCallback )
    {
        mDelegate.SetMemento( aCallback );
        mDelegate();
    }
private:
    gxDelegate mDelegate;
};

/**
 * @brief A specialized event class for events with 1 parameter.
 */
template < typename t1 >
class gxEvent1 : public gxEvent
{
public:
    typedef fastdelegate::FastDelegate1< t1 > gxDelegate;
    
    /**
     * @brief Caches the arguments about to be fired (or queued) with this
     * event.
     */
    void SetParams( t1 a1 )
    {
        m1 = a1;
    }
    
    /**
     * @brief Calls the actual callback with the cached arguments.
     *
     * The base class gxEvent will call this for each callback, or the subject
     * will call this directly upon subscription of bound events.
     */
    virtual void Fire( gxCallback &aCallback )
    {
        mDelegate.SetMemento( aCallback );
        mDelegate( m1 );
    }
private:
    gxDelegate mDelegate;
    
    t1 m1;
};

/**
 * @brief A specialized event class for events with 2 parameters.
 */
template < typename t1, typename t2 >
class gxEvent2 : public gxEvent
{
public:
    typedef fastdelegate::FastDelegate2< t1, t2 > gxDelegate;
    
    /**
     * @brief Caches the arguments about to be fired (or queued) with this
     * event.
     */
    void SetParams( t1 a1, t2 a2 )
    {
        m1 = a1; m2 = a2;
    }
    
    /**
     * @brief Calls the actual callback with the cached arguments.
     *
     * The base class gxEvent will call this for each callback, or the subject
     * will call this directly upon subscription of bound events.
     */    
    virtual void Fire( gxCallback &aCallback )
    {
        mDelegate.SetMemento( aCallback );
        mDelegate( m1, m2 );
    }
private:
    gxDelegate mDelegate;
    
    t1 m1; t2 m2;
};

/**
 * @brief A specialized event class for events with 3 parameters.
 */
template < typename t1, typename t2, typename t3 >
class gxEvent3 : public gxEvent
{
public:
    typedef fastdelegate::FastDelegate3< t1, t2, t3 > gxDelegate;
    
    /**
     * @brief Caches the arguments about to be fired (or queued) with this
     * event.
     */
    void SetParams( t1 a1, t2 a2, t3 a3 )
    {
        m1 = a1; m2 = a2; m3 = a3;
    }
    
    /**
     * @brief Calls the actual callback with the cached arguments.
     *
     * The base class gxEvent will call this for each callback, or the subject
     * will call this directly upon subscription of bound events.
     */
    virtual void Fire( gxCallback &aCallback )
    {
        mDelegate.SetMemento( aCallback );
        mDelegate( m1, m2, m3 );
    }
private:
    gxDelegate mDelegate;
    
    t1 m1; t2 m2; t3 m3;
};

/**
 * @brief A specialized event class for events with 4 parameters.
 */
template < typename t1, typename t2, typename t3, typename t4 >
class gxEvent4 : public gxEvent
{
public:
    typedef fastdelegate::FastDelegate4< t1, t2, t3, t4 > gxDelegate;

    /**
     * @brief Caches the arguments about to be fired (or queued) with this
     * event.
     */
    void SetParams( t1 a1, t2 a2, t3 a3, t4 a4 )
    {
        m1 = a1; m2 = a2; m3 = a3; m4 = a4;
    }
    
    /**
     * @brief Calls the actual callback with the cached arguments.
     *
     * The base class gxEvent will call this for each callback, or the subject
     * will call this directly upon subscription of bound events.
     */
    virtual void Fire( gxCallback &aCallback )
    {
        mDelegate.SetMemento( aCallback );
        mDelegate( m1, m2, m3, m4 );
    }
private:
    gxDelegate mDelegate;
    
    t1 m1; t2 m2; t3 m3; t4 m4;
};

#endif // gxEvent_h
