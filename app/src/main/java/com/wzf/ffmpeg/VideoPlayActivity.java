package com.wzf.ffmpeg;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.Surface;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.io.File;

import butterknife.Bind;
import butterknife.ButterKnife;
import butterknife.OnClick;

/**
 * @Description:
 * @author: wangzhenfei
 * @date: 2017-08-04 08:38
 */

public class VideoPlayActivity extends AppCompatActivity {
    @Bind(R.id.sv_video)
    VideoView svVideo;
    private String in_video = "1500538734540.mp4";
    private String out_audio = "1500538734540.mp3";
//    private String video = "1500538734540.mp4";
    String input = new File(Environment.getExternalStorageDirectory(), in_video).getAbsolutePath();
    String output = new File(Environment.getExternalStorageDirectory(), out_audio).getAbsolutePath();
    VideoUtils utils;
    Surface surface;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //定义全屏参数
        int flag = WindowManager.LayoutParams.FLAG_FULLSCREEN;
        //获得当前窗体对象
        Window window = getWindow();
        //设置当前窗体为全屏显示
        window.setFlags(flag, flag);
        setContentView(R.layout.activity_video_play);
        ButterKnife.bind(this);
        utils = new VideoUtils();
        surface = svVideo.getHolder().getSurface();
    }

    @OnClick({R.id.btn_video, R.id.btn_audio_java, R.id.btn_audio_opensl, R.id.btn_play})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_video:
                //Surface传入到Native函数中，用于绘制
                utils.playSimpleVideo(input, surface);
                break;
            case R.id.btn_audio_java:
                utils.playSimpleAudio(input, output);
                break;
            case R.id.btn_audio_opensl:
                utils.playSimpleAudioForOpensl(input, output);
                break;
            case R.id.btn_play:
                utils.play(input, surface);
                break;
        }
    }
}
