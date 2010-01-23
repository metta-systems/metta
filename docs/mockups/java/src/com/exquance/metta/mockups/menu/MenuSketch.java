package com.exquance.metta.mockups.menu;

import com.exquance.metta.mockups.Sketch;

import processing.core.*;

public class MenuSketch extends Sketch {
	
	boolean floatMenuShown = false;
	
	Menu left_menu;
	Menu center_menu;
	Menu right_menu;
	Menu float_menu;
	
	public MenuSketch() {
	    super();
	    
	    // Left Menu
	    
	    left_menu = new Menu(this);
	    
	    left_menu.add_item(new MenuItem(this,"cut"));
	    left_menu.add_item(new MenuItem(this,"copy"));
	    left_menu.add_item(new MenuItem(this,"paste"));
	    left_menu.add_item(new MenuItem(this,"delete"));
	    left_menu.add_item(new MenuItem(this,"back"));
	    left_menu.add_item(new MenuItem(this,"forward"));
	    left_menu.add_item(new MenuItem(this,"reload"));

	    left_menu.set_bounds(-HALF_PI, HALF_PI * 3 / 2);
	    left_menu.set_colors(color(0x330000), color(0x660000));
	    left_menu.set_radius(120);

	    left_menu.set_pos(100, 200);
	    
	    // Center Menu
	    
	    center_menu = new Menu(this);
	    
	    center_menu.add_item(new MenuItem(this,"a"));
	    center_menu.add_item(new MenuItem(this,"b"));
	    center_menu.add_item(new MenuItem(this,"c"));
	    center_menu.add_item(new MenuItem(this,"d"));

	    center_menu.set_bounds(HALF_PI * 6 / 5, HALF_PI * 7 / 2);
	    center_menu.set_colors(color(0x003333), color(0x006666));
	    center_menu.set_radius(90);

	    center_menu.set_pos(300, 200);	 
	    
        // Right Menu
	    
	    right_menu = new Menu(this);
	    
	    right_menu.add_item(new MenuItem(this,"say"));
	    right_menu.add_item(new MenuItem(this,"a"));
	    right_menu.add_item(new MenuItem(this,"word"));
	    right_menu.add_item(new MenuItem(this,"here"));

	    right_menu.set_bounds(-(HALF_PI / 2), HALF_PI * 7 / 2);
	    right_menu.set_colors(color(0x003300), color(0x006600));
	    right_menu.set_radius(130);

	    right_menu.set_pos(500, 200);	    
	    
	    
	    // Floating Menu
	    
	    float_menu = new Menu(this);

	    float_menu.add_item(new MenuItem(this,"cut"));
	    float_menu.add_item(new MenuItem(this,"copy"));
	    float_menu.add_item(new MenuItem(this,"paste"));
	    float_menu.add_item(new MenuItem(this,"send to..."));
	    float_menu.add_item(new MenuItem(this,"what the?"));
	    float_menu.add_item(new MenuItem(this,"let's party"));
	    float_menu.add_item(new MenuItem(this,"happy new year"));

	    float_menu.set_bounds(HALF_PI * 6 / 5, HALF_PI * 7 / 2);
	    float_menu.set_colors(color(0x000033), color(0x000066));
	    float_menu.set_radius(120);
	    
        //textBlock = loadImage("textBlock.png");
        //djvuSansFont = createFont("DejaVuSans.ttf", Menu.TEXT_SIZE)
	    
	} 
	
	@Override
	public void setup() {
	    stretch();
	    smooth();
	    frameRate(30);
	    
        // set_cur_font(loadFont("DejaVuSans-40.vlw"));
	  
	    left_menu.prepare();
	    center_menu.prepare();
	    right_menu.prepare();
	    float_menu.prepare();		
	}
	
	public void draw() {
	    background(0x666666);
	    
	    // background(#ffffff);    
	    // image(textBlock, 20, 20); 
	    
	    left_menu.mouse_move(mouseX, mouseY);
	    center_menu.mouse_move(mouseX, mouseY);
	    right_menu.mouse_move(mouseX, mouseY);
	            
	    left_menu.update();
	    center_menu.update();    
	    right_menu.update();
	  
	    if (floatMenuShown) {
	        float_menu.mouse_move(mouseX, mouseY);
	        float_menu.update();        
	    }
	}
	
	@Override
	public void mousePressed() {

		left_menu.mouse_click(mouseX, mouseY);
		center_menu.mouse_click(mouseX, mouseY);
		right_menu.mouse_click(mouseX, mouseY);   
		   
		float_menu.set_pos(mouseX, mouseY); 
		floatMenuShown = true;   
		   
		if (floatMenuShown) {
		    float_menu.mouse_click(mouseX, mouseY);
		}  
	}	
}
