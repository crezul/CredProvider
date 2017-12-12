package apriorit.windowscredential;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.regex.Pattern;


public class MainActivity extends AppCompatActivity {

    private final byte codeAuto = 0;     // send to autorization
    private final byte codeSessionList = 1;  // active session
    private byte codeCommand;
    //
    private ArrayList<String> sessionlist;
    private ArrayAdapter<String> listadapter;

    private String serIpAddress;       // аdress of server
    private String login;   //to send server
    private String password;//to send server

    private String sesiontolist;
    private String domentolist;


    public String getSesiontolist() {
        return sesiontolist;
    }

    public void setSesiontolist(String sesiontolist) {
        this.sesiontolist = sesiontolist;
    }

    public String getDomentolist() {
        return domentolist;
    }

    public void setDomentolist(String domentolist) {
        this.domentolist = domentolist;
    }

    public String getSerIpAddress() {
        return serIpAddress;
    }

    public void setSerIpAddress(String serIpAddress) {
        this.serIpAddress = serIpAddress;
    }

    public String getLogin() {
        return login;
    }

    public void setLogin(String login) {
        this.login = login;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public byte getCodeAuto() {
        return codeAuto;
    }

    public byte getCodeSessionList() {
        return codeSessionList;
    }

    public byte getCodeCommand() {
        return codeCommand;
    }

    public void setCodeCommand(byte codeCommand) {
        this.codeCommand = codeCommand;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ListView listview = (ListView) findViewById(R.id.sessionview);
        //adapter
        sessionlist = new ArrayList<>();
        listadapter = new ArrayAdapter<String>(this, R.layout.support_simple_spinner_dropdown_item, sessionlist);
        listview.setAdapter(listadapter);
    }

    @TargetApi(Build.VERSION_CODES.GINGERBREAD)
    public void onClick(View v) {
        EditText etIPaddress = (EditText) findViewById(R.id.edIPaddress);
        setSerIpAddress(etIPaddress.getText().toString());
        if (getSerIpAddress().isEmpty()) {
            Toast msgToast = Toast.makeText(this, "Input IP point, please", Toast.LENGTH_SHORT);
            msgToast.show();
            return;
        } else {
            //volidate
            IPAddressValidator validator = new IPAddressValidator();
            boolean result = validator.validate(etIPaddress.getText().toString());
            if (!result) {
                Toast msgToast = Toast.makeText(this, "Input volidate IP adress ", Toast.LENGTH_SHORT);
                msgToast.show();
                Log.d("Programm Logger :", "IP-adress not volidate");
                return;
            } else {
                Log.d("Programm Logger :", "IP-adress  volidate");
            }

        }
        EditText etLogin = (EditText) findViewById(R.id.etLogin);
        setLogin(etLogin.getText().toString().trim()); // delete _ in login

        EditText etPass = (EditText) findViewById(R.id.etPass);
        setPassword(etPass.getText().toString());

// controller
        SenderThread sender = new SenderThread(); // class to send and recieve message
        // create one theard in this place
        switch (v.getId()) {
            case R.id.btnAutorization:
                if (!getLogin().isEmpty() && !getPassword().isEmpty()) {
                    setCodeCommand(codeAuto);
                    Log.d("Programm Logger :", "you turn button autorization");
                    sender.start();
                } else {
                    Toast msgToast = Toast.makeText(this, "Please, input all margins", Toast.LENGTH_SHORT);
                    msgToast.show();
                }
                break;
            case R.id.btnActiveSessions:
                setCodeCommand(codeSessionList);
                Log.d("Programm Logger :", "you turn button sessionlist");
                sender.start();
                listcclear();
                break;
        }
    }

    protected Void listconroller() {
        String member;
        if (getDomentolist().equals("@"))
            setDomentolist("system@PC");
        member = "session " + getSesiontolist() + ": " + getDomentolist();
        sessionlist.add(member);
        return null;
    }

    protected Void changedlist() {
        listadapter.notifyDataSetChanged(); // changed
        return null;
    }

    protected Void listcclear() {
        sessionlist.clear();
        listadapter.notifyDataSetChanged(); // changed
        return null;
    }



    //
    class SenderThread extends Thread {
        private final int port = 87; //port to socket
        private Socket socket;
        private DataOutputStream out;
        private DataInputStream in;

        public int getPort() {
            return port;
        }

        public void run() {
            Handler handler = new Handler(getBaseContext().getMainLooper());
            try {
                try {
                    InetAddress ipAddress = InetAddress.getByName(getSerIpAddress());
                    Log.d("Programm Logger :", "IP server: " + ipAddress.toString());
                    socket = new Socket(ipAddress, getPort());
                } catch (IOException e) {
                    Log.d("Error", "Error to create socket" + e.toString());
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast toast = Toast.makeText(getApplicationContext(),
                                    "Error to connect with PC", Toast.LENGTH_SHORT);
                            toast.show();
                        }
                    });
                    return;
                }
// check connection
                //  if (!socket.getInetAddress().isReachable(1000))
                //   return null;

                try {
                    switch (getCodeCommand()) {

                        case codeAuto:
                            //send command what to do
                            if (requestServerAuthentication(socket)) {
                                handler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast toast = Toast.makeText(getApplicationContext(),
                                                "Sucess : send message on PC", Toast.LENGTH_SHORT);
                                        toast.show();
                                    }
                                });
                            }
                            break;
                        case codeSessionList:
                            if (requestServerSessionList(socket)) {
                                handler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast toast = Toast.makeText(getApplicationContext(),
                                                "Sucess : send message on PC", Toast.LENGTH_SHORT);
                                        toast.show();
                                    }
                                });
                            }
                            //respone
                            if (responeServerSessionList(socket)) {

                                handler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        changedlist();  // update the interface
                                    }
                                });
                            }
                            break;
                    }
                } catch (JSONException ex) {
                    out.close();
                    in.close();
                    Log.d("Error", "Error to create JSON" + ex.toString());
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast toast = Toast.makeText(getApplicationContext(),
                                    "Error to send message on PC", Toast.LENGTH_SHORT);
                            toast.show();
                        }
                    });
                } catch (IOException e) {
                    Log.d("Error", "Error of write to server" + e.toString());
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast toast = Toast.makeText(getApplicationContext(),
                                    "Error to send message on PC", Toast.LENGTH_SHORT);
                            toast.show();
                        }
                    });
                }
            } catch (Exception ex) {
                Log.d("Error of : ", ex.toString());
            } finally {
                if (socket != null) {
                    try {
                        socket.close();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        Log.d("Error", "Error of close socket" + e.toString());
                    }
                }
            }
            return;
        }

        private boolean requestServerAuthentication(Socket socket) throws JSONException, IOException {
            try {
                OutputStream outputStream = socket.getOutputStream();
                out = new DataOutputStream(outputStream);
                // BufferedWriter out = new BufferedWriter((new OutputStreamWriter(socket.getOutputStream())));
                JSONObject jo = new JSONObject();
                jo.put("command", getCodeAuto());
                jo.put("login", getLogin());
                jo.put("password", getPassword());
                out.write(jo.toString().getBytes());
                Log.d("Programm Logger :", "send massege...");
                Log.d("Programm Logger :", jo.toString());
            } catch (JSONException ex) {
                throw ex;
            } catch (IOException e) {
                throw e;
            }

            return true;
        }

        private boolean requestServerSessionList(Socket socket) throws JSONException, IOException {
            try {
                OutputStream outputStream = socket.getOutputStream();
                out = new DataOutputStream(outputStream);
                JSONObject jo = new JSONObject();
                jo.put("command", getCodeSessionList());
                out.write(jo.toString().getBytes());
                Log.d("Programm Logger :", "Send to server: " + jo.toString());
            } catch (JSONException ex) {
                throw ex;
            } catch (IOException e) {
                throw e;
            }
            return true;
        }

        private boolean responeServerSessionList(Socket socket) throws JSONException, IOException {

            try {
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                String respone = in.readLine();

                Log.d("Programm Logger :", "Respone for server = " + respone);
                JSONObject jorespone = new JSONObject(respone);
                int count = jorespone.getInt("count");
                for (Integer i = 0; i < count; i++) {
                    setSesiontolist(jorespone.getString("session" + i.toString()));
                    setDomentolist(jorespone.getString("domenname" + i.toString()));
                    Log.d("Programm Logger :", "JSON session " + getSesiontolist());
                    Log.d("Programm Logger :", "JSON domenname " + getDomentolist());
                    listconroller(); // changed arraylist
                }

            } catch (JSONException ex) {
                throw ex;
            } catch (IOException e) {
                throw e;
            }
            return true;
        }
    }
}

