gxObserver
==========

A Sleek C++ Implementation of the Observer Pattern


This implementation prioritises code readability (then performance, then memory).

Simple Usage Example
--------------------

main.cpp includes examples of all the features, on the most basic usage:    

### Subject

    class SimpleSubject : virtual public gxSubject
    {
    public:
        // Defining an event with a single int parameter
        gxDefineEvent( evAge, int )
        
        void FireEvents()
        {
            // Fire the event
            Fire( evAge, 69 );
        }
    };
    
### Observer    
    
    class SimpleObserver
    {
        gxDeclareObserver( SimpleObserver )
    public:
        SimpleObserver( SimpleSubject* aSubject )
        {
            mSubject = aSubject;
            
            // Subscribe to the subject events
            mSubject->gxSubscribe( evAge, OnAgeChanged );
        }
        
        virtual ~SimpleObserver()
        {
            // Unsubscribe from the subject events
            mSubject->gxUnsubscribe( evAge );
        }
        
        // The event handler callback
        void OnAgeChanged( int aAge )
        {
            cout << "SimpleObserver::OnAgeChanged with " << aAge << "\n";
        }
    private:
        SimpleSubject* mSubject;
    };

Pros and Cons
-------------

### Pros
 - Clean and short syntax.
 - Well documented readable code.
 - Event handlers are callbacks; they can be virtual. Thanks [Don Clugston](http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible)!
 - Up to 4 parameters per event (can be easily extended to 8 with a bit of copy-paste work)
 - Event suspension feature (with queuing option). 
 - Push strategy with or without sender.
 - Compile-time errors if the subject's firing method or the observer's handler signatures do not match the event definition.
 - Bound events (events bound to a member variable or a getter) allow automatic state update of the observer when it subscribe to its subject.

### Cons
 - Liberal memory usage - all event parameters will add to the subject's stack allocation.
 - Macros had to be used to ensure strict signatures and compile-time errors; but some are just to save the programmer typing long lines of code.
 - Event parameters cannot be passed by reference (either pointers or value passing only), and are better not be consts.
 - Callbacks must be member functions, so your observer must be a class. 
 - Subjects are non-copy classes (due to the event queue - might change in the future)

 
Manual
------

### Declaring Subjects

Subject classes are declared like this:

    #include "gxSubject.h"
    
    class Subject : virtual public gxSubject
    {
    }

### Defining Normal Events

Normal events (ie, not bound ones), are defined like so:
    
    class Subject : virtual public gxSubject
    {
    public:
        // An event called evMono with a single int parameter.
        gxDefineEvent( evMono, int )
    
        // An event called evQuad with 4 parameters, the first of 
        // which is the sender.
        gxDefineEvent( evQuad, const Subject*, int, int, bool )
    }

The first parameter is the event name. The rest of parameters represent 
the type of the parameters this event involves.
    
Notice that the event definition is required to be `public`.

Each event defines the signature of its protocols. The signature of the 
`Fire()` method and the observer's handler has to match the event signature.

**Note:** It is generally a bad idea to have the parameters of events as const - 
Some compilers ignore the const for primitive parameters in the handlers;
As the parameter types given in the event definition are copied to members
of the actual events, some compilers will complain that you are trying to 
assign a value to a const member of the event.
    
### Firing Normal Events

Given the previous two events:

    Fire( evMono, 9 );
    Fire( evQuad, this, 10, 50, true );
    
    
### Defining Bound Events

Apart for an optional sender parameter, a bound event is a single parameter 
event that knows how to retrieve that one parameter from a member variable 
or method.

To define a bound event, do this:

    class Subject : virtual public gxSubject
    {
    public:
        gxDefineBoundEvent( evAge, int, mAge )
    private:
        int mAge;    
    }

Or with a getter:

    class Subject : virtual public gxSubject
    {
    public:
        gxDefineBoundEvent( evAge, int, GetAge() )
        
        int GetAge() { return mAge; }
    private:
        int mAge;    
    }
    
Bound events are automatically fired when an observer subscribes to them. This 
ensures that the observer isconsistent with the subject (without the need for 
additional lines of code).

The second version of a bound event is one that carries the sender as well, 
and will be defined like so:

    class Subject : virtual public gxSubject
    {
    public:
        gxDefineBoundEvent( evAge, Subject*, int, mAge )
    private:
        int mAge;    
    }

The second parameter is a pointer to the sender class.

### Firing Bound Events

Since the subject knows how to retrieve the value of the sole parameter, 
bound events can be fired like so:

    Fire( evAge );
    
Notice that this will work for either event definition - either with or 
without a sender.

You can still call fire with custom arguments:

    Fire( evAge, 77 );
    
This again will work for events with or without a sender.

Although it is hard to think why you'd like to do so, you can still call 
fire with a custom sender:

    Fire( evAge, this, 77 );

### Suspending and Resuming events

Really easy:

    // Suspending events, but queuing them
    aSubject->SuspendEvents( true );    

    // Resuming events
    aSubject->ResumeEvents();
    
    
### Declaring Observers

An observer class skeleton looks like so:

    #include "gxObserver.h"

    class SomeObserver
    {
        // Note: no problem with this being private
        gxDeclareObserver( SomeObserver )
    public:
    }
    
As far a subscribing and unsubscribing goes, we repeat the example 
previously given:

    class SimpleObserver
    {
        gxDeclareObserver( SimpleObserver )
    public:
        SimpleObserver( SimpleSubject* aSubject )
        {
            mSubject = aSubject;
            
            // Subscribe to the subject events
            mSubject->gxSubscribe( evAge, OnAgeChanged );
        }
        
        virtual ~SimpleObserver()
        {
            // Unsubscribe from the subject events
            mSubject->gxUnsubscribe( evAge );
        }
        
        // The event handler callback
        void OnAgeChanged( int aAge )
        {
            cout << "SimpleObserver::OnAgeChanged with " << aAge << "\n";
        }
    private:
        SimpleSubject* mSubject;
    };
    
The only trick is with derived and base classes. We cannot subscribe to virtual handlers in the base class constructor, 
since when the base class is constructed it does not know anything about its derived class. So we need to subscribe 
outside the base class constructor. The base class might look like this:

    class BaseObserver
    {
        gxDeclareObserver( BaseObserver )
    public:
        BaseObserver( ComplexSubject* aSubject )
          : mSubject( aSubject ) { }
        
        void DoSubscribe()
        {
            // Subscribe to the subject events
            mSubject->gxSubscribe( evName, OnName );
        }
        
        virtual ~BaseObserver()
        {
            // Unsubscribe from the subject events
            mSubject->gxUnsubscribe( evName );
        }
        
        // A virtual event handler
        virtual void OnName( string aName )
        {
            cout << "BaseObserver::OnName with: " << aName << "\n";
        }
    protected:
        ComplexSubject* mSubject;
    };
    
And the derived one like this:    
    
    class DerivedObserver : public BaseObserver
    {
        gxDeclareObserver( DerivedObserver )
    public:
        DerivedObserver( ComplexSubject* aSubject )
        : BaseObserver( aSubject )
        {
            DoSubscribe();
        }
        
        // Event handlers
        virtual void OnName( string aName )
        {
            // We can call the base class handler if we want by uncommenting this line:
            // BaseObserver::OnName( aName );
            
            cout << "DerivedObserver::OnName with: " << aName << "\n";
    
        }
    };

