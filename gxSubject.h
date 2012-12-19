#ifndef gxSubject_h
#define gxSubject_h

/**
 * @author Izhaki - https://github.com/Izhaki/gxObserver
 */

/** @file gxSubject.h */

#include "gxEvent.h"

/**
 * @brief The base class for all subjects (aka observabales).
 *
 * Subjects fire events, to which observers subscribe.
 * This class also allows the suspension of events (with a queuing option).
 *
 * Subclasses define their own events using the #gxDefineEvent() or 
 * #gxDefineBoundEvent() macros. For example:
 * 
 *     class Figure : virtual public gxSubject
 *     {
 *     public:
 *         gxDeclareEvent( evSize, Size* )
 *         gxDeclareBoundEvent( evAge, int, GetAge() )
 *     };
 */
class gxSubject
{
private:
    // Due to the event queue, this is a non-copy class.
    gxSubject(const gxSubject& aOther);
    gxSubject& operator= (const gxSubject& aOther);

public:
    gxSubject():
        mFiringMode( on ) { }
    
    virtual ~gxSubject();

    /**
     * @brief Allows observers to subscribe to a particular event.
     * @param aEvent The {@link gxEvent event} to which observers subscribe.
     * @param aCallback The observer's callback handeling this event.
     */
    void Subscribe( gxEvent &aEvent, gxCallback aCallback );

    /**
     * @brief Allows observers unsubscribe from a particular event.
     * @param aEvent The {@link gxEvent event} from which observer unsubscribe.
     * @param aObj The observer's reference (this).
     */
    void Unsubscribe( gxEvent &aEvent, void *aObj );

    /**
     * @brief Suspends the firing of events.
     *
     * @param aQueueSuspended
     *     If true, events fired by the subject will be queued and fired once
     *     ResumeEvents is called. 
     *     **Note:** If an event is is being fired more than onces when queue 
     *     suspension is on, only the most recent event will be fired upon 
     *     ResumeEvents().
     */
    void SuspendEvents( bool aQueueSuspended );

    /**
     * @brief Resumes the firing of events.
     *
     * If aQueueSuspended was true when SuspendEvents was called, this method
     * will fire all queued events.
     */
    void ResumeEvents();
    
protected:
    /**
     * @brief Fires an event, accounting for any event supspension or queuing.
     *
     * **Note:** The actual Fire method used with subjects is the one generated
     * by the #gxDefineEvent() or #gxDefineBoundEvent() macros, which will have
     * the correct vari-length parameters for the event being called. The macros
     * will also ensure that the passed parameters are chached in the event 
     * prior to firing it.
     *
     * @param aEvent The {@link gxEvent event} to be fired.
     * @param [aCallback] The {@link gxCallback callback} to be fired. Defaults
     * to an empty callback, in which case all callbacks will be called.
     */
    void Fire( gxEvent &aEvent, gxCallback aCallback = gxCallback() );

    /**
     * @brief Fires an event to a specific callback, accounting for any event 
     * supspension or queuing.
     *
     * This method is only used with bound events, where upon subscription 
     * we only want to update the subscribing observer.
     *
     * @param aEvent The {@link gxEvent event} to be fired.
     * @param aCallback The {@link gxCallback callback} to be fired.     
     */
//    void Fire( gxEvent &aEvent, gxCallback aCallback );

private:
    /** The event Firing mode can be one of 3: */
    enum gxFiringMode {
        on,   /**< Events will fire straigt away. */
        off,  /**< No events will be fired. */
        queue /**< Events will be queued and fired once ResumeEvents() is 
               * called. */
    } mFiringMode;
    
    typedef std::vector< gxEvent* > gxEventsQueue;
    // The events queue
    gxEventsQueue mEventsQueue;
    
    /**
     * @brief Checks if an {@link gxEvent event} already in the queue.
     *
     * @param aEvent The {@link gxEvent event} to check.
     * @return true if the event is already in the queue.
     */    
    bool EventAlreadyInQueue( gxEvent &aEvent );

    /**
     * @brief Puts an {@link gxEvent event} in the queue.
     *
     * @param aEvent The {@link gxEvent event} to be queued.
     */
    void QueueEvent( gxEvent &aEvent );
};

////////////////////////////////////////////////////////////////////////////////
//                        Code Generation Macros                              //
////////////////////////////////////////////////////////////////////////////////


/** The implementation of a non-bound event subscription.
 *  The generated method is event specific and simply calls subscribe on the
 *  subject base class with the correct event member.
 *
 *  The #gxSubscribe() macro is the one calling the generated method.
 */
#define gxImpSubscribe( mEvent ) \
    void mEvent##Subscribe( mEvent##Type::gxDelegate aDelegate ) \
    { \
        Subscribe( mEvent, aDelegate.GetMemento() ); \
    }

/** The implementation of an event unsubsceiption.
 * The generated method is event specific and simply calls unsubscribe on the
 * subject base class with the correct event member.
 *
 *  The #gxUnsubscribe() macro is the one calling the generated method.
 */
#define gxImpUnsubscribe( mEvent ) \
    void mEvent##Unsubscribe( void *aObj ) \
    { \
        Unsubscribe( mEvent, aObj ); \
    } \

/** Common non-bound event macro.
 * It generates the event member variable, and the subscription related methods.
 */
#define gxImpEventCommon( mEvent ) \
    mEvent##Type mEvent; \
    gxImpSubscribe( mEvent ) \
    gxImpUnsubscribe( mEvent )

/** A macro generating a common vari-argument fire methods.
 * First, we set the parameters of the relevant event. This will be useful if
 * events are queued rather than fired straight away. Then we fire the event.
 */
#define gxImpFire( mEvent, ... ) \
    { \
        mEvent.SetParams(  __VA_ARGS__  ); \
        gxSubject::Fire( mEvent ); \
    }

////////////////////////////////////////////////////////////////////////////////
//               Non-bound Events: Variable parameter event generator         //
//----------------------------------------------------------------------------//
// The generated code includes Subscribe, Unsubscribe and Fire methods that   //
// are unique to the generate event.                                          //
// Note that the event type is a class rather than typedef, so it is unique.  //
////////////////////////////////////////////////////////////////////////////////

/** Generates code for events with no parameters. */
#define gxDefineEvent0( mEvent ) \
    class mEvent##Type : public gxEvent0 {}; \
    gxImpEventCommon( mEvent ) \
    void Fire( mEvent##Type &aEvent ) \
    { \
        gxSubject::Fire( mEvent ); \
    }

/** Generates code for events with 1 parameter. */
#define gxDefineEvent1( mEvent, t1 ) \
    class mEvent##Type : public gxEvent1< t1 > {}; \
    gxImpEventCommon( mEvent ) \
    void Fire( mEvent##Type &aEvent, t1 a1 ) \
        gxImpFire( aEvent, a1 ) \

/** Generates code for events with 2 parameters. */
#define gxDefineEvent2( mEvent, t1, t2 ) \
    class mEvent##Type : public gxEvent2< t1, t2 > {}; \
    gxImpEventCommon( mEvent ) \
    void Fire( mEvent##Type &aEvent, t1 a1, t2 a2 ) \
        gxImpFire( aEvent, a1, a2 ) \

/** Generates code for events with 3 parameters. */
#define gxDefineEvent3( mEvent, t1, t2, t3 ) \
    class mEvent##Type : public gxEvent3< t1, t2, t3 > {}; \
    gxImpEventCommon( mEvent ) \
    void Fire( mEvent##Type &aEvent, t1 a1, t2 a2, t3 a3 ) \
        gxImpFire( aEvent, a1, a2, a3 ) \

/** Generates code for events with 4 parameters. */
#define gxDefineEvent4( mEvent, t1, t2, t3, t4 ) \
    class mEvent##Type : public gxEvent4< t1, t2, t3, t4 > {}; \
    gxImpEventCommon( mEvent ) \
    void Fire( mEvent##Type &aEvent, t1 a1, t2 a2, t3 a3, t4 a4 ) \
        gxImpFire( aEvent, a1, a2, a3, a4 ) \


/** A router macros to get us to the macro with the correct amount of parameters.
 * Based on http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros
 */
#define gxDefineEventX( x, A, B, C, D, BINGO, ...) BINGO
/**
 * Defines a non-bound event and its (variable length) parameters. 
 * Example usage:
 *
 *     gxDefineEvent( evAgeChange, int )
 *
 * **Note:** Do not use const before primitive types, as compilers often omit
 * that const in callback signature (the callback may still have const for its
 * parameters, just don't include it in the event definition). Non primitive
 * parameters should come with consts, if such is the callback signature:
 *
 *     gxDefineEvent( evSizeChang, const Size* )
 *
 * @param mEvent The event name, which will become the name of the event member.
 */
#define gxDefineEvent( mEvent, ...)\
    gxDefineEventX( mEvent, ##__VA_ARGS__,\
        gxDefineEvent4( mEvent, __VA_ARGS__),\
        gxDefineEvent3( mEvent, __VA_ARGS__),\
        gxDefineEvent2( mEvent, __VA_ARGS__),\
        gxDefineEvent1( mEvent, __VA_ARGS__),\
        gxDefineEvent0( mEvent )\
    );

////////////////////////////////////////////////////////////////////////////////
//                                 Bound Events                               //
//----------------------------------------------------------------------------//
// Bound events bound their single parameter to a getter. There are two types //
// of bound events: one without the sender, and one with.                     //
// Bound events fire upon subscription, and can also be fired just by the event name  //
////////////////////////////////////////////////////////////////////////////////

/**
 * Defines a bound event. Example usage:
 *
 *     gxDefineBoundEvent( evAgeChange, int, GetAge() )
 *
 * @param mEvent The event name, which will become the name of the event member.
 * @param tParam The type of the single parameter of this event.
 * @param aGetter The getter function or member variable that this event is
 * bound to.
 */
#define gxDefineBoundEventWithoutSender( mEvent, t1, aGetter ) \
    class mEvent##Type : public gxEvent1< t1 > {}; \
    mEvent##Type mEvent; \
    void mEvent##Subscribe( mEvent##Type::gxDelegate aDelegate ) \
    { \
        Subscribe( mEvent, aDelegate.GetMemento() ); \
        mEvent.SetParams( aGetter ); \
        gxSubject::Fire( mEvent, aDelegate.GetMemento() ); \
    } \
    gxImpUnsubscribe( mEvent )\
    void Fire( mEvent##Type &aEvent, t1 a1 ) \
        gxImpFire( aEvent, a1 ) \
    void Fire( mEvent##Type &aEvent ) \
        gxImpFire( aEvent, aGetter ) \

/**
 * Defines a bound event that also carries the sender when being fired.
 * Example usage:
 *
 *     gxDefineBoundEvent( evSizeChang, SubjectClass*, int, GetAge() )
 *
 * @param mEvent The event name, which will become the name of the event member.
 * @param tSubject The class of the subject class.
 * @param tParam The type of the single parameter of this event.
 * @param aGetter The getter function or member variable that this event is
 * bound to.
 */
#define gxDefineBoundEventWithSender( mEvent, tSubject, t1, aGetter ) \
    class mEvent##Type : public gxEvent2< tSubject, t1 > {}; \
    mEvent##Type mEvent; \
    void mEvent##Subscribe( mEvent##Type::gxDelegate aDelegate ) \
    { \
        Subscribe( mEvent, aDelegate.GetMemento() ); \
        mEvent.SetParams( this, aGetter ); \
        gxSubject::Fire( mEvent, aDelegate.GetMemento() ); \
    } \
    gxImpUnsubscribe( mEvent )\
    void Fire( mEvent##Type &aEvent, tSubject aSubject, t1 a1 ) \
        gxImpFire( aEvent, aSubject, a1 ) \
    void Fire( mEvent##Type &aEvent, t1 a1 ) \
        gxImpFire( aEvent, this, a1 ) \
    void Fire( mEvent##Type &aEvent ) \
        gxImpFire( aEvent, this, aGetter ) \

/** A router macros to get us to the macro with the correct amount of parameters.
 * Based on http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros
 */
#define gxDefineBoundEventX( x, A, B, C, BINGO, ...) BINGO
/**
 * A router macro to pick either #gxDefineBoundEventWithSender() or 
 * #gxDefineBoundEventWithoutSender().
 *
 * Excepts two signatures:
 *
 *     gxDefineBoundEvent( evAgeChange, int, GetAge() )
 * or
 *
 *     gxDefineBoundEvent( evSizeChang, SubjectClass*, int, GetAge() )
 */
#define gxDefineBoundEvent( mEvent, ...)\
    gxDefineBoundEventX( mEvent, ##__VA_ARGS__,\
        gxDefineBoundEventWithSender( mEvent, __VA_ARGS__),\
        gxDefineBoundEventWithoutSender( mEvent, __VA_ARGS__)\
    );

#endif
