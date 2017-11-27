package com.example.zoldan.funnyjoke;

import android.annotation.TargetApi;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.DataOutputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

//import android.support.v7.appcompat.R;

public class MainActivity extends AppCompatActivity {

    String serIpAddress;       // адрес сервера
    String login;
    String password;

    //String msg;                 // Сообщение
    final byte codeAuto = 0;     // Оправить сообщение
    final byte codeSessionList = 1;  // активніе сесии
    //final byte codePoff = 3;    // Выключить компьютер
    byte codeCommand;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Thread serverSocck = new Thread(new ServerSocck());
        serverSocck.start();
    }

    @TargetApi(Build.VERSION_CODES.GINGERBREAD)
    public void onClick(View v) {
        EditText etIPaddress = (EditText) findViewById(R.id.edIPaddress);
        serIpAddress = etIPaddress.getText().toString();
        if (serIpAddress.isEmpty()) {
            Toast msgToast = Toast.makeText(this, "Введите ip адрес", Toast.LENGTH_SHORT);
            msgToast.show();
            return;
        }
        EditText etMsg = (EditText) findViewById(R.id.etMsg);
        login = etMsg.getText().toString();

        EditText etPass = (EditText) findViewById(R.id.etPass);
        password = etPass.getText().toString();


        SenderThread sender = new SenderThread();

        switch (v.getId()) {
            case R.id.btnSMsg:
                if (!login.isEmpty() && !password.isEmpty()) {
                    Log.d("myTag", "you turn button autorization");
                    codeCommand = codeAuto;
                    sender.execute();
                } else {
                    Toast msgToast = Toast.makeText(this, "Please, input all margins", Toast.LENGTH_SHORT);
                    msgToast.show();
                }
                break;
            case R.id.btnRotate:
                Log.d("myTag", "you turn button sessionlist");
                codeCommand = codeSessionList;
                sender.execute();
                break;
        }
    }

    class SenderThread extends AsyncTask<Void, Void, Void> {
        int port = 87; //port

        @Override
        protected Void doInBackground(Void... params) {
            try {
                Log.d("myTag", "doInBackground section");

                InetAddress ipAddress = InetAddress.getByName(serIpAddress);
                Log.d("myTag", "1");
                Log.d("myTag", ipAddress.toString());
                Socket socket= socket = new Socket(ipAddress, port);
                Log.d("myTag", "2");
                //InputStream inputStream = socket.getInputStream();
                OutputStream outputStream = socket.getOutputStream();
                Log.d("myTag", "3");
                DataOutputStream out = new DataOutputStream(outputStream);
                ///DataInputStream in = new DataInputStream(inputStream);
                Log.d("myTag", "4");

                switch (codeCommand) {
                    case codeAuto:
                        //send command what to do
                        try {

                            JSONObject jo = new JSONObject();
                            jo.put("command", codeAuto);
                            jo.put("login", login);
                            jo.put("password", password);
                            out.write(jo.toString().getBytes());
                            Log.d("myTag", "send massege is up to adress");
                            Log.d("myTag", ipAddress.toString());
                            Log.d("myTag", jo.toString());
                        }
                        catch(JSONException ex)
                        {
                            Log.d("Error","Error to create JSON"+ ex.toString());
                        }
                        break;
                    case codeSessionList:
                        try {

                            JSONObject jo = new JSONObject();
                            jo.put("command", codeSessionList);
                        }
                        catch(JSONException ex)
                        {
                            Log.d("Error","Error to create JSON"+ ex.toString());
                        }
                        break;
                    default:
                        out.flush();
                        out.close();
                        socket.close();
                }
            } catch (Exception ex) {
                Log.d("Error", ex.toString());
            }
            return null;
        }
    }


    class ServerSocck extends Thread {

        @Override
        public void run() {
            Log.d("myTag", "doInBackground server section");
            ServerSocket serverSocket = null;
            String message;

            try {

                final int SocketServerPORT = 4008;
                serverSocket = new ServerSocket(SocketServerPORT);

                Log.d("myTag", "1 server section");
                while (true) {
                    Socket socket = serverSocket.accept();
                    Log.d("myTag", "2 server section");

                    message = "#" + " from " + socket.getInetAddress()
                            + ":" + socket.getPort() + "\n";
                    Log.d("myTag", message);
                }
            } catch (Exception ex) {
                Log.d("mytag", ex.toString());
            }
        }
    }
}