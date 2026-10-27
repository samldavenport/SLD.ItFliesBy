#pragma once

#include "physics-internal.hpp"
#include <cfloat>

namespace ifb {

    IFB_INTERNAL void
    phys_aabb_reset(
        phys_aabb& aabb) {

        // inverted so the first grow snaps to the new bounds
        aabb.min.x =  FLT_MAX;
        aabb.min.y =  FLT_MAX;
        aabb.min.z =  FLT_MAX;
        aabb.max.x = -FLT_MAX;
        aabb.max.y = -FLT_MAX;
        aabb.max.z = -FLT_MAX;
    } 

    IFB_INTERNAL void
    phys_aabb_grow(
        phys_aabb&       aabb_curr,
        const phys_aabb& aabb_new) {

        aabb_curr.min.x = fminf(aabb_curr.min.x, aabb_new.min.x);
        aabb_curr.min.y = fminf(aabb_curr.min.y, aabb_new.min.y);
        aabb_curr.min.z = fminf(aabb_curr.min.z, aabb_new.min.z);
        
        aabb_curr.max.x = fmaxf(aabb_curr.max.x, aabb_new.max.x);
        aabb_curr.max.y = fmaxf(aabb_curr.max.y, aabb_new.max.y);
        aabb_curr.max.z = fmaxf(aabb_curr.max.z, aabb_new.max.z);
    } 

    IFB_INTERNAL void
    phys_aabb_grow_pt(
        phys_aabb&  aabb_curr,
        const vec3& pt) {

        aabb_curr.min.x = fminf(aabb_curr.min.x, pt.x);
        aabb_curr.min.y = fminf(aabb_curr.min.y, pt.y);
        aabb_curr.min.z = fminf(aabb_curr.min.z, pt.z);
        
        aabb_curr.max.x = fmaxf(aabb_curr.max.x, pt.x);
        aabb_curr.max.y = fmaxf(aabb_curr.max.y, pt.y);
        aabb_curr.max.z = fmaxf(aabb_curr.max.z, pt.z);
    } 
    
    IFB_INTERNAL bool
    phys_aabb_check(
        const phys_aabb& aabb_a,
        const phys_aabb& aabb_b) {

        bool does_overlap = true;
        does_overlap &= (aabb_a.min.x < aabb_b.max.x);
        does_overlap &= (aabb_a.max.x > aabb_b.min.x);
        does_overlap &= (aabb_a.min.y < aabb_b.max.y);
        does_overlap &= (aabb_a.max.y > aabb_b.min.y);
        does_overlap &= (aabb_a.min.z < aabb_b.max.z);
        does_overlap &= (aabb_a.max.z > aabb_b.min.z);
        return(does_overlap);
    }
    
    IFB_INTERNAL f32
    phys_aabb_area(
        const phys_aabb& aabb) {

        const f32 dx = aabb.max.x - aabb.min.x;
        const f32 dy = aabb.max.y - aabb.min.y;
        const f32 dz = aabb.max.z - aabb.min.z;
    
        const f32 area = (
            dx * dy +
            dy * dz +
            dz * dx
        );

        return(area);
    }

    IFB_INTERNAL vec3 
    phys_aabb_centroid(
        const phys_aabb& aabb) {

        vec3 centroid;
        centroid.x = (aabb.min.x + aabb.max.x) * 0.5f;
        centroid.y = (aabb.min.y + aabb.max.y) * 0.5f;
        centroid.z = (aabb.min.z + aabb.max.z) * 0.5f;
        return(centroid);
    }
};
