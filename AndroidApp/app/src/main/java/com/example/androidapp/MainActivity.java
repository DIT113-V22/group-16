package com.example.androidapp;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.Toast;

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



public class MainActivity extends AppCompatActivity {
    private MqttAndroidClient mMqttClient;
    private static final String MQTT_SERVER = "tcp://192.168.0.242:1883";
    private static final String TAG = "SmartCarMqttController";
    static String USERNAME = "admin";
    static String PASSWORD = "hivemq";
    private String topic = "/Group/16";
    private String backTopic = "/Group/16/Back";
    private String frontTopic = "/Group/16/Front";
    private String leftTopic = "/Group/16/Left";
    private String rightTopic = "/Group/16/Right";
    private String stopTopic = "/Group/16/Stop";
    private String cruiseTopic = "/Group/16/Cruise";

    private String backMessage;
    private String frontMessage;
    private String leftMessage;
    private String rightMessage;

    MqttAndroidClient client;

    int IMAGE_WIDTH = 411;
    int IMAGE_HEIGHT = 250;
    final Bitmap bm = Bitmap.createBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        client = new MqttAndroidClient(MainActivity.this, MQTT_SERVER, TAG);

        MqttConnectOptions options = new MqttConnectOptions();
        options.setUserName(USERNAME);//set the username
        options.setPassword(PASSWORD.toCharArray());//set the username

        try {
            client.connect(options,new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.e(TAG,"success");
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG,"Fail");
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
        Switch Cruise = (Switch)findViewById(R.id.switchCruise);
        ImageView Stream = findViewById(R.id.imageView);

        Stream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

        Cruise.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){
            @Override
            public void onCheckedChanged(CompoundButton Cruise, boolean isChecked) {
                String cruiseMessage = "Cruise";
                //Prevent the listener from triggering during initialization
                if (Cruise.isPressed()) {
                    try {
                        client.publish(cruiseTopic, cruiseMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                    return;
                } else {
                    //return to use manual control;
                    String message = "Stop";
                    try {
                        client.publish(stopTopic, message.getBytes(), 0, false);
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
                    client.publish(stopTopic, message.getBytes(), 0, false);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
            }
        });


        Ahead.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String frontMessage = "Ahead";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button released
                    try {
                        client.publish(frontTopic, frontMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }

                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    String message = "Stop";
                    try {
                        client.publish(stopTopic, message.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }

                }
                return false;
            }
        });

        Back.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String backMessage = "Back";
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    try {
                        client.publish(backTopic, backMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    String message = "Stop";
                    try {
                        client.publish(stopTopic, message.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }

                }
                return false;
            }
        });

        Left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    String leftMessage = "Left";
                    try {
                        client.publish(leftTopic, leftMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    String message = "Stop";
                    try {
                        client.publish(stopTopic, message.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                }
                return false;
            }
        });

        Right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // button pressed
                    String rightMessage = "Right";
                    try {
                        client.publish(rightTopic, rightMessage.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    // button released.
                    String message = "Stop";
                    try {
                        client.publish(stopTopic, message.getBytes(), 0, false);
                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                }
                return false;
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

        /* client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                if (topic.equals(cameraTopic)) {

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

                    Log.e(topic, "[MQTT] Topic: " + topic + " | Message: " + message.toString());

                }
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {

            }
        });
        */
    }

}