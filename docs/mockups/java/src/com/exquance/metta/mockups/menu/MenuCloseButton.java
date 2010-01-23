package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.Sketch;

public class MenuCloseButton extends RadialComponent {
    
    float btn_radius = 18;
    int button_color;    

    public MenuCloseButton(Sketch p) {
        super(p);
        button_color = p.color(0x660000);
    }

    @Override
    public void update() {
        p.fill(button_color);   
        p.ellipse(center_x, center_y, btn_radius * 2, btn_radius * 2);
    }
    
    @Override
    public boolean is_mouse_in(float mouse_x, float mouse_y) {
        return in_circle(mouse_x, mouse_y,
                         center_x, center_y,
                         btn_radius);
    } 
    
    @Override
    public boolean on_mouse_over(float mouse_x, float mouse_y) {
        button_color = p.color(0xffffff);
        return true;
    }    
    
    @Override
    public boolean on_mouse_out() {
        button_color = p.color(0x660000);
        return true;
    }    

}
