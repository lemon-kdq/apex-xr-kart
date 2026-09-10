package com.deeptime.kartremote;
import android.app.AlertDialog;
import android.text.InputType;
import android.widget.EditText;
import java.net.*;
import java.util.*;
import org.json.JSONObject;

final class Pairing {
 final MainActivity activity; volatile boolean scanning=false,closed=false; boolean prompting=false;
 String deviceId="";
 Pairing(MainActivity a){activity=a;}
 void scan(){if(scanning||closed||prompting)return;scanning=true;activity.status.setVisibility(0);activity.status.setText("正在搜索局域网眼镜…");
 new Thread(()->{Map<String,String> found=new LinkedHashMap<>();try(DatagramSocket socket=new DatagramSocket()){
  socket.setBroadcast(true);socket.setSoTimeout(350);byte[] magic="APEX_DISCOVER_V1".getBytes("UTF-8");
  for(int round=0;round<3&&!closed;round++){
   Set<InetAddress> destinations=new HashSet<>();destinations.add(InetAddress.getByName("255.255.255.255"));
   Enumeration<NetworkInterface> interfaces=NetworkInterface.getNetworkInterfaces();while(interfaces.hasMoreElements())for(InterfaceAddress address:interfaces.nextElement().getInterfaceAddresses())if(address.getBroadcast()!=null)destinations.add(address.getBroadcast());
   for(InetAddress destination:destinations)try{socket.send(new DatagramPacket(magic,magic.length,destination,8089));}catch(Exception ignored){}
   long deadline=System.currentTimeMillis()+700;while(System.currentTimeMillis()<deadline&&!closed){try{byte[] buffer=new byte[200];DatagramPacket packet=new DatagramPacket(buffer,buffer.length);socket.receive(packet);String[] parts=new String(buffer,0,packet.getLength(),"UTF-8").split(" ");if(parts.length==3&&parts[0].equals("APEX_XR_V1")&&parts[1].matches("[0-9]{1,30}")&&parts[2].equals("8088"))found.put(parts[1],packet.getAddress().getHostAddress());}catch(SocketTimeoutException ignored){}}
  }
 }catch(Exception ignored){}
 activity.runOnUiThread(()->{scanning=false;if(closed)return;if(found.isEmpty()){activity.status.setText("未发现眼镜：打开眼镜游戏，确认同一 Wi-Fi；也可手动连接");return;}
  String remembered=activity.getPreferences(0).getString("pairedId","");if(found.containsKey(remembered)){open(found.get(remembered),remembered);return;}
  if(found.size()==1){Map.Entry<String,String> item=found.entrySet().iterator().next();open(item.getValue(),item.getKey());return;}
  String[] ids=found.keySet().toArray(new String[0]),labels=new String[ids.length];for(int i=0;i<ids.length;i++)labels[i]="APEX XR · "+found.get(ids[i]);new AlertDialog.Builder(activity).setTitle("选择眼镜").setItems(labels,(dialog,index)->open(found.get(ids[index]),ids[index])).setNegativeButton("取消",null).show();
 });}).start();}
 void manual(String ip){activity.status.setVisibility(0);activity.status.setText("正在确认眼镜…");new Thread(()->{try{HttpURLConnection c=(HttpURLConnection)new URL("http://"+ip+":8088/identity").openConnection();c.setConnectTimeout(2000);c.setReadTimeout(2000);String body;try(java.io.InputStream in=c.getInputStream()){java.io.ByteArrayOutputStream out=new java.io.ByteArrayOutputStream();byte[] b=new byte[1024];int n;while((n=in.read(b))>0){out.write(b,0,n);if(out.size()>4096)throw new Exception();}body=out.toString("UTF-8");}finally{c.disconnect();}String id=new JSONObject(body).getString("id");activity.runOnUiThread(()->{if(!closed)open(ip,id);});}catch(Exception e){activity.runOnUiThread(()->activity.status.setText("连接失败，请检查眼镜地址和网络"));}}).start();}
 void open(String ip,String id){deviceId=id;activity.host=ip;activity.address.setText(ip);String pin=activity.getPreferences(0).getString("pin_"+id,"");if(pin.isEmpty())prompt();else load(pin);}
 void load(String pin){Map<String,String> headers=new HashMap<>();headers.put("X-APEX-PIN",pin);activity.web.loadUrl("http://"+activity.host+":8088/",headers);}
 void prompt(){if(prompting||closed)return;prompting=true;EditText field=new EditText(activity);field.setInputType(InputType.TYPE_CLASS_NUMBER|InputType.TYPE_NUMBER_VARIATION_PASSWORD);field.setHint("眼镜赛场前方的六位数字");field.setFilters(new android.text.InputFilter[]{new android.text.InputFilter.LengthFilter(6)});
 AlertDialog dialog=new AlertDialog.Builder(activity).setTitle("首次配对 APEX XR").setMessage("输入眼镜中显示的六位配对码。成功后将记住这台眼镜。").setView(field).setPositiveButton("配对",null).setNegativeButton("取消",null).create();dialog.setOnDismissListener(d->prompting=false);dialog.setOnShowListener(d->dialog.getButton(-1).setOnClickListener(v->{String pin=field.getText().toString();if(!pin.matches("[0-9]{6}")){field.setError("请输入六位数字");return;}activity.getPreferences(0).edit().putString("pin_"+deviceId,pin).apply();dialog.dismiss();load(pin);}));dialog.show();}
 void rejected(){activity.getPreferences(0).edit().remove("pin_"+deviceId).apply();activity.status.setVisibility(0);activity.status.setText("配对码不正确，请重试");prompt();}
 void accepted(){activity.getPreferences(0).edit().putString("pairedId",deviceId).putString("host",activity.host).apply();activity.status.setText("已配对 · "+activity.host);}
}
