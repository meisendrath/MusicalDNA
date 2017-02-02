//
//  LElement.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _LELEMENT_H_
#define _LELEMENT_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>

#include "TMessage.h"
#include "LMList.h"

class GameState;           // fwd
class LElement;         // fwd
class TMessage;         // fwd
class TXMLControlItem;  // fwd
class TAtlas;           // fwd

#ifdef SHOW_DEBUG_BOXES
extern int gScreenHeight;
#endif // SHOW_DEBUG_BOXES

//LElement: scene graph element base class showing interfaces for creation, tick, touch, render, message_receive, message_send; has & can show extent and name
//  - rendering mechanism override: draw [empty underlay, ]subclass, then overlay
class LElement {
public:
                                LElement();
	virtual						~LElement() { gElements.remove(this); }
public:
	std::string					name;							// instance name
    int							ident;							// old-school id (NOT USED)
    float						extent[4];
    bool						visible;
public:
	virtual void				super_render(double now) {
        render_underlay(now);
        render(now);
        render_overlay(now);
    }
    virtual void				override_extent(float x, float y, float sx, float sy) {
        extent[0] = x;
        extent[1] = y;
        extent[2] = sx;
        extent[3] = sy;
    }
public:		// specialize
	virtual void				update(double now, float frame_time) { }
	virtual TActiveTouch		*touch(int phase, double now, TActiveTouch *t, bool is_inside) { return NULL; }	// return t or a new subclass to "claim" the touch (and block further traversal)
	virtual bool				far_touch(TActiveTouch *t, float expand) {	// return true to for a looser hit test
        bool is_inside = (t->cx >= (extent[0]-expand)) && (t->cx <= (extent[0]+extent[2]+2*expand)) && (t->cy >= (extent[1]-expand)) && (t->cy <= (extent[1]+extent[3]+2*expand));
        return is_inside;
	}
	virtual TActiveTouch		*test_touch(int phase, double now, TActiveTouch *t) {	// return t or a new subclass to "claim" the touch (and block further traversal) (subclass not tested for LScrollingUI or LMap)
        float expand = 0;
        bool is_inside = (t->cx >= (extent[0]-expand)) && (t->cx <= (extent[0]+extent[2]+2*expand)) && (t->cy >= (extent[1]-expand)) && (t->cy <= (extent[1]+extent[3]+2*expand));
        return touch(phase, now, t, is_inside);
	}
	virtual void				render_underlay(double now) {
#ifdef SHOW_DEBUG_BOXES
        //render_rect(extent[0], extent[1], extent[2], extent[3], 1, 1, 1, 0.125f);
#endif // SHOW_DEBUG_BOXES
    }
	virtual void				render(double now) { }
	virtual void				render_overlay(double now);
	virtual bool				message(const std::string &msg, float arg1, float arg2) { return false; }
	virtual bool				message(const TMessage &msg) { return message(msg.msg, msg.arg1, msg.arg2); }
    static void					send(const char *msg, float arg=0, float arg2=0);
    static void					send_NO_QUEUE(const char *msg, float arg=0, float arg2=0);
    static void					send(const TMessage &msg);
    static void					send_NO_QUEUE(const TMessage &msg);
    virtual void				change_state(const char *new_state_name, float arg=0);
	virtual void				self_setup_on_enter(GameState *parent_state, void *arg=NULL) { }
	virtual void				cleanup_on_exit() { }
	virtual void				setup_center_square();
public:		// named global list of elements for caching and sharing
	static LMList<LElement>		gElements;
	static LMList<LElement>		gScene;
public:
    static int resolve_attribute_name(TXMLControlItem *parent_item, const char *msg, float *rbds, std::string &result);
    static void process_layer_name(GameState *parent_state, TXMLControlItem *parent_item, float *rbds, bool *is_visible, const char *input_fn, std::string &output_fn);
    static void read_psd_work(GameState *parent_state, TXMLControlItem *parent_item, TAtlas **ppsd_file, int ident, LMList<LElement> &dest_list, bool ignore_aspect_ratio, int no_screen_scale, const char *psd_name, const char *elm_prefix=NULL);
};

#define LE_GET_OR_CREATE(var, _name, creation_expression, init_expression) { var = LElement::gElements.get_silent(_name); if (var == NULL) { var = creation_expression; var->name = _name; init_expression; } }


#endif // !_LELEMENT_H_
