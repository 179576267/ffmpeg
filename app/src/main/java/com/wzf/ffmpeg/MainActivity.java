package com.wzf.ffmpeg;

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
    String input = new File(Environment.getExternalStorageDirectory(), "input.wmv").getAbsolutePath();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

    }

    @OnClick({R.id.btn_video_info, R.id.btn_encode})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_video_info:
                tvResult.setText(VideoUtils.getVideoInfo(input));
                break;
            case R.id.btn_encode:
                String output = new File(Environment.getExternalStorageDirectory(), "output_1280x720_yuv420p.yuv").getAbsolutePath();
                VideoUtils.decode(input, output);
                break;
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
}
