package com.exquance.metta.mockups._components_lib;

import com.exquance.metta.mockups.Component;
import com.exquance.metta.mockups.M;

public abstract class RadialComponent extends Component {

    protected float center_x = 0f;
    protected float center_y = 0f;

    public void set_pos(float center_x, float center_y) {
        this.center_x = center_x;
        this.center_y = center_y;
    }
    
    public static boolean in_circle(float point_x, float point_y,
                      float circle_x, float circle_y,
                      float circle_radius) {
        // if point lies on the square describing this circle
        float dx = M.abs(point_x - circle_x); 
        float dy = M.abs(point_y - circle_y);
        if ((dx <= circle_radius) && (dy <= circle_radius)) {
             // get angle from the circle center to the point
             float at_angle = M.atan2(dy, dx);
             // get distance to the instersection point 
             // of this angle and ring with given radius
             float x_way = circle_radius * M.cos(at_angle);
             float y_way = circle_radius * M.sin(at_angle);
             // check if given point is in bounds of this distance
             return (dx <= x_way) && (dy <= y_way); 
        } else {
            return false;
        }
    }

    public static boolean on_a_ring(float point_x, float point_y,
                    float ring_x, float ring_y,
                    float min_radius, float max_radius) {
        float dx = M.abs(point_x - ring_x);
        float dy = M.abs(point_y - ring_y);
        // if point lies on the square describing this ring        
        if ((dx <= max_radius) && (dy <= max_radius)) {            
             // get angle from the ring center to the point
             float at_angle = M.atan2(dy, dx);
             // get distance to the instersection point 
             // of this angle and ring with max radius
             float x_max_way = max_radius * M.cos(at_angle);
             float y_max_way = max_radius * M.sin(at_angle);
             // get distance to the instersection point 
             // of this angle and ring with min radius
             float x_min_way = min_radius * M.cos(at_angle);
             float y_min_way = min_radius * M.sin(at_angle);             
             // check if given point is in bounds of these distances             
             return (dx >= x_min_way) && (dx <= x_max_way) &&
                    (dy >= y_min_way) && (dy <= y_max_way); 
        } else {
            return false;
        }                    
    }
    
    // FIXME: static    
    public boolean in_ring_sector(float point_x, float point_y,
                                  float ring_x, float ring_y,
                                  float min_radius, float max_radius,
                                  float sector_start, float sector_end) {
        float chk_start = ((sector_start < 0) ? (sector_start + TWO_PI) : sector_start);
        float chk_end = ((sector_end < 0) ? (sector_end + TWO_PI) : sector_end);
        float dx = point_x - ring_x;
        float dy = point_y - ring_y;        
        float ang_tan = M.atan2(dy, dx);
        if (ang_tan < 0) ang_tan += TWO_PI;
        boolean angle_fits = (chk_end >= chk_start) ? 
                             ((ang_tan >= chk_start) && (ang_tan <= chk_end)) :
                             ((ang_tan <= chk_end) || (ang_tan >= chk_start));
        return angle_fits &&
               on_a_ring(point_x, point_y,
                         ring_x, ring_y,
                         min_radius, max_radius);
    }

};
