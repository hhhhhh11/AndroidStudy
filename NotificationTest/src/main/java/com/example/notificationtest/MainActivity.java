package com.example.notificationtest;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.NotificationCompat;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btn_sendNotice = (Button) findViewById(R.id.button_send_notice);
        btn_sendNotice.setOnClickListener(this);
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.button_send_notice:
                Intent intent = new Intent(this,NotificationActivity.class);
                PendingIntent pendingIntent = PendingIntent.getActivity(this,0,intent,0);
                NotificationManager notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
                Notification notification = new NotificationCompat.Builder(this)
                        .setContentTitle(getResources().getString(R.string.content_title))
                        .setContentText(getResources().getString(R.string.content_text))
                        .setWhen(System.currentTimeMillis())
                        .setSmallIcon(R.mipmap.labixiaoxin_icon)
                        .setLargeIcon(BitmapFactory.decodeResource(getResources(),R.mipmap.labixiaoxin_icon))
                        .setContentIntent(pendingIntent)
                        .setAutoCancel(true)
//                        .setStyle(new NotificationCompat.BigTextStyle().bigText(getResources().getString(R.string.content_text_long)))
//                        .setStyle(new NotificationCompat.BigPictureStyle().bigPicture(BitmapFactory.decodeResource(getResources(),R.mipmap.labixiaoxin_icon)))
                        .setPriority(NotificationCompat.PRIORITY_MAX)
                        .build();
                notificationManager.notify(1,notification);
                break;
            default:
                break;
        }
    }
}