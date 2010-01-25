package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;

import processing.core.*;

public class MenuSketch extends Sketch {
	
	boolean floatMenuShown = false;
	
	private Menu left_menu;
	private Menu center_menu;
	private Menu right_menu;
	private Menu float_menu;
	
	public MenuSketch() {
	    super();
	    
	    // Left Menu
	    
	    left_menu = new Menu();
	    
	    left_menu.add_item(new MenuItem("cut"));
	    left_menu.add_item(new MenuItem("copy"));
	    left_menu.add_item(new MenuItem("paste"));
	    left_menu.add_item(new MenuItem("delete"));
	    left_menu.add_item(new MenuItem("back"));
	    left_menu.add_item(new MenuItem("forward"));
	    left_menu.add_item(new MenuItem("reload"));

	    left_menu.set_bounds(-HALF_PI, HALF_PI * 3 / 2);
	    left_menu.set_colors(M.wcolor(0x330000), M.wcolor(0x660000));
	    left_menu.set_radius(120);

	    left_menu.set_pos(100, 200);
	    
        add_component(left_menu);	   
	    
	    // Center Menu
	    
	    center_menu = new Menu();
	    
	    center_menu.add_item(new MenuItem("a"));
	    center_menu.add_item(new MenuItem("b"));
	    center_menu.add_item(new MenuItem("c"));
	    center_menu.add_item(new MenuItem("d"));

	    center_menu.set_bounds(HALF_PI * 6 / 5, HALF_PI * 7 / 2);
	    center_menu.set_colors(M.wcolor(0x003333), M.wcolor(0x006666));
	    center_menu.set_radius(90);

	    center_menu.set_pos(300, 200);	
	    
        add_component(center_menu);	   
	    
        // Right Menu
	    
	    right_menu = new Menu();
	    
	    right_menu.add_item(new MenuItem("say"));
	    right_menu.add_item(new MenuItem("a"));
	    right_menu.add_item(new MenuItem("word"));
	    right_menu.add_item(new MenuItem("here"));

	    right_menu.set_bounds(-(HALF_PI / 2), HALF_PI * 7 / 2);
	    right_menu.set_colors(M.wcolor(0x003300), M.wcolor(0x006600));
	    right_menu.set_radius(130);

	    right_menu.set_pos(500, 200);	    

        add_component(right_menu);	    
	    
	    // Floating Menu
	    
	    float_menu = new Menu();

	    float_menu.add_item(new MenuItem("cut"));
	    float_menu.add_item(new MenuItem("copy"));
	    float_menu.add_item(new MenuItem("paste"));
	    float_menu.add_item(new MenuItem("send to..."));
	    float_menu.add_item(new MenuItem("what the?"));
	    float_menu.add_item(new MenuItem("let's party"));
	    float_menu.add_item(new MenuItem("happy new year"));

	    float_menu.set_bounds(HALF_PI * 6 / 5, HALF_PI * 7 / 2);
	    float_menu.set_colors(M.wcolor(0x000033), M.wcolor(0x000066));
	    float_menu.set_radius(120);
	   
        add_component(float_menu); 
        float_menu.hide();
	    
        //textBlock = loadImage("textBlock.png");
        //djvuSansFont = createFont("DejaVuSans.ttf", Menu.TEXT_SIZE)
	    
	} 
	
	@Override
	public void setup() {
	    stretch();
	    smooth();
	    frameRate(30);
	    
        set_cur_font(loadFont("DejaVuSans-40.vlw"));
	  
	    super.setup();		
	}
	
	public void draw() {
	    background(0x666666);
	    
	    // background(#ffffff);    
	    // image(textBlock, 20, 20); 
	    
	    super.draw();
	}
	
	
    @Override
    public void mousePressed() {
        
        float_menu.set_pos(mouseX, mouseY); 
        float_menu.show();
        
        super.mousePressed();
    }
}
