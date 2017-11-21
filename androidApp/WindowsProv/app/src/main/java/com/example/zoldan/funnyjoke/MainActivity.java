package com.example.zoldan.funnyjoke;

import android.annotation.TargetApi;
import android.os.AsyncTask;
import android.os.Build;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;
//import android.support.v7.appcompat.R;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class MainActivity extends AppCompatActivity {

    String serIpAddress;       // адрес сервера
    String login;
    String password;
    int port = 4000;           // порт
    //String msg;                 // Сообщение
    final byte codeAuto = 0;     // Оправить сообщение
    final byte codeSessionList = 1;  // Повернуть экран
    //final byte codePoff = 3;    // Выключить компьютер
    byte codeCommand;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @TargetApi(Build.VERSION_CODES.GINGERBREAD)
    public void onClick (View v)
    {
        EditText etIPaddress = (EditText)findViewById(R.id.edIPaddress);
        serIpAddress = etIPaddress.getText().toString();
        if (serIpAddress.isEmpty()){
            Toast msgToast = Toast.makeText(this, "Введите ip адрес", Toast.LENGTH_SHORT);
            msgToast.show();
            return;
        }
        EditText etMsg = (EditText)findViewById(R.id.etMsg);
        login = etMsg.getText().toString();
        if (login.isEmpty()){
            Toast msgToast = Toast.makeText(this, "Введите login", Toast.LENGTH_SHORT);
            msgToast.show();
            return;
        }
        EditText etPass = (EditText) findViewById(R.id.etPass);
        password = etPass.getText().toString();
        if (password.isEmpty()){
            Toast msgToast = Toast.makeText(this, "Введите password", Toast.LENGTH_SHORT);
            msgToast.show();
            return;
        }
        SenderThread sender = new SenderThread();
     switch (v.getId())
     {
         case R.id.btnSMsg:
             //if (!msg.isEmpty()) {
                 codeCommand = codeAuto;
                 sender.execute();
            // }
            // else {
              //   Toast msgToast = Toast.makeText(this, "Введите сообщение", Toast.LENGTH_SHORT);
               //  msgToast.show();
            // }
             break;
         case R.id.btnRotate:
             codeCommand = codeSessionList;
             sender.execute();
             break;

     }
    }

    class SenderThread extends AsyncTask <Void, Void, Void>
    {
        @Override
        protected Void doInBackground(Void... params) {
            try {
                InetAddress ipAddress = InetAddress.getByName(serIpAddress);
                Socket socket = new Socket(ipAddress, port);
                //InputStream inputStream = socket.getInputStream();
                OutputStream outputStream = socket.getOutputStream();
                DataOutputStream out = new DataOutputStream(outputStream);
                ///DataInputStream in = new DataInputStream(inputStream);
                switch (codeCommand) {
                    case codeAuto:
                        out.write(codeAuto);
                   //     out.flush();
                        Integer sizeLogin = login.length();
                        String result = sizeLogin.toString()+login+password;
                        byte[] outMsg;
                        //Integer sizepaccket = result.length();
                             outMsg =   result.getBytes("UTF8");
                        out.write(outMsg);
                        break;
                    case codeSessionList:
                        out.write(codeSessionList);
                        break;
                    default:
                        out.flush();
                        out.close();
                        socket.close();
                        break;
                     }

            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
            return null;
        }
    }
}
