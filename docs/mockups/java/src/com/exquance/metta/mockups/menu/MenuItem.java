package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;

public class MenuItem  extends RadialComponent {

    private String item_name;

    private int item_color = COLOR_UNSET;
    private int over_color = COLOR_UNSET;
    
    private float start_radius = RADIUS_UNSET; 
    private float end_radius = RADIUS_UNSET;
    
    private float start_angle = ANGLE_UNSET;
    private float end_angle = ANGLE_UNSET;
    
    int cur_color = COLOR_UNSET;
    
    boolean is_mouse_over = false;
    
    public MenuItem(String item_name) {
        super();
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
        this.over_color = M.lerpColor(item_color, M.wcolor(0x333333), .5f);
        this.cur_color = this.item_color;
    }
    
    public boolean is_color_set() {
        return (item_color != COLOR_UNSET);
    }    

    public void update(Sketch p) {        
    
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
        p.fill(M.lerpColor(cur_color, M.wcolor(0xffffff), is_mouse_over ? .6f : .07f));   
        p.translate(center_x, center_y);
        /* // to show in the center of sector 
        rotate(start_angle + abs(end_angle - start_angle) / 2);
        float padding_x = ((end_radius - start_radius) - (item_name.length * 6)) / 2;
        text(item_name, start_radius + padding_x, -8); */
        // to show at the side of the sector
        p.rotate(end_angle);
        p.textFont(p.get_cur_font());
        p.text(item_name, start_radius, 0f);        
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

    @Override
    public void prepare(Sketch p) { }

};
