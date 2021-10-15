package com.newland.download;

import android.app.Application;


import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @author lin
 * @version 2021/6/8
 */
public class App extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
    }

    public ExecutorService getGlobalThreadPool(){
        return service;
    }

    /**
     * private ExecutorService service = Executors.newSingleThreadExecutor();
     * ThreadFactory namedThreadFactory = new ThreadFactoryBuilder()
     *	.setNameFormat("demo-pool-%d").build();
     */

    ExecutorService service = new ThreadPoolExecutor(1,1,60L, TimeUnit.MICROSECONDS,new LinkedBlockingDeque<Runnable>(1024),
            Executors.defaultThreadFactory(),new ThreadPoolExecutor.AbortPolicy());

}
