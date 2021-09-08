/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: slider.c
 *
 */

/* Slider */

#include "slider.h"
#include "slider.inl"
#include "cell.inl"
#include "component.inl"
#include "gui.inl"
#include "guicontexth.inl"
#include "obj.inl"

#include "cassert.h"
#include "event.h"
#include "ptr.h"
#include "s2d.h"

struct _slider_t
{
    GuiComponent component;
    dtype_t dtype;
    uint32_t intmax;
    real32_t current_pos;
    S2Df size;
    ResId ttipid;
    slider_flag_t flags;
    fsize_t knob_size;
    Listener *OnMoved;
};

/*---------------------------------------------------------------------------*/

void _slider_destroy(Slider **slider)
{
    cassert_no_null(slider);
    cassert_no_null(*slider);
    _component_destroy_imp(&(*slider)->component);
    listener_destroy(&(*slider)->OnMoved);
    obj_delete(slider, Slider);
}

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_int_pos(const real32_t pos, const uint32_t intmax)
{
    uint32_t ipos = (uint32_t)((pos + .001f) * (intmax + 1));
    if (ipos > intmax)
        return intmax;
    else
        return ipos;
}

/*---------------------------------------------------------------------------*/

static void i_OnSliderMoved(Slider *slider, Event *e)
{    
    const EvSlider *p = event_params(e, EvSlider);
    uint32_t cip = UINT32_MAX;
    uint32_t nip = UINT32_MAX;
    bool_t moved_event = TRUE;
    cassert_no_null(slider);
    cassert_no_null(slider->component.context);
    cassert(event_sender_imp(e, NULL) == slider->component.ositem);
    cassert(event_type(e) == ekEVSLIDER);
    cassert(p->incr == 0);
    cassert(p->step == UINT32_MAX);

    // Discrete slider
    if (slider->intmax != UINT32_MAX)
    {
        real32_t pos = 0;
        cip = i_int_pos(slider->current_pos, slider->intmax);
        nip = i_int_pos(p->pos, slider->intmax);
        pos = (real32_t)nip * (1.f / slider->intmax);
        if (cip != nip)
        {
            moved_event = TRUE;
            ((EvSlider*)p)->incr = p->pos - slider->current_pos;
            ((EvSlider*)p)->step = nip;
        }

        slider->component.context->func_slider_set_position(slider->component.ositem, pos);
        slider->current_pos = pos;
    }
    else
    {
        if (p->pos != slider->current_pos)
        {
            moved_event = TRUE;
            ((EvSlider*)p)->incr = p->pos - slider->current_pos;
            slider->current_pos = p->pos;
        }
    }

    if (slider->dtype == ekDTYPE_UINT32)
        _cell_upd_uint32(slider->component.parent, nip);
    else if (slider->dtype == ekDTYPE_REAL32)
        _cell_upd_norm_real32(slider->component.parent, p->pos);

    if (slider->OnMoved != NULL && moved_event == TRUE)
        listener_pass_event(slider->OnMoved, e, slider, Slider);
}

/*---------------------------------------------------------------------------*/

static Slider *i_create(const slider_flag_t flags, const fsize_t knob_size)
{
    Slider *slider = NULL;
    void *ositem = NULL;
    const GuiContext *context = gui_context_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_slider_create);
    cassert_no_nullf(context->func_slider_OnMoved);
    slider = obj_new0(Slider);
    ositem = context->func_slider_create((const enum_t)flags);
    _component_init(&slider->component, context, ekGUI_COMPONENT_SLIDER, &ositem);
    slider->dtype = ekDTYPE_UNKNOWN;
    slider->intmax = UINT32_MAX;
    slider->current_pos = 0;
    slider->size = s2df(-1, -1);
    slider->flags = flags;
    slider->knob_size = knob_size;
    slider->OnMoved = NULL;
    context->func_slider_OnMoved(slider->component.ositem, obj_listener(slider, i_OnSliderMoved, Slider));
    return slider;
}

/*---------------------------------------------------------------------------*/

Slider *slider_create(void)
{
    return i_create(ekSLHORZ, ekREGULAR);
}

/*---------------------------------------------------------------------------*/

Slider *slider_vertical(void)
{
    return i_create(ekSLVERT, ekREGULAR);
}

/*---------------------------------------------------------------------------*/

void slider_OnMoved(Slider *slider, Listener *listener)
{
    cassert_no_null(slider);
    listener_update(&slider->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void slider_tooltip(Slider *slider, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(slider);
    if (text != NULL)
        ltext = _gui_respack_text(text, &slider->ttipid);
    slider->component.context->func_set_tooltip[ekGUI_COMPONENT_SLIDER](slider->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void slider_steps(Slider *slider, const uint32_t steps)
{
    cassert_no_null(slider);
    cassert(steps > 1); 
    if (steps == UINT32_MAX)
        slider->intmax = UINT32_MAX;
    else
        slider->intmax = steps - 1;

    slider->component.context->func_slider_set_tickmarks(slider->component.ositem, steps, FALSE);
}

/*---------------------------------------------------------------------------*/

void slider_value(Slider *slider, const real32_t value)
{
    real32_t v = value;
    cassert_no_null(slider);
    cassert_no_null(slider->component.context);
    cassert_no_nullf(slider->component.context->func_slider_set_position);
    
    if (v < 0.f)
        v = 0.f;
    else if (v > 1.f)
        v = 1.f;
    
    slider->current_pos = v;
    slider->component.context->func_slider_set_position(slider->component.ositem, v);
}

/*---------------------------------------------------------------------------*/

real32_t slider_get_value(const Slider *slider)
{
    cassert_no_null(slider);
    return slider->current_pos;
    //cassert_no_null(slider->component.context);
    //cassert_no_nullf(slider->component.context->func_slider_get_position);
    //return slider->component.context->func_slider_get_position(slider->component.ositem);
}

/*---------------------------------------------------------------------------*/

void _slider_dimension(Slider *slider, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(slider);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    if (slider->size.width < 0)
    {
        cassert_no_nullf(slider->component.context->func_slider_bounds);
        slider->component.context->func_slider_bounds(slider->component.ositem, 100, slider->knob_size, &slider->size.width, &slider->size.height);
    }

    if (i == 0)
    {
        *dim0 = slider->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = slider->size.height;
    }
}

/*---------------------------------------------------------------------------*/

bool_t _slider_is_horizontal(const Slider *slider)
{
    cassert_no_null(slider);
    return (bool_t)(slider_type(slider->flags) == ekSLHORZ);
}

/*---------------------------------------------------------------------------*/

void _slider_set_dtype_real32(Slider *slider)
{
    cassert_no_null(slider);
    cassert(slider->dtype == ekDTYPE_UNKNOWN);
    slider->dtype = ekDTYPE_REAL32;
    slider->component.context->func_slider_set_tickmarks(slider->component.ositem, UINT32_MAX, FALSE);
}

/*---------------------------------------------------------------------------*/

void _slider_set_dtype_uint32(Slider *slider, const uint32_t intmax)
{
    cassert_no_null(slider);
    cassert(slider->dtype == ekDTYPE_UNKNOWN);
    slider->dtype = ekDTYPE_UINT32;
    slider->intmax = intmax;
    slider->component.context->func_slider_set_tickmarks(slider->component.ositem, intmax + 1, FALSE);
}

/*---------------------------------------------------------------------------*/

void _slider_real32(Slider *slider, const real32_t value)
{
    cassert_no_null(slider);
    cassert(slider->dtype == ekDTYPE_REAL32);
    cassert(value >= 0.f && value <= 1.f);
    slider->current_pos = value;
    slider->component.context->func_slider_set_position(slider->component.ositem, value);
}

/*---------------------------------------------------------------------------*/

void _slider_uint32(Slider *slider, const uint32_t value)
{
    real32_t rvalue = 0;
    cassert_no_null(slider);
    cassert(slider->dtype == ekDTYPE_UINT32);
    cassert(value <= slider->intmax);
    rvalue = (real32_t)value * (1.f / slider->intmax);
    slider->component.context->func_slider_set_position(slider->component.ositem, rvalue);
}

/*
void slider_set_tickmarks(Slider *slider, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top);
void slider_set_tickmarks(Slider *slider, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top)
{
    cassert_no_null(slider);
    cassert_no_null(slider->component.context);
    cassert_no_nullf(slider->component.context->func_slider_set_tickmarks);
    slider->component.context->func_slider_set_tickmarks(slider->component.ositem, num_tickmarks, tickmarks_at_left_top);
}*/

