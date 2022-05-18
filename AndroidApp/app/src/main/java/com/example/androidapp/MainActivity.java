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
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.Toast;


import androidx.appcompat.app.AppCompatActivity;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import static com.example.androidapp.JoystickView.*;

import java.util.ArrayList;
import java.util.Queue;
import java.util.LinkedList;


public class MainActivity extends AppCompatActivity implements JoystickListener{
    private MqttClient mMqttClient;
    private static final String MQTT_SERVER = "tcp://192.168.0.242:1883";
    private static final String TAG = "SmartCarMqttController";
    private static final int QOS = 1;
    static String USERNAME = "admin";
    static String PASSWORD = "emqx";
    private final String topic = "/Group/16";
    private final String controlTopic = "/Group/16/Control";
    private final String streamTopic = "/Group/16/Camera";

    private final String leftDistanceTopic = "/Group/16/Distance/Left";
    private final String rightDistanceTopic = "/Group/16/Distance/Right";
    private final String frontDistanceTopic = "/Group/16/Distance/Front";

    private boolean isConnected = false;

    int IMAGE_WIDTH = 320;
    int IMAGE_HEIGHT = 240;
    private ImageView mCameraView;
    private ProgressBar leftBar;
    private ProgressBar rightBar;
    private ProgressBar middleBar;
    public Queue<Float> yDataPackage = new LinkedList<>();
    public Queue<Float> xDataPackage = new LinkedList<>();
    private String moveMessage;
    private String turnMessage;
    private String moveTopic = "/Group/13/Move";
    private String turnTopic = "/Group/13/Turn";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mMqttClient = new MqttClient(getApplicationContext(), MQTT_SERVER, TAG);
        MqttConnectOptions options = new MqttConnectOptions();
        options.setUserName(USERNAME);//set the username
        options.setPassword(PASSWORD.toCharArray());//set the username
        connectToMqttBroker();

        Button Ahead = findViewById(R.id.buttonAhead);
        Button Back = findViewById(R.id.buttonBack);
        Button Left = findViewById(R.id.buttonLeft);
        Button Right = findViewById(R.id.buttonRight);
        Switch Cruise = (Switch)findViewById(R.id.switchCruise);
        mCameraView = findViewById(R.id.imageView);

        leftBar = findViewById(R.id.progressBarLeft);
        middleBar = findViewById(R.id.progressBarMiddle);
        rightBar = findViewById(R.id.progressBarRight);


        Cruise.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){
            @Override
            public void onCheckedChanged(CompoundButton Cruise, boolean isChecked) {
                String cruiseMessage = "Cruise";
                //Prevent the listener from triggering during initialization
                if (isChecked) {

                        mMqttClient.publish(controlTopic, cruiseMessage, 1, null);

                } else {
                    //return to use manual control;
                    String message = "Stop";
                        mMqttClient.publish(controlTopic, message, 1, null);
                }
            }
        });



        Ahead.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message ="Stop";

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    message = "Forward";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    message = "Stop";
                }
                    mMqttClient.publish(controlTopic, message, 1, null);

                return false;
            }
        });

        Back.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "Stop";

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    message = "Backward";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    message = "Stop";
                }
                    mMqttClient.publish(controlTopic, message, 1, null);
                return false;
            }
        });

        Left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "Stop";

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    message = "Left";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    message = "Stop";
                }
                    mMqttClient.publish(controlTopic, message, 1, null);

                return false;
            }
        });

        Right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                String message = "Stop";

                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    message = "Right";
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    message = "Stop";
                }
                    mMqttClient.publish(controlTopic, message, 1, null);

                return false;
            }
        });


    }
    @Override
    protected void onResume() {
        super.onResume();

        connectToMqttBroker();
    }

    @Override
    protected void onPause() {
        super.onPause();

        mMqttClient.disconnect(new IMqttActionListener() {
            @Override
            public void onSuccess(IMqttToken asyncActionToken) {
                Log.i(TAG, "Disconnected from broker");
            }

            @Override
            public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
            }
        });
    }

    private void connectToMqttBroker() {
        if (!isConnected) {
            mMqttClient.connect(TAG, "", new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    isConnected = true;

                    final String successfulConnection = "Connected to MQTT broker";
                    Log.i(TAG, successfulConnection);
                    Toast.makeText(getApplicationContext(), successfulConnection, Toast.LENGTH_SHORT).show();

                    mMqttClient.subscribe(leftDistanceTopic, QOS, null);
                    Log.e("sub","test");
                    mMqttClient.subscribe(rightDistanceTopic, QOS, null);
                    mMqttClient.subscribe(frontDistanceTopic, QOS, null);
                    mMqttClient.subscribe(streamTopic, QOS, null);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    final String failedConnection = "Failed to connect to MQTT broker";
                    Log.e(TAG, failedConnection);
                    Toast.makeText(getApplicationContext(), failedConnection, Toast.LENGTH_SHORT).show();
                }
            }, new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    isConnected = false;

                    final String connectionLost = "Connection to MQTT broker lost";
                    Log.w(TAG, connectionLost);
                    Toast.makeText(getApplicationContext(), connectionLost, Toast.LENGTH_SHORT).show();
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {

                    if (topic.equals(streamTopic)) {
                        Log.e("Debug","Stream start");
                        final Bitmap bm = Bitmap.createBitmap(IMAGE_WIDTH, IMAGE_HEIGHT, Bitmap.Config.ARGB_8888);

                        final byte[] payload = message.getPayload();
                        final int[] colors = new int[IMAGE_WIDTH * IMAGE_HEIGHT];
                        for (int ci = 0; ci < colors.length; ++ci) {
                            final int r = payload[3 * ci] & 0xFF;
                            final int g = payload[3 * ci + 1] & 0xFF;
                            final int b = payload[3 * ci + 2] & 0xFF;
                            colors[ci] = Color.rgb(r, g, b);
                        }
                        bm.setPixels(colors, 0, IMAGE_WIDTH, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
                        mCameraView.setImageBitmap(bm);
                    }
                    if(topic.equals(leftDistanceTopic)){
                      int progress = Integer.parseInt(message.toString())*10;

                       leftBar.setProgress(progress);
                    }
                    if(topic.equals(rightDistanceTopic)){
                        int progress = Integer.parseInt(message.toString())*10;

                       rightBar.setProgress(progress);
                    }
                    if(topic.equals(frontDistanceTopic)){
                        int progress = Integer.parseInt(message.toString())*10;

                      middleBar.setProgress(progress);
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    Log.d(TAG, "Message delivered");
                }
            });
        }
    }
    public float rectifier(float Percent) {
        ArrayList<Float> Pivot = new ArrayList<>();

        for (float i = 0; i <= 100; i = i + 10) {
            Pivot.add(i);
            for (float y : Pivot) {
                if (y >= Percent) {
                    Percent = y;
                    break;
                }
            }
        }
        return Percent;
    }

    public float pack(Queue<Float> dataPackage) {
        float x1 = 0;
        float x2 = 0;
        float x3 = 0;
        if (!(dataPackage.isEmpty())) {
            x1 = dataPackage.poll();
            x2 = dataPackage.poll();
            x3 = dataPackage.poll();
        }

        if (x1 == x2 && x2 == x3) {
            return x1;
        }

        return x3;
    }

    public void onJoystickMoved(float xPercent, float yPercent, int id) {
        yPercent = 0 - (yPercent * 100);
        xPercent = xPercent * 90;

        if (yPercent < 0) {
            yPercent = 0 - rectifier(Math.abs(yPercent));
        } else {
            yPercent = rectifier(yPercent);
        }
        if (xPercent < 0) {
            xPercent = 0 - rectifier(Math.abs(xPercent));
        } else {
            xPercent = rectifier(xPercent);
        }
        xDataPackage.offer(0.0f);
        yDataPackage.offer(0.0f);
        xDataPackage.offer(0.0f);
        yDataPackage.offer(0.0f);
        xDataPackage.offer(xPercent);
        yDataPackage.offer(yPercent);
        xPercent = pack(xDataPackage);
        yPercent = pack(yDataPackage);

        /*JoystickView joystickView = new JoystickView(Context context);
        float x1, x2, y1, y2;
        double dis;
        x1 = joystickView.getX();
        x2 = joystickView.centerX();
        y1 = joystickView.getY();
        y2 = joystickView.centerY();
        dis = Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));*/


        moveMessage = String.valueOf(yPercent);
        turnMessage = String.valueOf(xPercent);
        Log.d("move", moveMessage + " " + turnMessage);

            mMqttClient.publish(moveTopic, moveMessage, 0, null);
            mMqttClient.publish(turnTopic, turnMessage, 0, null);

    }

}
