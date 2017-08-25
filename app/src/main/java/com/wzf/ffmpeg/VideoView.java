package com.wzf.ffmpeg;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * @Description:
 * @author: wangzhenfei
 * @date: 2017-08-04 08:40
 */

public class VideoView extends SurfaceView {

    public VideoView(Context context) {
        super(context);
        init();
    }

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init(){
        //初始化，SufaceView绘制的像素格式
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
    }

}
