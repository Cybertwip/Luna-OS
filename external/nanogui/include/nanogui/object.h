/*
    nanogui/object.h -- Object base class with support for reference counting

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/common.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class Object object.h nanogui/object.h
 *
 * \brief Object base class with intrusive reference counting
 *
 * The Object class provides a convenient foundation of a class hierarchy that
 * will ease lifetime and ownership-related issues whenever Python bindings are
 * involved.
 *
 * Internally, its constructor sets the `m_state` field to `1`, which indicates
 * that the instance is owned by C++. Bits 2..63 of this field are used to
 * store the actual reference count value. The `inc_ref()` and `dec_ref()`
 * functions can be used to increment or decrement this reference count. When
 * `dec_ref()` removes the last reference, the instance will be deallocated
 * using a `delete` expression handled using a polymorphic destructor.
 *
 * When a subclass of `Object` is constructed to Python or returned from C++ to
 * Python, nanobind will invoke `Object::set_self_py()`, which hands ownership
 * over to Python/nanobind. Any remaining references will be moved from the
 * `m_state` field to the Python reference count. In this mode, `inc_ref()` and
 * `dec_ref()` wrap Python reference counting primitives (`Py_INCREF()` /
 * `Py_DECREF()`) which must be made available by calling the function
 * `object_init_py` once during module initialization. Note that the `m_state`
 * field is also used to store a pointer to the `PyObject *`. Python instance
 * pointers are always aligned (i.e. bit 1 is zero), which disambiguates
 * between the two possible configurations.
 *
 * Within C++, the RAII helper class `ref` (defined below) can be used to keep
 * instances alive. This removes the need to call the `inc_ref()` / `dec_ref()`
 * functions explicitly.
 *
 * ```
 * {
 *    ref<MyClass> inst = new MyClass();
 *    inst->my_function();
 *    ...
 * } // end of scope, 'inst' automatically deleted if no longer referenced
 * ```
 *
 * A separate optional file ``object_py.h`` provides a nanobind type caster
 * to bind functions taking/returning values of type `ref<T>`.
 */
class NANOGUI_EXPORT Object {
public:
    /// Default constructor
    Object() = default;

    // Polymorphic default destructor
    virtual ~Object();

    /* The following move/assignment constructors/operators are no-ops. They
       intentionally do not change the reference count field (m_state) that
       is associated with a fixed address in memory */
    Object(const Object &) : Object() { }
    Object(Object &&) : Object() { }
    Object &operator=(const Object &) { return *this; }
    Object &operator=(Object &&) { return *this; }
};

/**
 * \class ref object.h nanogui/object.h
 *
 * \brief Reference counting helper.
 *
 * ``ref`` is a simple RAII wrapper class that stores a pointer to a subclass
 * of ``Object``. It takes care of increasing and decreasing the reference
 * count of the underlying instance. When the last reference goes out of scope,
 * the associated object will be deallocated.
 *
 * The advantage over C++ solutions such as ``std::shared_ptr`` is that
 * the reference count is very compactly integrated into the base object
 * itself.
 */


NAMESPACE_END(nanogui)
