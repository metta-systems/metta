package com.exquance.metta.mockups._sketch_controls;

import java.util.List;

public interface FingersReactor {
    
    public void add_fingers_listener(FingersListener listener);
    public <T extends FingersListener> void add_fingers_listeners(List<T> listeners);
    
    public void fingers_tap(byte left_hand, byte right_hand, float x, float y);
    public void fingers_move(byte left_hand, byte right_hand, float x, float y);

}
