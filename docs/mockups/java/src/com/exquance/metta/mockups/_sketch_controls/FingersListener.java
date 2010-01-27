package com.exquance.metta.mockups._sketch_controls;

public interface FingersListener {
    
    public boolean is_finger_in(float finger_x, float finger_y);
    public boolean accepts_fingers(byte left_hand, byte right_hand);    
    public boolean can_be_gragged(byte left_hand, byte right_hand);    
    
    public boolean on_fingers_move(byte left_hand, byte right_hand);
    public boolean on_fingers_tap(byte left_hand, byte right_hand);
    public boolean on_fingers_drag(byte left_hand, byte right_hand);
    public boolean on_firgers_drop();

}
