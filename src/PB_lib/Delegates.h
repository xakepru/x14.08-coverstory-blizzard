#ifndef __DELEGATES
#define __DELEGATES

// Interface for delegate with delegating function with 2 arguments
template<class Out, class Arg, class Arg2> 
class IDelegate
{
public:
	virtual	Out Invoke(Arg, Arg2)=0;
};

// 1 argument
template<class Out, class Arg> 
class IDelegate<Out, Arg, void>
{
public:
	virtual	Out Invoke(Arg)=0;
};

// no arguments (signal)
template<class Out> 
class IDelegate<Out, void, void>
{
public:
	virtual	Out Invoke()=0;
};

// DelegateBase

template<class Object, class Out, class Arg, class Arg2> 
class DelegateBase2Arg: public IDelegate<Out, Arg, Arg2>
{
protected:
	Object* obj;
	typedef Out (Object::*pfunc) (Arg, Arg2);
	pfunc func;

public:
	DelegateBase2Arg(Object* obj, pfunc f)
	{
		this->obj = obj;
		func = f;
	}

	Out Invoke(Arg a, Arg2 b)
	{
		/*
		like this:
			mov ecx, obj
			mov eax, [obj.vtable+offset]
			call eax
		*/
		if (obj)
		(obj->*func)(a, b);
	}
}; 

template<class Arg> 
class EventHandler
{
	IDelegate<void, void*, Arg> *del;
public:
	EventHandler()
	{
		del = NULL;
	}

	void operator =(IDelegate<void, void*, Arg>* cbdel)
	{
		Assign( cbdel );
	}

	void Assign (IDelegate<void, void*, Arg>* cbdel)
	{
		del = cbdel;
		return;
	}

	void operator () (void* caller, Arg arg)
	{
		Invoke(caller, arg);
	}

	void Invoke (void* caller, Arg arg)
	{
		if (del)
		del->Invoke(caller, arg);
	}

	bool Empty() // useless?
	{
		return del==NULL;
	}

	template<class Handler>
	static IDelegate<void, void*, Arg>* CreateHandler(Handler* obj, void (Handler::*pfunc) (void*, Arg) )
	{
		return new DelegateBase2Arg<Handler, void, void*, Arg>(obj, pfunc);
	}
};



/*
	Out Invoke(Arg arg)
	{
		return (obj->*func)(arg);
	}

	Out Invoke(void)
	{
		return (obj->*func)();
	}

template<class Object> 
class Delegate<Object,void,void>: public IDelegate<void, void>
{
protected:
	Object* obj;
	typedef void (Object::*pfunc) ();
	pfunc func;

public:
	Delegate(Object* obj, pfunc f)
	{
		this->obj = obj;
		func = f;
	}

	void Invoke()
	{
		(obj->*func)();
	}
};

template<class Object, class Arg> 
Delegate<Object,void,Arg>* CreateDelegate(Object* obj, void (Object::*pfunc) (Arg arg))
{
	return new Delegate<Object, void, Arg>(obj, pfunc);
}

template<class Object> 
Delegate<Object,void,void>* CreateDelegate(Object* obj, void (Object::*pfunc) ())
{
	return new Delegate<Object, void, void>(obj, pfunc);
}

template<class ArgClass> class CallBackFunc
{
	IDelegate<void, ArgClass> *del;

public:
	CallBackFunc()
	{
		del = 0;
	}

	void operator =(IDelegate<void, ArgClass>* cbdel)
	{
		del = cbdel;
		return;
	}

	void operator () (ArgClass arg)
	{
		if (del)
		del->Invoke(arg);
	}
};

template<> class CallBackFunc<void>
{
	IDelegate<void, void> *del;

public:
	CallBackFunc()
	{
		del = 0;
	}

	void operator =(IDelegate<void, void>* cbdel)
	{
		del = cbdel;
		return;
	}

	void operator () (void)
	{
		if (del)
		del->Invoke();
	}
};
*/

#endif