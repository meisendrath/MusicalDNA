//
//  LMList.h
//
//  Created by Burt Sloane
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#ifndef _LMLIST_H_
#define _LMLIST_H_

#include "config.h"
#include "common.h"

#include <vector>
#include <string>


class LElement;         // fwd


// LMList: a simple list container, adding search by name or ident of the list
template <class T> class LMList {
public:
	std::vector<T *>			items;
    void						add(T *e) { items.push_back(e); }
    void						remove(T *e) { for (int i=0; i<items.size(); i++) if (e == items[i]) { items.erase(items.begin() + i); break; } }
    void						remove_by_name(const char *_name) { for (int i=0; i<items.size(); i++) if (strcmp(_name, items[i]->name.c_str()) == 0) { items.erase(items.begin() + i); break; } }
    void						remove_all() { items.resize(0); }
    T							*get_silent(const char *_name) { for (int i=0; i<items.size(); i++) if (strcmp(_name, items[i]->name.c_str()) == 0) { return items[i]; } return NULL; }
    T							*get_by_prefix(const char *_nameprefix) { for (int i=0; i<items.size(); i++) if (strstr(items[i]->name.c_str(), _nameprefix) == items[i]->name.c_str()) { return items[i]; } return NULL; }
    T							*get(const char *_name) {
    	T *rt = get_silent(_name);
        if (rt == NULL) {
            printf("  '%s' missing\nsearched: ", _name);
            for (int i=0; i<items.size(); i++) printf("%s, ", items[i]->name.c_str());
            printf("\n");
        }
        return rt;
    }
    T							*get(int _ident) { for (int i=0; i<items.size(); i++) if (items[i]->ident == _ident) { return items[i]; } return NULL; }
public:
	unsigned int				size() { return (int)items.size(); }
    T							*operator [] (unsigned int idx) { return items[idx]; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TActiveTouch tracks all active touches by disambiguator
class TActiveTouch {
public:
    							TActiveTouch() { disambiguator = NULL; owner = NULL; }
	virtual						~TActiveTouch() { }
public:
	void						*disambiguator;
    LElement					*owner;
    float						cx, cy;					// current touch coords
    float						sx, sy;					// initial touch coords
public:
	static std::vector<TActiveTouch*> active_touches;
};

#endif // !_LMLIST_H_
