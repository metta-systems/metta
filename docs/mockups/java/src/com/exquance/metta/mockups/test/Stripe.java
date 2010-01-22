package com.exquance.metta.mockups.test;
import processing.core.PApplet;

class Stripe {
	
  PApplet p; // The parent PApplet that we will render ourselves onto	
	
  float x;       // horizontal location of stripe
  float speed;   // speed of stripe
  float w;       // width of stripe
  boolean mouse; // state of stripe (mouse is over or not?)

  Stripe(PApplet p) {
	this.p = p;
	
    x = 0;              // All stripes start at 0
    speed = p.random(1);  // All stripes have a random positive speed
    w = p.random(10,30);
    mouse = false;
  }

  // Draw stripe
  void display() {
	p.fill(255,100);
	p.noStroke();
    p.rect(x,0,w,p.height);
  }

  // Move stripe
  void move() {
    x += speed;
    if (x > p.width+20) x = -20;
  }
}

