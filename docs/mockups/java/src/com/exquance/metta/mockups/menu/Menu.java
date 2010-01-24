package com.exquance.metta.mockups.menu;

import java.util.ArrayList;
import java.util.List;

import com.exquance.metta.mockups.Sketch;

public class Menu extends RadialComponent {

    private int stroke_color;
    private int start_color;
    private int end_color;
    
    private float menu_radius = 120;
    private float start_angle = -HALF_PI;
    private float end_angle = HALF_PI * 3 / 2;
    
    public static final int TEXT_SIZE = 40; 

    List<MenuItem> items;
    MenuCloseButton close_btn;
    
    public Menu(Sketch p, ArrayList<MenuItem> items) {
        super(p);
        close_btn = new MenuCloseButton(p);
        add_listener(close_btn); 
               
        if (items != null) {
            this.items = items;
            add_listeners(this.items);
        } else {
            this.items = new ArrayList<MenuItem>();
        }
        
        stroke_color = p.mcolor(0xeeeeee);
        start_color = p.mcolor(0x000033);
        end_color = p.mcolor(0x000066);        
                
    }
    
    /* Menu(String[] items_names) {  // FIXME: setting through array moves in recursion
        close_btn = new MenuCloseButton();
        add_listener(close_btn);
        
        for (int i = 0; i < items_names.length; i++) {
            add_item(new MenuItem(items_names[i]));
        }
    }  */
    
    public Menu(Sketch p) {
        this(p, null);
    }
    
    public void add_item(MenuItem item) {
        items.add(item);
        add_listener(item);
    }
        
    public void set_bounds(float start_angle, float end_angle) {
        this.start_angle = start_angle;
        this.end_angle = end_angle;
    }
    
    public void set_radius(float new_radius) {
        this.menu_radius = new_radius;
    }
    
    public void set_colors(int start_color, int end_color) {
        this.start_color = start_color;
        this.end_color = end_color;
    }
    
    public void prepare() {
        // called on processing setup()
        float angle_step = (end_angle - start_angle) / items.size();
        float start_radius = close_btn.btn_radius /*+ 1*/;
        float lerp_amount = 1f / items.size();
        
        int cur_color = start_color;
        float cur_start_angle = start_angle;
        float cur_end_angle = cur_start_angle + angle_step;
        
        //p.textSize(TEXT_SIZE);        
        
        for (int i = 0; i < items.size(); i++) {
            
            MenuItem item = items.get(i);
            if (!item.is_color_set())  item.set_color(cur_color);            
            if (!item.is_radius_set()) item.set_radius(start_radius, 
                                                       menu_radius);
            if (!item.is_angle_set())  item.set_angle(cur_start_angle, 
                                                      cur_end_angle);
            
            cur_start_angle += angle_step;
            cur_end_angle = cur_start_angle + angle_step;
            
            cur_color = p.lerpColor(start_color, end_color, (float)(lerp_amount * (i + 1)));
        }        
    }

    @Override
    public void update() {
        p.stroke(stroke_color);
        for (int i = 0; i < items.size(); i++) {
            items.get(i).update();
        }
        close_btn.update();
    }
    
    @Override
    public void set_pos(float center_x, float center_y) {
        close_btn.set_pos(center_x, center_y);
        for (int i = 0; i < items.size(); i++) {
            items.get(i).set_pos(center_x, center_y);
        }
    }

}
