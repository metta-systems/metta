package com.exquance.metta.mockups._sketch_controls;

import java.util.List;

public interface MouseReactor {
    
    public void add_mouse_listener(MouseListener listener);
    public <T extends MouseListener> void add_mouse_listeners(List<T> listeners);
    
    public void mouse_move(float mouse_x, float mouse_y);
    public void mouse_click(float mouse_x, float mouse_y);

}
