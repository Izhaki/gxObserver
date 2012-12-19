This article explains in a bit more depth how gxOvserver works.

## General

 - Each event is a member variable.
 - Each event class is a subclass of a specialised vari-parameter class inheriting from `gxEvent`.
 - It is events that store their callbacks attached to them (not the subject).
 - We do not allow the same object to register more than once to the same event.
 - Callbacks are implemented using using [Don Clugston FastDelegate](http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible).
   Although simple member function pointers would work in simple cases (simple inheritance, no virtual handlers ),
   as Don explains, callbacks can be a tricky thing to implement. FastDelegate solves this.

## Normal Events

We'll start with the bit of code:

    class SimpleSubject : virtual public gxSubject
    {
    public:
        gxDefineEvent( evAge, int )        
    };
    
After the preprocessor did its magic, the `gxDefineEvent` macro will generate this code:

    class SimpleSubject : virtual public gxSubject
    {
    public:
        class evAgeType : public gxEvent1< int > {}; 
        evAgeType evAge; 
        
        void evAgeSubscribe( evAgeType::gxDelegate aDelegate )
        { 
            Subscribe( evAge, aDelegate.GetMemento() ); 
        } 
        
        void evAgeUnsubscribe( void *aObj )
        {
            Unsubscribe( evAge, aObj );
        } 
        
        void Fire( evAgeType &aEvent, int a1 )
        { 
            aEvent.SetParams( a1 ); 
            gxSubject::Fire( aEvent );
        };
    };
    
Now, bit by bit:

### Event Definition

    class evAgeType : public gxEvent1< int > {}; 
    
We define a new type that is a subclass of an event with one int parameter. 
We'll use this type later on for the `Fire()` method, and as there might
be more than one event with only one int parameter (ie, `gxEvent1< int >`),
using `typedef` wouldn't do the trick here:

    // Not unique for overloading.
    typedef gxEvent1< int > evAgeType;

`gxEvent1` declares the delegate type for this combination of parameters like so:

    template < typename t1 >
    class gxEvent1 : public gxEvent
    {
    public:
        typedef fastdelegate::FastDelegate1< t1 > gxDelegate;
    
        ...    
    };
    
In the next line we define the event as a member variable:

    evAgeType evAge;

### Subscribe
    
Next, each event gets its own unique Subscribe method:

    void evAgeSubscribe( evAgeType::gxDelegate aDelegate )
    { 
        Subscribe( evAge, aDelegate.GetMemento() ); 
    }
    
We could have just used an overloaded method in the `gxSubject` base class, 
but then observers would have to subscribe with something like this:

    aSubject->gxSubscribe( aSubject->evAge, OnAgeChanged );
    
There are two issues with this syntax. First, there's an extra (ugly)
`aSubject->` that the programmer would have to type. But more 
importantly, we want to ensure that the signature of the callback
matches that of the event. This is why the delegate parameter has
to have the same type as the event delegate type (`evAgeType::gxDelegate`)
although `aDelegate` was actually created from the observer's handler signature.

The `gxSubscribe` macro translates this call:

    aSubject->gxSubscribe( evAge, OnAgeChanged );
    
Into:

    mSubject->evAgeSubscribe( fastdelegate::MakeDelegate( this, &ObserverClassName::OnAgeChanged ) );

### Unsubscribe

The unsubscribe method is also unique per event:

    void evAgeUnsubscribe( void *aObj )
    {
        Unsubscribe( evAge, aObj );
    }
    
The `gxUnsubcribe` macro translates an observer call like this:

    aSubject->gxUnsubcribe( evAge );
    
To:

    aSubject->evAgeUnsubcribe( this );
    
### Fire

Each subject also gets a unique `Fire` method per event. This ensures that Firing
an event comply with the event parameters definition.

    void Fire( evAgeType &aEvent, int a1 )
    { 
        aEvent.SetParams( a1 ); 
        gxSubject::Fire( aEvent );
    }; 
    
This is where things get slightly odd. The first thing we do is we cache the arguments
in the events. The `gxEvent1` used for the event also involves this code:

    template < typename t1 >
    class gxEvent1 : public gxEvent
    {
    public:
        void SetParams( t1 a1 )
        {
            m1 = a1;
        }
    private:
        gxDelegate mDelegate;
        
        t1 m1;
    };
    
The reason we do this is that events can be suspended (which happens via 
the subject interface) and we want the logic regarding whether or not to fire 
or queue an event to exist in the *general* gxSubject class - a class that has
no interface supporting variable length parameters. The implementation of 
`Fire()` in the `gxSubject` class looks like so:

    void gxSubject::Fire( gxEvent &aEvent, gxCallback aCallback )
    {    
        if ( mFiringMode == on )
        {
            // If a specific callback was requested then only fire that one,
            // otherwise Fire() will fire all callbacks.
            aCallback.empty() ? aEvent.Fire() : aEvent.Fire( aCallback );
        } else if ( mFiringMode == queue ) {
            QueueEvent( aEvent );
        } // Otherwise FiringMode is off
    }

We could have just taken this out of the `gxSubject` class and have it in the
specialised `Fire()` implementation of each event. But the design philosophy 
was to try and have as little as possible in macros.

The caching of event arguments is also handy at a lower level. It is the general
base class `gxEvent` that iterates all callbacks:

    void gxEvent::Fire()
    {
        gxCallbacks::iterator iCallback;
        
        // For each callback
        for( iCallback = mCallbacks.begin(); iCallback != mCallbacks.end(); ++iCallback )
        {
            Fire( *iCallback );
        }
    }
    
But it is each specialised subclass that does the actual firing (which is also the
right time to present the full code of `gxEvent1`):


    template < typename t1 >
    class gxEvent1 : public gxEvent
    {
    public:
        typedef fastdelegate::FastDelegate1< t1 > gxDelegate;
        
        void SetParams( t1 a1 )
        {
            m1 = a1;
        }
        
        virtual void Fire( gxCallback &aCallback )
        {
            mDelegate.SetMemento( aCallback );
            mDelegate( m1 );
        }
    private:
        gxDelegate mDelegate;
        
        t1 m1;
    };
    
The caching of event arguments also works nicely with event queuing.
If firing is suspended with a queue, firing an event will only set its
cache to the values of the last firing request (and add the event to the
queue). It is only these values that will be fired when `ResumeEvents()` 
is called, which will reduce redundancy of events (each event will only
be fired once).


## Bound Events

Bound events are declared like this:

    class SimpleSubject : virtual public gxSubject
    {
    public:
        gxDefineBoundEvent( evName, string, GetName() )
    };

This translates to the following code:

    class evNameType : public gxEvent1< string > {}; 
    evNameType evName; 
    
    void evNameSubscribe( evNameType::gxDelegate aDelegate )
    {
        Subscribe( evName, aDelegate.GetMemento() );
        evName.SetParams( GetName() );
        gxSubject::Fire( evName, aDelegate.GetMemento() );
    }
    
    void evNameUnsubscribe( void *aObj )
    {
        Unsubscribe( evName, aObj );
    }
    
    void Fire( evNameType &aEvent, string a1 )
    {
        aEvent.SetParams( a1 );
        gxSubject::Fire( aEvent );
    }
    
    void Fire( evNameType &aEvent )
    {
        aEvent.SetParams( GetName() );
        gxSubject::Fire( aEvent );
    };

Compared to normal event, the main difference here is in the subscribe method.
First, it sets the event arguments using the provided getter. Next, it fires the
event, but requesting to only notify the new observer (rather than all observers):

    void evNameSubscribe( evNameType::gxDelegate aDelegate )
    {
        Subscribe( evName, aDelegate.GetMemento() );
        evName.SetParams( GetName() );
        gxSubject::Fire( evName, aDelegate.GetMemento() );
    }
    
The other difference is that there's a new `Fire` method with the event only, that
uses the getter to fill the event:

    void Fire( evNameType &aEvent )
    {
        aEvent.SetParams( GetName() );
        gxSubject::Fire( aEvent );
    };
    
## Observers

Observers are required to provide their own class using the `gxDeclareObserver` macro:

    class SimpleObserver
    {
        gxDeclareObserver( SimpleObserver )
    public:
        ...
    };

This translates into:

    class SimpleObserver
    {
        typedef SimpleObserver ObserverClassName;
    public:
        ...
    };

Reason being is that we'd like to save the programmer having to provide
its own class with every subscription:

    mSubject->gxSubscribe( evAge, SimpleObserver::OnAgeChanged );

so the `gxSubscribe` macro injects `&ObserverClassName::` before the method
name in the actual call:

    mSubject->gxSubscribe( evAge, OnAgeChanged );
