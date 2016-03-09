/*!
 * \file painter_backend.hpp
 * \brief file painter_backend.hpp
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


#include <fastuidraw/text/glyph_atlas.hpp>
#include <fastuidraw/image.hpp>
#include <fastuidraw/colorstop_atlas.hpp>
#include <fastuidraw/painter/packing/painter_draw.hpp>
#include <fastuidraw/painter/painter_shader.hpp>


namespace fastuidraw
{
/*!\addtogroup PainterPacking
  @{
 */

  /*!
    A PainterBackend is an interface that defines the API-specific
    elements to implement Painter:
   */
  class PainterBackend:public reference_counted<PainterBackend>::default_base
  {
  public:

    /*!
      A Configuration holds how data should be set to a
      PainterBackend
     */
    class Configuration
    {
    public:
      /*!
        Ctor.
       */
      Configuration(void);

      /*!
        Copy ctor.
       */
      Configuration(const Configuration &obj);

      ~Configuration();

      /*!
        assignment operator
       */
      Configuration&
      operator=(const Configuration &obj);

      /*!
        Bits that are up in brush_shader_mask(void) that change
        in PainterBrush::shader() trigger a call to
        PainterDrawCommand::draw_break().
      */
      uint32_t
      brush_shader_mask(void) const;

      /*!
        Specify the value returned by brush_shader_mask(void) const,
        default value is 0
        \param v value
       */
      Configuration&
      brush_shader_mask(uint32_t v);

      /*!
        Specifies the alignment in units of generic_data for
        packing of seperately accessible entries of generic data
        in PainterDrawCommand::m_store.
       */
      int
      alignment(void) const;

      /*!
        Specify the value returned by alignment(void) const,
        default value is 4
        \param v value
       */
      Configuration&
      alignment(int v);

    private:
      void *m_d;
    };

    /*!
      Ctor.
      \param glyph_atlas GlyphAtlas for glyphs drawn by the PainterBackend
      \param image_atlas ImageAtlas for images drawn by the PainterBackend
      \param colorstop_atlas ColorStopAtlas for color stop sequences drawn by the PainterBackend
      \param config Configuration for how to pack data to PainterBackend
      \param default_shaders shaders are to NOT be registed yet to any
                             backend; they will be registered laziy
                             at default_shaders().
     */
    PainterBackend(GlyphAtlas::handle glyph_atlas,
                   ImageAtlas::handle image_atlas,
                   ColorStopAtlas::handle colorstop_atlas,
                   const Configuration &config,
                   const PainterShaderSet &default_shaders);

    virtual
    ~PainterBackend();

    /*!
      Informs the PainterBackend what the resolution of
      the target surface is.
      \param w width of target resolution
      \param h height of target resolution
     */
    virtual
    void
    target_resolution(int w, int h) = 0;

    /*!
      Returns a handle to the GlyphAtlas of this
      PainterBackend. All glyphs used by this
      PainterBackend must live on glyph_atlas().
     */
    const GlyphAtlas::handle&
    glyph_atlas(void);

    /*!
      Returns a handle to the ImageAtlas of this
      PainterBackend. All images used by all brushes
      of this PainterBackend must live on image_atlas().
     */
    const ImageAtlas::handle&
    image_atlas(void);

    /*!
      Returns a handle to the ColorStopAtlas of this
      PainterBackend. All color stops used by all brushes
      of this PainterBackend must live on colorstop_atlas().
     */
    const ColorStopAtlas::handle&
    colorstop_atlas(void);

    /*!
      Returns the Configuration passed in the ctor.
     */
    const Configuration&
    configuration(void) const;

    /*!
      Called by Painter to indicate the start of a painting session.
     */
    virtual
    void
    on_begin(void) = 0;

    /*!
      Called by Painter to indicate the end of a painting session.
     */
    virtual
    void
    on_end(void) = 0;

    /*!
      Called by Painter just before calling
      PainterDrawCommand::draw() on a sequence of
      PainterDrawCommand objects who have had
      their PainterDrawCommand::unmap() routine
      called.
     */
    virtual
    void
    on_pre_draw(void) = 0;

    /*!
      "Map" a PainterDrawCommand for filling of data.
     */
    virtual
    PainterDrawCommand::const_handle
    map_draw_command(void) = 0;

    /*!
      Registers a vertex shader for use. Must not be called within a
      on_begin()/on_end() pair.
     */
    void
    register_vert_shader(const PainterShader::handle &shader);

    /*!
      Registers a fragment shader for use. Must not be called within
      a on_begin()/on_end() pair.
    */
    void
    register_frag_shader(const PainterShader::handle &shader);

    /*!
      Registers a blend shader for use. Must not be called within
      a on_begin()/on_end() pair.
    */
    void
    register_blend_shader(const PainterShader::handle &shader);

    /*!
      Provided as a conveniance, equivalent to
      \code
      register_vert_shader(p.vert_shader());
      register_frag_shader(p.frag_shader());
      \endcode
      \param p PainterItemShader hold shaders to register
     */
    void
    register_shader(const PainterItemShader &p);

    /*!
      Provided as a conveniance, equivalent to
      \code
      register_shader(p.non_aa_shader());
      register_shader(p.aa_shader_pass1());
      register_shader(p.aa_shader_pass2());
      \endcode
      \param p PainterStrokeShader hold shaders to register
     */
    void
    register_shader(const PainterStrokeShader &p);

    /*!
      Register each of the PainterShader::handle
      in a PainterGlyphShader.
     */
    void
    register_shader(const PainterGlyphShader &p);

    /*!
      Register each of the PainterShader::handle
      in a PainterBlendShaderSet.
     */
    void
    register_shader(const PainterBlendShaderSet &p);

    /*!
      Register each of the shaders in a PainterShaderSet.
     */
    void
    register_shader(const PainterShaderSet &p);

    /*!
      Returns the PainterShaderSet for the backend.
      Returned values will already be registerd by the
      backend.
     */
    const PainterShaderSet&
    default_shaders(void);

  protected:
    /*!
      To be implemented by a derived class to take into use
      a vertex shader. Typically this means inserting the
      the vertex shader into a large uber shader. Returns
      the PainterShader::Tag to be used by the backend
      to identify the shader.
     */
    virtual
    PainterShader::Tag
    absorb_vert_shader(const PainterShader::handle &shader) = 0;

    /*!
      To be implemented by a derived class to take into use
      a fragment shader. Typically this means inserting the
      the fragment shader into a large uber shader. Returns
      the PainterShader::Tag to be used by the backend
      to identify the shader.
     */
    virtual
    PainterShader::Tag
    absorb_frag_shader(const PainterShader::handle &shader) = 0;

    /*!
      To be implemented by a derived class to take into use
      a blend shader. Typically this means inserting the
      the blend shader into a large uber shader. Returns
      the PainterShader::Tag to be used by the backend
      to identify the shader.
     */
    virtual
    PainterShader::Tag
    absorb_blend_shader(const PainterShader::handle &shader) = 0;

  private:
    void *m_d;
  };
/*! @} */

}