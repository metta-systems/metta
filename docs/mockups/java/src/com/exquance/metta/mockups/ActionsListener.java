package com.exquance.metta.mockups;

public interface ActionsListener {
    
    public boolean is_mouse_in(float mouse_x, float mouse_y);    
    public boolean on_mouse_over(float mouse_x, float mouse_y);
    public boolean on_mouse_click(float mouse_x, float mouse_y);
    public boolean on_mouse_out();    

}
