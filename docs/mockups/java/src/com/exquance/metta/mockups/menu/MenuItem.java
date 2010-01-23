package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.Sketch;

public class MenuItem  extends RadialComponent {

    String item_name;

    int item_color, over_color = COLOR_UNSET;
    float start_radius, end_radius = RADIUS_UNSET;
    float start_angle, end_angle = ANGLE_UNSET;
    
    int cur_color = COLOR_UNSET;
    
    boolean is_mouse_over = false;
    
    public MenuItem(Sketch p, String item_name) {
        super(p);
        this.item_name = item_name;
    }
    
    public void set_radius(float start_radius, float end_radius) {
        this.start_radius = start_radius;
        this.end_radius = end_radius;
    }
    
    public boolean is_radius_set() {
        return (start_radius != RADIUS_UNSET) &&
               (end_radius != RADIUS_UNSET);
    }
    
    public void set_angle(float start_angle, float end_angle) {
        this.start_angle = start_angle;
        this.end_angle = end_angle;
    }
    
    public boolean is_angle_set() {
        return (start_angle != ANGLE_UNSET) &&
               (end_angle != ANGLE_UNSET);        
    }
    
    public void set_color(int item_color) {
        this.item_color = item_color;
        this.over_color = p.lerpColor(item_color, p.color(0x33333300), .5f);
        this.cur_color = this.item_color;
    }
    
    public boolean is_color_set() {
        return (item_color != COLOR_UNSET);
    }    

    public void update() {        
    
        p.fill(cur_color);
        p.arc(center_x, center_y, 
            end_radius * 2, end_radius * 2, 
            start_angle, end_angle); // outer arc
        p.noFill();
        p.arc(center_x, center_y, 
            start_radius * 2, start_radius * 2, 
            start_angle, end_angle); // inner arc
            
        /* // [un]comment to turn on[off] lines between sectors
        line(center_x + start_radius * cos(start_angle),
             center_y + start_radius * sin(start_angle),
             center_x + end_radius * cos(start_angle),
             center_y + end_radius * sin(start_angle));
        line(center_x + start_radius * cos(end_angle),
             center_y + start_radius * sin(end_angle),
             center_x + end_radius * cos(end_angle),
             center_y + end_radius * sin(end_angle)); */            
        
        p.pushMatrix();
        p.fill(p.lerpColor(cur_color, p.color(0xffffff00), is_mouse_over ? .6f : .07f));   
        p.translate(center_x, center_y);
        /* // to show in the center of sector 
        rotate(start_angle + abs(end_angle - start_angle) / 2);
        float padding_x = ((end_radius - start_radius) - (item_name.length * 6)) / 2;
        text(item_name, start_radius + padding_x, -8); */
        // to show at the side of the sector
        p.rotate(start_angle);
        //p.textFont(p.get_cur_font());
        //p.text(item_name, start_radius, -20);        
        p.popMatrix();
    }
    
    @Override
    public boolean is_mouse_in(float mouse_x, float mouse_y) { 
        return in_ring_sector(mouse_x, mouse_y,
                              this.center_x, this.center_y,
                              start_radius, end_radius,
                              start_angle, end_angle);                       
    }
    
    @Override
    public boolean on_mouse_over(float mouse_x, float mouse_y) {
        is_mouse_over = true;
        cur_color = over_color;
        return true;
    }
    
    @Override
    public boolean on_mouse_out() {
        is_mouse_over = false;    
        cur_color = item_color;
        return true;        
    } 

};
