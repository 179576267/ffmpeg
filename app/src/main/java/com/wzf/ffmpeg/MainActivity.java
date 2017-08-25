package com.wzf.ffmpeg;

import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import butterknife.Bind;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class MainActivity extends AppCompatActivity {

    @Bind(R.id.tv_result)
    TextView tvResult;
    String input = new File(Environment.getExternalStorageDirectory(), "ffmpeg_test.mp4").getAbsolutePath();
    VideoUtils utils;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        utils = new VideoUtils();
        Utils.getUUid();

    }

    @OnClick({R.id.btn_video_info, R.id.btn_encode, R.id.btn_player, R.id.btn_thread})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_video_info:
                tvResult.setText(utils.getVideoInfo(input));
                break;
            case R.id.btn_encode:
                new  Thread(new Runnable() {
                    @Override
                    public void run() {
                        String output = new File(Environment.getExternalStorageDirectory(), "output_1280x720_yuv420p.yuv").getAbsolutePath();
                        utils.decode(input, output);
                    }
                }).start();

                break;
            case R.id.btn_player:
                startActivity(new Intent(this, VideoPlayActivity.class));
                break;
            case R.id.btn_thread:
                utils.initThread();
                utils.threadTest();
                break;
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
}
