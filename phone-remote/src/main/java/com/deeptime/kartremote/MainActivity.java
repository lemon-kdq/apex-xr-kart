package com.deeptime.kartremote;
import android.app.Activity;
import android.os.Bundle;
import android.graphics.Color;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.*;
import android.widget.*;
public class MainActivity extends Activity {
 WebView web; EditText address; TextView status; String host; Pairing pairing; boolean pageRejected=false;
 final android.os.Handler reconnectHandler=new android.os.Handler(android.os.Looper.getMainLooper());
 final Runnable reconnectCheck=new Runnable(){public void run(){if(web!=null&&pairing!=null&&!pairing.prompting){web.evaluateJavascript("typeof lastOK!=='undefined' && !exited && performance.now()-lastOK>3000",value->{if("true".equals(value))pairing.scan();});}reconnectHandler.postDelayed(this,3000);}};
 public void onCreate(Bundle state){super.onCreate(state);getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);getWindow().getDecorView().setSystemUiVisibility(5894);
 LinearLayout layout=new LinearLayout(this);layout.setOrientation(1);layout.setPadding(12,0,12,0);layout.setBackgroundColor(Color.rgb(16,24,29));
 LinearLayout bar=new LinearLayout(this);address=new EditText(this);address.setSingleLine(true);address.setTextColor(Color.WHITE);address.setText(getPreferences(0).getString("host","192.168.78.72"));address.setHint("眼镜 IP 地址");address.setTextSize(15);bar.addView(address,new LinearLayout.LayoutParams(0,-1,1));Button connect=new Button(this);connect.setText("手动连接");bar.addView(connect);Button search=new Button(this);search.setText("搜索眼镜");bar.addView(search);search.setOnClickListener(v->pairing.scan());layout.addView(bar,new LinearLayout.LayoutParams(-1,(int)(40*getResources().getDisplayMetrics().density)));
 status=new TextView(this);status.setTextColor(Color.LTGRAY);status.setText("手机与眼镜连接同一 Wi-Fi，先打开眼镜游戏");layout.addView(status);status.setVisibility(android.view.View.GONE);
 web=new WebView(this);
 // Keep held game controls out of WebView long-click selection/context actions.
 web.setOnLongClickListener(v -> true);
 web.setLongClickable(false);
 web.setHapticFeedbackEnabled(false);
 if(android.os.Build.VERSION.SDK_INT>=29) web.setImportantForContentCapture(android.view.View.IMPORTANT_FOR_CONTENT_CAPTURE_NO_EXCLUDE_DESCENDANTS);
web.setBackgroundColor(Color.rgb(16,24,29));web.getSettings().setJavaScriptEnabled(true);web.getSettings().setDomStorageEnabled(true);web.getSettings().setAllowFileAccess(false);web.getSettings().setAllowContentAccess(false);
 web.setWebViewClient(new WebViewClient(){public boolean shouldOverrideUrlLoading(WebView v,WebResourceRequest r){return !r.getUrl().getHost().equals(host);}public void onPageStarted(WebView v,String url,android.graphics.Bitmap icon){pageRejected=false;} public void onReceivedHttpError(WebView v,WebResourceRequest r,WebResourceResponse response){if(r.isForMainFrame()&&response.getStatusCode()==403){pageRejected=true;pairing.rejected();}} public void onPageFinished(WebView v,String url){
 if(!pageRejected&&url.equals("http://"+host+":8088/"))pairing.accepted();
 v.evaluateJavascript("document.addEventListener('contextmenu',function(e){e.preventDefault();},true);document.addEventListener('selectstart',function(e){e.preventDefault();},true);var st=document.createElement('style');st.textContent='*{-webkit-touch-callout:none;-webkit-user-select:none;user-select:none}';document.head.appendChild(st);",null);
status.setText("眼镜地址："+host+" · 返回键可退出遥控");}public void onReceivedError(WebView v,WebResourceRequest r,WebResourceError e){if(r.isForMainFrame()){pageRejected=true;status.setVisibility(0);status.setText("连接失败，正在重新搜索眼镜…");web.postDelayed(()->{if(!isFinishing()&&pairing!=null)pairing.scan();},3000);}}});
 layout.addView(web,new LinearLayout.LayoutParams(-1,0,1));setContentView(layout);pairing=new Pairing(this);connect.setOnClickListener(v->connect());pairing.scan(); }
 void connect(){String value=address.getText().toString().trim();String[] parts=value.split("\\.");boolean valid=parts.length==4;try{for(String p:parts)valid &= Integer.parseInt(p)>=0&&Integer.parseInt(p)<=255;}catch(Exception e){valid=false;}if(!valid){status.setText("请输入眼镜的 IPv4 地址，例如 192.168.78.72");return;}host=value;getPreferences(0).edit().putString("host",host).apply();status.setText("正在连接…");pairing.manual(host);}
 protected void onPause(){reconnectHandler.removeCallbacks(reconnectCheck);web.evaluateJavascript("if(typeof release==='function')release();",null);web.onPause();super.onPause();}
 protected void onResume(){super.onResume();if(web!=null)web.onResume();reconnectHandler.removeCallbacks(reconnectCheck);reconnectHandler.postDelayed(reconnectCheck,3000);}
 protected void onDestroy(){reconnectHandler.removeCallbacks(reconnectCheck);if(pairing!=null)pairing.closed=true;if(web!=null){web.destroy();}super.onDestroy();}
}
