
//
// SqratClass: Class Binding
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source
//    distribution.
//

#if !defined(_SCRAT_CLASS_H_)
#define _SCRAT_CLASS_H_

#include <squirrel.h>
#include <sqmodule.h>
extern HSQUIRRELVM v;
extern HSQAPI sq;

#include <string.h>

#include "sqratObject.h"
#include "sqratClassType.h"
#include "sqratMemberMethods.h"
#include "sqratAllocator.h"

namespace Sqrat
{

    /**
        @tparam    C    class type to expose
        @tparam A    allocator to use when instantiating and destroying class instances in Squirrel

        @remarks
        DefaultAllocator<C> is used if no allocator is specified. This should be sufficent for most classes
        but if specific behavior is desired it can be overridden. If the class should not be instantiated from
        Squirrel the NoConstructor allocator may be used.
    */
    /// Exposes a C++ class to Squirrel
    template <class C, class A = DefaultAllocator<C>>
    class Class : public Object
    {
    public:
        /**
            @param v    Squirrel virtual machine to bind to
        */
        /// Constructor
        Class(HSQUIRRELVM v = DefaultVM::Get(), bool createClass = true) : Object(v, false)
        {
            if (createClass && !ClassType<C>::Initialized(v))
            {
                HSQOBJECT &classObj = ClassType<C>::ClassObject(v);
                sq->resetobject(&classObj);

                sq->newclass(vm, false);
                sq->getstackobj(vm, -1, &classObj);
                sq->addref(vm, &classObj); // must addref before the pop!
                sq->pop(vm, 1);

                InitClass();
                ClassType<C>::Initialized(v) = true;
            }
        }

        ~Class()
        {
            /*ClassType<C>::deleteClassTypeData(vm);*/
            /* it seems the original design by Tojo was that  ClassType objects are static
               so they presist with the lifetime of the program; so we cannot delete the
               ClassType object here */
        }

        /// Get the Squirrel Object for this Class (const)
        virtual HSQOBJECT GetObject() const
        {
            return ClassType<C>::ClassObject(vm);
        }

        /// Get the Squirrel Object for this Class (ref)
        virtual HSQOBJECT &GetObject()
        {
            return ClassType<C>::ClassObject(vm);
        }

    public:
        //
        // Variable Binding
        //

        /**
            @param name    name of the static slot
            @param var    value to assign
        */
        /// Assign a static class slot a value
        template <class V>
        Class &SetStaticValue(const SQChar *name, const V &val)
        {
            BindValue<V>(name, val, true);
            return *this;
        }

        /**
            @param name    name of the slot
            @param var    value to assign
        */
        /// Assign a class slot a value
        template <class V>
        Class &SetValue(const SQChar *name, const V &val)
        {
            BindValue<V>(name, val, false);
            return *this;
        }

        /**
            @param name    name of the variable as it will appear in Squirrel
            @param var    variable to bind
        */
        /// Bind a class variable
        template <class V>
        Class &Var(const SQChar *name, V C::*var)
        {
            // Add the getter
            BindAccessor(name, &var, sizeof(var), &sqDefaultGet<C, V>, ClassType<C>::GetTable(vm));

            // Add the setter
            BindAccessor(name, &var, sizeof(var), &sqDefaultSet<C, V>, ClassType<C>::SetTable(vm));

            return *this;
        }

        /// Bind a class property (variable accessed via a setter and getter)
        template <class F1, class F2>
        Class &Prop(const SQChar *name, F1 getMethod, F2 setMethod)
        {
            if (getMethod != NULL)
            {
                // Add the getter
                BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberFunc(getMethod), ClassType<C>::GetTable(vm));
            }

            if (setMethod != NULL)
            {
                // Add the setter
                BindAccessor(name, &setMethod, sizeof(setMethod), SqMemberFunc(setMethod), ClassType<C>::SetTable(vm));
            }

            return *this;
        }

        /// Bind a read only class property (variable accessed via a getter)
        template <class F>
        Class &Prop(const SQChar *name, F getMethod)
        {
            // Add the getter
            BindAccessor(name, &getMethod, sizeof(getMethod), SqMemberFunc(getMethod), ClassType<C>::GetTable(vm));

            return *this;
        }

        // TODO: Handle static instance vars

        //
        // Function Binding
        //

        template <class F>
        Class &Func(const SQChar *name, F method)
        {
            BindFunc(name, &method, sizeof(method), SqMemberFunc(method));
            return *this;
        }

        template <class F>
        Class &Func(const SQChar *name, F method, SQInteger argCount, const SQChar *arguments)
        {
            BindFunc(name, &method, sizeof(method), SqMemberFunc(method), argCount, arguments);
            return *this;
        }

        template <class F>
        Class &Overload(const SQChar *name, F method)
        {
            BindOverload(name, &method, sizeof(method), SqMemberFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
            return *this;
        }

        template <class F>
        Class &Overload(const SQChar *name, F method, SQInteger checkParamCount, const SQChar *arguments)
        {
            BindOverload(name, &method, sizeof(method), SqMemberFunc(method), SqOverloadFunc(method), SqGetArgCount(method), checkParamCount, arguments);
            return *this;
        }

        template <class F>
        Class &GlobalFunc(const SQChar *name, F method)
        {
            BindFunc(name, &method, sizeof(method), SqMemberGlobalFunc(method));
            return *this;
        }

        template <class F>
        Class &StaticFunc(const SQChar *name, F method)
        {
            BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
            return *this;
        }

        Class &SquirrelFunc(const SQChar *name, SQFUNCTION func)
        {
            sq->pushobject(vm, ClassType<C>::ClassObject(vm));
            sq->pushstring(vm, name, -1);
            sq->newclosure(vm, func, 0);
            sq->newslot(vm, -3, false);
            sq->pop(vm, 1); // pop table

            return *this;
        }

        //
        // Function Calls
        //

        Function GetFunction(const SQChar *name)
        {
            HSQOBJECT funcObj;
            sq->pushobject(vm, ClassType<C>::ClassObject(vm));
            sq->pushstring(vm, name, -1);
            if (SQ_FAILED(sq->get(vm, -2)))
            {
                sq->pushnull(vm);
            }
            sq->getstackobj(vm, -1, &funcObj);
            sq->pop(vm, 2);

            return Function(vm, ClassType<C>::ClassObject(vm), funcObj);
        }

    protected:
        static SQInteger ClassWeakref(HSQUIRRELVM vm)
        {
            sq->weakref(vm, -1);
            return 1;
        }

        // Initialize the required data structure for the class
        void InitClass()
        {
            ClassType<C>::CopyFunc(vm) = &A::Copy;

            // push the class
            sq->pushobject(vm, ClassType<C>::ClassObject(vm));

            // add the default constructor
            sq->pushstring(vm, _SC("constructor"), -1);
            sq->newclosure(vm, &A::New, 0);
            sq->newslot(vm, -3, false);

            // add the set table (static)
            HSQOBJECT &setTable = ClassType<C>::SetTable(vm);
            sq->resetobject(&setTable);
            sq->pushstring(vm, _SC("__setTable"), -1);
            sq->newtable(vm);
            sq->getstackobj(vm, -1, &setTable);
            sq->addref(vm, &setTable);
            sq->newslot(vm, -3, true);

            // add the get table (static)
            HSQOBJECT &getTable = ClassType<C>::GetTable(vm);
            sq->resetobject(&getTable);
            sq->pushstring(vm, _SC("__getTable"), -1);
            sq->newtable(vm);
            sq->getstackobj(vm, -1, &getTable);
            sq->addref(vm, &getTable);
            sq->newslot(vm, -3, true);

            // override _set
            sq->pushstring(vm, _SC("_set"), -1);
            sq->pushobject(vm, setTable); // Push the set table as a free variable
            sq->newclosure(vm, &sqVarSet, 1);
            sq->newslot(vm, -3, false);

            // override _get
            sq->pushstring(vm, _SC("_get"), -1);
            sq->pushobject(vm, getTable); // Push the get table as a free variable
            sq->newclosure(vm, &sqVarGet, 1);
            sq->newslot(vm, -3, false);

            // add weakref (apparently not provided by default)
            sq->pushstring(vm, _SC("weakref"), -1);
            sq->newclosure(vm, &Class::ClassWeakref, 0);
            sq->newslot(vm, -3, false);

            // pop the class
            sq->pop(vm, 1);
        }

        // Helper function used to bind getters and setters
        inline void BindAccessor(const SQChar *name, void *var, size_t varSize, SQFUNCTION func, HSQOBJECT table)
        {
            // Push the get or set table
            sq->pushobject(vm, table);
            sq->pushstring(vm, name, -1);

            // Push the variable offset as a free variable
            SQUserPointer varPtr = sq->newuserdata(vm, static_cast<SQUnsignedInteger>(varSize));
            memcpy(varPtr, var, varSize);

            // Create the accessor function
            sq->newclosure(vm, func, 1);

            // Add the accessor to the table
            sq->newslot(vm, -3, false);

            // Pop get/set table
            sq->pop(vm, 1);
        }
    };

    /**
        @tparam    C    class type to expose
        @tparam    B    base class type (must already be bound)
        @tparam A    allocator to use when instantiating and destroying class instances in Squirrel

        @remarks
        DefaultAllocator<C> is used if no allocator is specified. This should be sufficent for most classes
        but if specific behavior is desired it can be overridden. If the class should not be instantiated from
        Squirrel the NoConstructor allocator may be used.

        @remarks
        Classes in Squirrel are single-inheritance only, and as such Sqrat only allows for single inheritance as well
    */
    /// Exposes a C++ class with a base class to Squirrel
    template <class C, class B, class A = DefaultAllocator<C>>
    class DerivedClass : public Class<C, A>
    {
    public:
        DerivedClass(HSQUIRRELVM v = DefaultVM::Get()) : Class<C, A>(v, false)
        {
            if (!ClassType<C>::Initialized(v))
            {
                HSQOBJECT &classObj = ClassType<C>::ClassObject(v);
                sq->resetobject(&classObj);

                sq->pushobject(v, ClassType<B>::ClassObject(v));
                sq->newclass(v, true);
                sq->getstackobj(v, -1, &classObj);
                sq->addref(v, &classObj); // must addref before the pop!
                sq->pop(v, 1);

                InitDerivedClass(v);
                ClassType<C>::Initialized(v) = true;
            }
        }

    protected:
        void InitDerivedClass(HSQUIRRELVM vm)
        {
            ClassType<C>::CopyFunc(vm) = &A::Copy;

            // push the class
            sq->pushobject(vm, ClassType<C>::ClassObject(vm));

            // add the default constructor
            sq->pushstring(vm, _SC("constructor"), -1);
            sq->newclosure(vm, &A::New, 0);
            sq->newslot(vm, -3, false);

            // clone the base classes set table (static)
            HSQOBJECT &setTable = ClassType<C>::SetTable(vm);
            sq->resetobject(&setTable);
            sq->pushobject(vm, ClassType<B>::SetTable(vm));
            sq->pushstring(vm, _SC("__setTable"), -1);
            sq->clone(vm, -2);
            sq->remove(vm, -3);
            sq->getstackobj(vm, -1, &setTable);
            sq->addref(vm, &setTable);
            sq->newslot(vm, -3, true);

            // clone the base classes get table (static)
            HSQOBJECT &getTable = ClassType<C>::GetTable(vm);
            sq->resetobject(&getTable);
            sq->pushobject(vm, ClassType<B>::GetTable(vm));
            sq->pushstring(vm, _SC("__getTable"), -1);
            sq->clone(vm, -2);
            sq->remove(vm, -3);
            sq->getstackobj(vm, -1, &getTable);
            sq->addref(vm, &getTable);
            sq->newslot(vm, -3, true);

            // override _set
            sq->pushstring(vm, _SC("_set"), -1);
            sq->pushobject(vm, setTable); // Push the set table as a free variable
            sq->newclosure(vm, sqVarSet, 1);
            sq->newslot(vm, -3, false);

            // override _get
            sq->pushstring(vm, _SC("_get"), -1);
            sq->pushobject(vm, getTable); // Push the get table as a free variable
            sq->newclosure(vm, sqVarGet, 1);
            sq->newslot(vm, -3, false);

            // add weakref (apparently not provided by default)
            sq->pushstring(vm, _SC("weakref"), -1);
            sq->newclosure(vm, &Class<C, A>::ClassWeakref, 0);
            sq->newslot(vm, -3, false);

            // pop the class
            sq->pop(vm, 1);
        }
    };

}

#endif
