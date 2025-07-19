//
// SqratTable: Table Binding
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
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//

#if !defined(_SCRAT_TABLE_H_)
#define _SCRAT_TABLE_H_

#include <squirrel.h>
#include <sqmodule.h>
extern HSQUIRRELVM v;
extern HSQAPI sq;

#include <string.h>

#include "sqratObject.h"
#include "sqratFunction.h"
#include "sqratGlobalMethods.h"

namespace Sqrat
{

    class TableBase : public Object
    {
    public:
        TableBase(HSQUIRRELVM v = DefaultVM::Get()) : Object(v, true)
        {
        }

        TableBase(const Object &obj) : Object(obj)
        {
        }

        // Bind a Table or Class to the Table (Can be used to facilitate Namespaces)
        // Note: Bind cannot be called "inline" like other functions because it introduces order-of-initialization bugs.
        void Bind(const SQChar *name, Object &obj)
        {
            sq->pushobject(vm, GetObject());
            sq->pushstring(vm, name, -1);
            sq->pushobject(vm, obj.GetObject());
            sq->newslot(vm, -3, false);
            sq->pop(vm, 1); // pop table
        }

        // Bind a raw Squirrel closure to the Table
        TableBase &SquirrelFunc(const SQChar *name, SQFUNCTION func)
        {
            sq->pushobject(vm, GetObject());
            sq->pushstring(vm, name, -1);
            sq->newclosure(vm, func, 0);
            sq->newslot(vm, -3, false);
            sq->pop(vm, 1); // pop table

            return *this;
        }

        //
        // Variable Binding
        //

        template <class V>
        TableBase &SetValue(const SQChar *name, const V &val)
        {
            BindValue<V>(name, val, false);
            return *this;
        }
        template <class V>
        TableBase &SetValue(const SQInteger index, const V &val)
        {
            BindValue<V>(index, val, false);
            return *this;
        }

        template <class V>
        TableBase &SetInstance(const SQChar *name, V *val)
        {
            BindInstance<V>(name, val, false);
            return *this;
        }

        template <class F>
        TableBase &Func(const SQChar *name, F method)
        {
            BindFunc(name, &method, sizeof(method), SqGlobalFunc(method));
            return *this;
        }

        template <class F>
        TableBase &Func(const SQChar *name, F method, SQInteger paramCount, const SQChar *params)
        {
            BindFunc(name, &method, sizeof(method), SqGlobalFunc(method), paramCount, params);
            return *this;
        }

        template <class F>
        TableBase &Overload(const SQChar *name, F method)
        {
            BindOverload(name, &method, sizeof(method), SqGlobalFunc(method), SqOverloadFunc(method), SqGetArgCount(method));
            return *this;
        }

        template <class F>
        TableBase &Overload(const SQChar *name, F method, SQInteger paramCount, const SQChar *params)
        {
            BindOverload(name, &method, sizeof(method), SqGlobalFunc(method), SqOverloadFunc(method), SqGetArgCount(method), paramCount, params);
            return *this;
        }

        //
        // Function Calls
        //

        Function GetFunction(const SQChar *name)
        {
            HSQOBJECT funcObj;
            sq->pushobject(vm, GetObject());
            sq->pushstring(vm, name, -1);
            if (SQ_FAILED(sq->get(vm, -2)))
            {
                sq->pushnull(vm);
            }
            sq->getstackobj(vm, -1, &funcObj);
            Function ret(vm, GetObject(), funcObj); // must addref before the pop!

            sq->pop(vm, 2);

            return ret;
        }
        Function GetFunction(const SQInteger index)
        {
            HSQOBJECT funcObj;
            sq->pushobject(vm, GetObject());
            sq->pushinteger(vm, index);
            if (SQ_FAILED(sq->get(vm, -2)))
            {
                sq->pushnull(vm);
            }
            sq->getstackobj(vm, -1, &funcObj);
            Function ret(vm, GetObject(), funcObj);
            sq->pop(vm, 2);

            return ret;
        }
    };

    class Table : public TableBase
    {
    public:
        Table(HSQUIRRELVM v = DefaultVM::Get()) : TableBase(v)
        {
            sq->newtable(vm);
            sq->getstackobj(vm, -1, &obj);
            sq->addref(vm, &obj);
            sq->pop(vm, 1);
        }
        Table(const Object &obj) : TableBase(obj)
        {
        }
    };

    //
    // Root Table
    //

    class RootTable : public TableBase
    {
    public:
        RootTable(HSQUIRRELVM v = DefaultVM::Get()) : TableBase(v)
        {
            sq->pushroottable(vm);
            sq->getstackobj(vm, -1, &obj);
            sq->addref(vm, &obj);
            sq->pop(v, 1); // pop root table
        }
    };
}

#endif
