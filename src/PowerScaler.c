//
//  PowerScaler.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/06/2012.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
//

#include "VexOS.h"
#include "Error.h"

/********************************************************************
 * PowerScaler Structures                                           *
 ********************************************************************/

struct PowerScaler {
    const char* name;
    List        points;
};

typedef struct {
    float   input;
    float   output;
} ScalePoint;

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

PowerScaler* PowerScaler_new(const char* name) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    
    PowerScaler* scale = malloc(sizeof(PowerScaler));
    scale->name             = name;
    scale->points.nodeCount = 0;
    scale->points.firstNode = NULL;
    scale->points.lastNode  = NULL;
    PowerScaler_addPoint(scale, 0.0, 0.0);
    PowerScaler_addPoint(scale, 1.0, 1.0);
    return scale;
}

void PowerScaler_addPoint(PowerScaler* scale, Power input, Power output) {
    ErrorIf(scale == NULL, VEXOS_ARGNULL);
    ErrorIf(input < -1.0 || input > 1.0, VEXOS_ARGRANGE);
    
    // create the scale point //
    ScalePoint* point = malloc(sizeof(ScalePoint));
    point->input   = input;
    point->output  = output;
    ListNode* pnode = List_newNode(point);
    // is list empty //
    if(scale->points.nodeCount == 0) {
        List_insertLast(&scale->points, pnode);
    } else {
        // insert in sorted order //
        ListNode* lnode = scale->points.firstNode;
        while(lnode != NULL) {
            ScalePoint* lpoint = (ScalePoint*) lnode->data;
            // if less, add before //
            if(point->input < lpoint->input) {
                List_insertBefore(lnode, pnode);
                break;
            }
            // if same, replace //
            if(point->input == lpoint->input) {
                List_insertAfter(lnode, pnode);
                List_remove(lnode);
                break;
            }
            lnode = lnode->next;
        }
        // if still not added, add last //
        if(pnode->list == NULL) {
            List_insertLast(&scale->points, pnode);
        }
    }
}

Power PowerScaler_get(PowerScaler* scale, Power input) {
    ErrorIf(scale == NULL, VEXOS_ARGNULL);
    
    float ipower = (input < 0)? -input: input;
    float output = 0.0;
    
    // make sure we have nodes //
    ListNode* pnode = scale->points.firstNode;
    if(pnode == NULL) return output;
    
    // check for underflow //
    if(ipower < ((ScalePoint*) pnode->data)->input) {
        output = ((ScalePoint*) pnode->data)->output;
    } else {
        while(pnode != NULL) {
            ScalePoint* point1 = (ScalePoint*) pnode->data;
            // check for overflow //
            if(pnode->next == NULL) {
                output = ((ScalePoint*) pnode->data)->output;
                break;
            }
            ScalePoint* point2 = (ScalePoint*) pnode->next->data;
            if(ipower >= point1->input && ipower < point2->input) {
                output = ((ipower - point1->input) * (point2->output - point1->output)
                          / (point2->input - point1->input)) + point1->output;
                break;
            }
            // got to next point //
            pnode = pnode->next;
        }
    }
    return (Power) (input < 0)? -output: output;
}

String PowerScaler_toString(PowerScaler* scale) {
    ErrorIf(scale == NULL, VEXOS_ARGNULL);
    
    char* ret = NULL;
    ListNode* pnode = scale->points.firstNode;
    while(pnode != NULL) {
        ScalePoint* point = (ScalePoint*) pnode->data;
        char* old = ret;
        asprintf(&ret, "%s  { %f, %f }%s\n", (old? old: ""), point->input, point->output, (pnode->next)? ",": "");
        free(old);
        pnode = pnode->next;
    }
    char* old = ret;
    asprintf(&ret, "PowerScaler(%s) {\n%s}\n", scale->name, ret);
    free(old);
    return ret;
}
