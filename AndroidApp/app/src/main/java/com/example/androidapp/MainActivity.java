package com.example.androidapp;


import android.app.ProgressDialog;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;


import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;


public class MainActivity extends AppCompatActivity {
    private static final String TAG = "SmartCarMqttController";
    private static final String LOCALHOST = "192.168.0.149";
    private static final String MQTT_SERVER = "tcp://" + LOCALHOST + ":1883";

    static String USERNAME = "admin";
    static String PASSWORD = "";  // **

    private String controlTopic = "/Group/16/Control";
    private String cameraTopic = "/Group/16/Camera";

    private String leftDistanceTopic = "/Group/16/LeftDistance";
    private String middleDistanceTopic = "/Group/16/MiddleDistance";
    private String rightDistanceTopic = "/Group/16/RightDistance";

    private static final int IMAGE_WIDTH = 320; // **
    private static final int IMAGE_HEIGHT = 240; // **

    private String backMessage;
    private String frontMessage;
    private String leftMessage;
    private String rightMessage;

    private int progress = 0;

    MqttAndroidClient client;
    private MqttClient mMqttClient;

    final Bitmap bm = Bitmap.createBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        client = new MqttAndroidClient(MainActivity.this, MQTT_SERVER, TAG);

        MqttConnectOptions options = new MqttConnectOptions();
        options.setUserName(USERNAME); //set the username
        options.setPassword(PASSWORD.toCharArray()); //set the username

        try {
            client.connect(options, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.e(TAG, "success");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Fail");
                }
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }

        Button Ahead = findViewById(R.id.buttonAhead);
        Button Back = findViewById(R.id.buttonBack);
        Button Left = findViewById(R.id.buttonLeft);
        Button Right = findViewById(R.id.buttonRight);
        Button Stop = findViewById(R.id.buttonStop);
        Switch Cruise = (Switch) findViewById(R.id.switchCruise);
        Cruise.setChecked(false);
        ImageView Stream = findViewById(R.id.imageView);

        ProgressBar LeftBar = findViewById(R.id.progressBarLeft);
        ProgressBar MiddleBar = findViewById(R.id.progressBarMiddle);
        ProgressBar RightBar = findViewById(R.id.progressBarRight);


        Cruise.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton Cruise, boolean isChecked) {
                String cruiseMessage = "Cruise";
                //Prevent the listener from triggering during initialization
                if (Cruise.isPressed() && isChecked)     // **
                {
                    try {
                        client.publish(controlTopic, cruiseMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                } else {
                    //return to use manual control;
                    String message = "Stop";
                    try {
                        client.publish(controlTopic, message.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                }
            }
        });

        Stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //When "Stop" is clicked the car stop
                String message = "Stop";
                try {
                    client.publish(controlTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
            }
        });

        Ahead.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    message = "Ahead";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    message = "Stop";
                }
                try {
                    client.publish(controlTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
                return false;
            }
        });

        Back.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    message = "Back";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    message = "Stop";
                }
                try {
                    client.publish(controlTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
                return false;
            }
        });

        Left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    message = "Left";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    message = "Stop";
                }
                try {
                    client.publish(controlTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
                return false;
            }
        });

        Right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    message = "Right";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    message = "Stop";
                }
                try {
                    client.publish(controlTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
                return false;
            }
        });

        Stream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    cameraView(cameraTopic, Stream);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
            }
        });

    }

    public void progressBar(String topic, ProgressBar progressBar) throws MqttException {

        IMqttToken subToken = client.subscribe(topic, 1);
        subToken.setActionCallback(new IMqttActionListener() {
            @Override
            public void onSuccess(IMqttToken asyncActionToken) {

            }

            @Override
            public void onFailure(IMqttToken asyncActionToken, Throwable exception) {

            }
        });
        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {

                if (topic.equals(leftDistanceTopic)) {
                    progress = (Integer.parseInt(message.toString()) / 320) * 100;

                    progressBar.setProgress(progress);
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });
    }


    public void cameraView(String topic, ImageView camera) throws MqttException {

        IMqttToken subToken = client.subscribe(topic, 0);
        subToken.setActionCallback(new IMqttActionListener() {
            @Override
            public void onSuccess(IMqttToken asyncActionToken) {
                Toast.makeText(MainActivity.this, "Camera Connected", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                Toast.makeText(MainActivity.this, "Camera Not connected", Toast.LENGTH_SHORT).show();
            }
        });

        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if (topic.equals(cameraTopic)) {
//                    final Bitmap bm = Bitmap.createBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);
                    final byte[] payload = message.getPayload();
                    final int[] colors = new int[IMAGE_WIDTH * IMAGE_HEIGHT];
                    for (int ci = 0; ci < colors.length; ++ci) {
                        final byte r = payload[3 * ci];
                        final byte g = payload[3 * ci + 1];
                        final byte b = payload[3 * ci + 2];
                        colors[ci] = Color.rgb(r, g, b);
                    }
                    bm.setPixels(colors, 0, IMAGE_WIDTH, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
                    camera.setImageBitmap(bm);
                } else {
                    Log.e(topic, "[MQTT] Topic: " + topic + " | Message: " + message.toString());
                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                Log.d(TAG, "Message delivered");
            }
        });
    }

}

