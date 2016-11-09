/*!
 * \file painter_packed_value.hpp
 * \brief file painter_packed_value.hpp
 *
 * Copyright 2016 by Intel.
 *
 * Contact: kevin.rogovin@intel.com
 *
 * This Source Code Form is subject to the
 * terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with
 * this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * \author Kevin Rogovin <kevin.rogovin@intel.com>
 *
 */


#pragma once


#include <fastuidraw/util/reference_counted.hpp>
#include <fastuidraw/painter/packing/painter_item_matrix.hpp>
#include <fastuidraw/painter/packing/painter_clip_equations.hpp>
#include <fastuidraw/painter/brush/painter_brush.hpp>
#include <fastuidraw/painter/painter_shader_data.hpp>

namespace fastuidraw
{
  class PainterPackedValuePool;
  class PainterPacker;
  class PainterShaderSet;

  template<typename T>
  class PainterPackedValue;

/*!\addtogroup Painter
  @{
 */

  /*!
    (Private) base class used for PainterPackedValue
   */
  class PainterPackedValueBase
  {
  private:
    friend class PainterPacker;
    friend class PainterPackedValuePool;
    template<typename> friend class PainterPackedValue;

    template<typename T, typename has_derived_value>
    class derived_value_type_builder
    {};

    template<typename T>
    class derived_value_type_builder<T, false_type>
    {
    public:
      typedef T type;
    };

    template<typename T>
    class derived_value_type_builder<T, true_type>
    {
    public:
      typedef typename T::derived_value_type type;
    };

    template<typename T, typename has_value>
    class derived_value_type
    {};

    template<typename T>
    class derived_value_type<T, true_type>
    {
    public:
      typedef false_type has_derived_value;
      typedef typename derived_value_type_builder<T, has_derived_value>::type type;
    };

    template<typename T>
    class derived_value_type<T, false_type>
    {
    public:
      typedef typename T::packed_value_has_derived_value has_derived_value;
      typedef typename derived_value_type_builder<T, has_derived_value>::type type;
    };

    template<typename T>
    class derived_value_t
    {
    public:
      typedef typename T::packed_value_has_value packed_value_has_value;
      typedef typename derived_value_type<T, packed_value_has_value>::type type;
      typedef typename derived_value_type<T, packed_value_has_value>::has_derived_value has_derived_value;
    };

    ~PainterPackedValueBase();
    PainterPackedValueBase(void);
    PainterPackedValueBase(const PainterPackedValueBase&);

    explicit
    PainterPackedValueBase(void*);

    void
    operator=(const PainterPackedValueBase&);

    const void*
    raw_value(void) const;

    unsigned int
    alignment_packing(void) const;

    void *m_d;
  };

  /*!
    A PainterPackedValue represents a handle to an object that stores
    packed state data and tracks if that underlying data is already
    copied to PainterDraw::m_store. If already on a store, then rather
    than copying the data again, the data is reused. The object behind
    the handle is NOT thread safe. In addition, the underlying reference
    count is not either. Hence any access (even dtor, copy ctor and
    equality operator) on a fixed object cannot be done from multiple
    threads simutaneously. A fixed PainterPackedValue can be used by
    different Painter (and PainterPacker) objects subject to the condition
    that the data store alignment (see
    PainterPacker::Configuration::alignment()) is the same for each of
    these objects.
   */
  template<typename T>
  class PainterPackedValue:PainterPackedValueBase
  {
  private:
    typedef const T& (PainterPackedValue::*unspecified_bool_type)(void) const;
    typedef typename T::packed_value_has_value has_value;
    typedef typename PainterPackedValueBase::derived_value_t<T>::type derived_type;
    typedef typename PainterPackedValueBase::derived_value_t<T>::has_derived_value has_derived_value;
  public:
    /*!
      Ctor, initializes handle to NULL, i.e.
      no underlying value object.
     */
    PainterPackedValue(void)
    {}

    /*!
      Returns the value to which the handle points.
      If the handle is not-value, then asserts.
      If T::packed_value_has_value is not true_type,
      then asserts.
     */
    const T&
    value(void) const
    {
      const T *p;
      assert(this->m_d);
      assert(has_value());
      assert(!has_derived_value());
      p = reinterpret_cast<const T*>(this->raw_value());
      return *p;
    }

    /*!
      Returns the derived value to which the handle
      points. If the handle is not-value, then asserts.
      If T::packed_value_has_value is not false_type,
      then asserts. If T::has_derived_value is not true_type
      then asserts.
     */
    const derived_type&
    derived_value(void) const
    {
      const derived_type *p;
      assert(this->m_d);
      assert(!has_value());
      assert(has_derived_value());
      p = reinterpret_cast<const derived_type*>(this->raw_value());
      return *p;
    }

    /*!
      Returns the alignment packing for PainterPackedValue
      object (see PainterPacker::Configuration::alignment());
      If the PainterPackedValue represents a NULL handle then
      returns 0.
     */
    unsigned int
    alignment_packing(void) const
    {
      return this->m_d->alignment_packing();
    }

    /*!
      Used to allow using object as a boolean without
      accidentally converting to a boolean (since it
      returns a pointer to a function).
     */
    operator
    unspecified_bool_type(void) const
    {
      return this->m_d ? &PainterPackedValue::value : 0;
    }

    /*!
      Comparison operator to underlying object
      storing value.
      \param rhs handle to which to compare
     */
    bool
    operator==(const PainterPackedValue &rhs) const
    {
      return this->m_d == rhs.m_d;
    }

    /*!
      Comparison operator to underlying object
      storing value.
      \param rhs handle to which to compare
     */
    bool
    operator!=(const PainterPackedValue &rhs) const
    {
      return this->m_d != rhs.m_d;
    }

    /*!
      Comparison operator to underlying object
      storing value.
      \param rhs handle to which to compare
     */
    bool
    operator<(const PainterPackedValue &rhs) const
    {
      return this->m_d < rhs.m_d;
    }

    /*!
      Pointer to opaque data of PainterPackedValueBase, used
      internally by fastuidraw. Do not touch!
     */
    void*
    opaque_data(void) const
    {
      return this->m_d;
    }

  private:
    friend class PainterPacker;
    friend class PainterPackedValuePool;

    explicit
    PainterPackedValue(void *d):
      PainterPackedValueBase(d)
    {}
  };

  /*!
    A PainterPackedValuePool can be used to create PainterPackedValue
    objects. Just like PainterPackedValue, PainterPackedValuePool is
    NOT thread safe, as such it is not a safe operation to use the
    same PainterPackedValuePool object from multiple threads at the
    same time. A fixed PainterPackedValuePool can create PainterPackedValue
    objects used by different Painter (and PainterPacker) objects subject
    to the condition that the data store alignment (see
    PainterPacker::Configuration::alignment()) is the same for each of
    these objects.
   */
  class PainterPackedValuePool:noncopyable
  {
  public:
    /*!
      Ctor.
      \param shaders default shaders to use when a stage of a PainterBrush
                     is named by an enumeration instead of by specifying a
                     shader.
      \param painter_alignment the alignment to create packed data, see
                                PainterPacker::Configuration::alignment()
     */
    explicit
    PainterPackedValuePool(const PainterShaderSet &shaders, int painter_alignment);

    ~PainterPackedValuePool();

    /*!
      Create and return a PainterPackedValue<PainterBrush>
      object for the value of a PainterBrush object.
      \param value data to pack into returned PainterPackedValue
     */
    PainterPackedValue<PainterBrush>
    create_packed_value(const PainterBrush &value);

    /*!
      Create and return a PainterPackedValue<PainterClipEquations>
      object for the value of a PainterClipEquations object.
      \param value data to pack into returned PainterPackedValue
     */
    PainterPackedValue<PainterClipEquations>
    create_packed_value(const PainterClipEquations &value);

    /*!
      Create and return a PainterPackedValue<PainterItemMatrix>
      object for the value of a PainterItemMatrix object.
      \param value data to pack into returned PainterPackedValue
     */
    PainterPackedValue<PainterItemMatrix>
    create_packed_value(const PainterItemMatrix &value);

    /*!
      Create and return a PainterPackedValue<PainterItemShaderData>
      object for the value of a PainterItemShaderData object.
      \param value data to pack into returned PainterPackedValue
     */
    PainterPackedValue<PainterItemShaderData>
    create_packed_value(const PainterItemShaderData &value);

    /*!
      Create and return a PainterPackedValue<PainterBlendShaderData>
      object for the value of a PainterBlendShaderData object.
      \param value data to pack into returned PainterPackedValue
     */
    PainterPackedValue<PainterBlendShaderData>
    create_packed_value(const PainterBlendShaderData &value);

  private:
    void *m_d;
  };

/*! @} */

}
