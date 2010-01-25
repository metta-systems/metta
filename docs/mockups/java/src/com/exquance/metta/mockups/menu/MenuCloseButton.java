package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;

public class MenuCloseButton extends RadialComponent {
    
    protected float btn_radius = 18;
    private int button_color;    

    public MenuCloseButton() {
        super();        
    }

    @Override
    public void update(Sketch p) {
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
        button_color = M.wcolor(0xffffff);
        return true;
    }    
    
    @Override
    public boolean on_mouse_out() {
        //button_color = p.mcolor(0xffff6600);
        button_color = M.wcolor(0x660000);
        return true;
    }

    @Override
    public void prepare(Sketch p) { 
        button_color = M.wcolor(0x660000);
    }    

}
