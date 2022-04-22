package com.example.androidapp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;


public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button Ahead = findViewById(R.id.buttonAhead);
        Button Back = findViewById(R.id.buttonBack);
        Button Left = findViewById(R.id.buttonLeft);
        Button Right = findViewById(R.id.buttonRight);
        Button Stop = findViewById(R.id.buttonStop);
        CompoundButton Cruise = findViewById(R.id.switchCruise);

        Cruise.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){
            @Override
            public void onCheckedChanged(CompoundButton Cruise, boolean isChecked) {
                //Prevent the listener from triggering during initialization
                if (!Cruise.isPressed()) {
                    return;
                }
                //doSomeThing();
            }

        });


        Stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //When "Stop" is clicked the car stop
            }
        });


        Ahead.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed

                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.

                }
                return false;
            }
        });

        Back.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed

                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.

                }
                return false;
            }
        });

        Left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed

                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.

                }
                return false;
            }
        });

        Right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed

                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.

                }
                return false;
            }
        });
    }

}