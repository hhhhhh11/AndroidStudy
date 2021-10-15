package com.newland.download.activity;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;

import android.os.SystemClock;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.newland.download.MainActivity;
import com.newland.download.R;
import com.newland.download.adapter.FileAdapter;
import com.newland.download.adapter.FileHolder;
import com.newland.download.adapter.TitleAdapter;
import com.newland.download.adapter.base.RecyclerViewAdapter;
import com.newland.download.bean.FileBean;
import com.newland.download.bean.FileType;
import com.newland.download.bean.OperationLog;
import com.newland.download.bean.TitlePath;
import com.newland.download.dao.OperationDao;
import com.newland.download.utils.Constants;
import com.newland.download.utils.FileHelper;
import com.newland.download.utils.FileUtil;
import com.newland.download.utils.LogUtil;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FileMangerActivity extends BaseActivity {

    private RecyclerView title_recycler_view ;
    private RecyclerView recyclerView;
    private FileAdapter fileAdapter;
    private List<FileBean> beanList = new ArrayList<>();
    private File rootFile ;
    private LinearLayout empty_rel ;
    private int PERMISSION_CODE_WRITE_EXTERNAL_STORAGE = 100 ;
    private String rootPath ;
    private TitleAdapter titleAdapter ;
    private List<String> fileNames;
    private File listFile;
    private String currentFolderPath;
    /**
     * 是否显示.list
     */
    private int showList = 0;

    /**
     * 带路径的文件
     */
    private File[] fileArrayWithPath;

    /**
     * 是不是导入文件
     */
    public static boolean isCopy = false;
    /**
     * 是不是删除导入的文件
     */
    public static boolean isDelete = false;

    OperationDao dao;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_file_manger);

        dao = new OperationDao(this);
        //设置Title
        title_recycler_view = (RecyclerView) findViewById(R.id.title_recycler_view);
        title_recycler_view.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL , false ));
        titleAdapter = new TitleAdapter( FileMangerActivity.this , new ArrayList<TitlePath>() ) ;
        title_recycler_view.setAdapter( titleAdapter );

        recyclerView = (RecyclerView) findViewById(R.id.recycler_view);


        fileAdapter = new FileAdapter(this, beanList);

        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        recyclerView.setAdapter(fileAdapter);

        empty_rel = (LinearLayout) findViewById( R.id.empty_rel );
        LogUtil.e("beanList.size() " + beanList.size());
        currentFolderPath = getIntent().getStringExtra(Constants.PATH_KEY);
        fileAdapter.setOnItemClickListener(new RecyclerViewAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, RecyclerView.ViewHolder viewHolder, int position) {
                if ( viewHolder instanceof FileHolder ){
                    FileBean file = beanList.get(position);
                    FileType fileType = file.getFileType() ;
                    String missingFileMessage = "";
                    if ( fileType == FileType.directory) {
                        currentFolderPath = file.getPath();
                        getFile(file.getPath());
                        refreshTitleState( file.getName() , file.getPath() );
                        return;
                    }
                    LogUtil.e("beanList.size() " + beanList.size());
                    LogUtil.e(" position " + position + " | fileType " + fileType);
                    LogUtil.e("file.getPath() " + file.getPath());
//                else if ( fileType == FileType.apk ){
//                        //安装app
//                        FileUtil.openAppIntent( FileMangerActivity.this , new File( file.getPath() ) );
//                    }else if ( fileType == FileType.image ){
//                        FileUtil.openImageIntent( FileMangerActivity.this , new File( file.getPath() ));
//                    }else if ( fileType == FileType.txt ){
//                        FileUtil.openTextIntent( FileMangerActivity.this , new File( file.getPath() ) );
//                    }else if ( fileType == FileType.music ){
//                        FileUtil.openMusicIntent( FileMangerActivity.this ,  new File( file.getPath() ) );
//                    }else if ( fileType == FileType.video ){
//                        FileUtil.openVideoIntent( FileMangerActivity.this ,  new File( file.getPath() ) );
//                    }else {
//                        FileUtil.openApplicationIntent( FileMangerActivity.this , new File( file.getPath() ) );
//                    }
                    if (fileType == FileType.list){
                        listFile = new File(file.getPath());
                        String parentPath = listFile.getParent();
                        LogUtil.e("parentPath " + parentPath);
                        fileNames = analysisListFile(file);
                        if (fileNames.size() == 0 || fileNames == null){
                            LogUtil.e("fileNames is null");
                        }
                        for (String str : fileNames) {
                            LogUtil.e("fileName " + str);
                        }
                        fileArrayWithPath = new File[fileNames.size() + 1];
                        fileArrayWithPath[0] = new File(file.getPath());
                        for (int i = 1;i < fileArrayWithPath.length;i++){
                            fileArrayWithPath[i] = new File( parentPath + "/" + fileNames.get(i - 1));
                            LogUtil.e( " import filePath " + fileArrayWithPath[i].getPath() + "|");
                            if (!fileArrayWithPath[i].exists()){
                                missingFileMessage = missingFileMessage + fileArrayWithPath[i].getName() + " " + getString(R.string.file_not_exsit) + "\n";
                            }
                        }
                        LogUtil.e(" file miss [" + missingFileMessage + "]");
                        if (missingFileMessage != ""){
                            showMissingFileDialog(missingFileMessage);
                            return;
                        }


                        if (isCopy){
                            LogUtil.e(" ----- iscopy --> import");
                            new ListFileCopyAsyncTask().execute(fileArrayWithPath);
                        }
                        if (isDelete) {
                            LogUtil.e(" ----- isDelete --> delete");
                            showDeleteListFileConfirmDialog(fileArrayWithPath);
                        }
                        if (!isCopy && !isDelete){
                            LogUtil.e(" ----- isSelect");
                            Intent intent = new Intent();
                            intent.putExtra(Constants.KEY_FILE_PATH,file.getPath());
                            setResult(1,intent);
                            finish();
                        }
                    }else {
                        Toast.makeText(FileMangerActivity.this, getString(R.string.file_select_please_select_list_file), Toast.LENGTH_SHORT).show();
                    }


                }
            }
        });

        fileAdapter.setOnItemLongClickListener(new RecyclerViewAdapter.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(View view, RecyclerView.ViewHolder viewHolder, int position) {
                if ( viewHolder instanceof FileHolder){
                    FileBean fileBean = (FileBean) fileAdapter.getItem( position );
                    FileType fileType = fileBean.getFileType() ;
//                    if ( fileType != null && fileType != FileType.directory ){
//                        FileHelper.sendFile( FileMangerActivity.this , new File( fileBean.getPath() ) );
//                    }
                //长按删除
//                    if (Constants.isAdmin){
//                        showNormalDialog(new File(fileBean.getPath()));
//                        fileAdapter.notifyDataSetChanged();
//                    }

                }

                return false;
            }
        });

        titleAdapter.setOnItemClickListener(new RecyclerViewAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, RecyclerView.ViewHolder viewHolder, int position) {
                TitlePath titlePath = (TitlePath) titleAdapter.getItem( position );
                currentFolderPath = titlePath.getPath();
                getFile( titlePath.getPath() );

                int count = titleAdapter.getItemCount() ;
                int removeCount = count - position - 1 ;
                for ( int i = 0 ; i < removeCount ; i++ ){
                    titleAdapter.removeLast();
                }
            }
        });

        isCopy = getIntent().getBooleanExtra("import",false);
        isDelete = getIntent().getBooleanExtra("delete",false);
//        rootPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        rootPath = getIntent().getStringExtra(Constants.PATH_KEY);
        if (rootPath == null){
            rootPath = Constants.rootPath;
        }
        String name = getIntent().getStringExtra(Constants.PATH_NAME_KEY);
        if (name == null){
            name = getString(R.string.file_mk_sd);
        }
        refreshTitleState(name, rootPath );


        // 有权限，直接do anything.
        getFile(rootPath);

    }

    private ProgressDialog progressDialog;
    FileBean fileBean1;
    class LogCopyAsyncTask extends AsyncTask<Integer, String, Integer> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progressDialog = ProgressDialog.show(FileMangerActivity.this,"",getString(R.string.file_importing));
        }

        @Override
        protected Integer doInBackground(Integer... params) {
            String importFileNames = "";
            fileBean1 = beanList.get(params[0]);
            LogUtil.e("fileBean1 " + fileBean1.getName());
            LogUtil.e("fileBean1 path " + fileBean1.getPath());
            FileUtil.copyFileByBytes(new File(fileBean1.getPath()),new File(Constants.downloadFile,fileBean1.getName()));
            importFileNames = importFileNames + fileBean1.getName() + "\n";
            LogUtil.e("importFileNames " + importFileNames);

            dao.updateLog(FileMangerActivity.this, OperationLog.TYPE_2,getString(R.string.file_import) + " " +  importFileNames);
            return null;
        }

        @Override
        protected void onProgressUpdate(final String... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPostExecute(Integer integer) {
            super.onPostExecute(integer);
            Toast.makeText(FileMangerActivity.this, R.string.file_import_succ,Toast.LENGTH_SHORT).show();
//            dao.updateLog(FileMangerActivity.this, OperationLog.TYPE_2,getString(R.string.file_import) + " " +  fileBean1.getName());
            progressDialog.dismiss();
        }
    }

    class ListFileCopyAsyncTask extends AsyncTask<File[], String, Integer> {
        String copyResultSuccess = "";
        String copyResultFail = "";
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progressDialog = ProgressDialog.show(FileMangerActivity.this,"",getString(R.string.file_importing));
        }

        @Override
        protected Integer doInBackground(File[]... params) {
            String importFileNames = "";
            boolean copySuccess = false;
            File destFile;
            LogUtil.e(" params.length " + params.length + " | params[0].length " + params[0].length);
            for (File file : params[0]) {
                destFile = new File(Constants.downloadFile,file.getName());
                copySuccess = FileUtil.copyFileByBytes(file,destFile);
                if (copySuccess){
                    copyResultSuccess += "√ : " + file.getName() + "\n";
                }else {
                    copyResultFail += "x : " + file.getName() + "\n";
                }
                try {
                    Runtime.getRuntime().exec("chmod 777 " + destFile.getAbsolutePath());
                } catch (IOException e) {
                    e.printStackTrace();
                }
                importFileNames = importFileNames + file.getName() + "\n";
            }
            LogUtil.e(" importFileNames " + importFileNames);
            LogUtil.e(" copyResult " + copyResultSuccess + copyResultFail );
            dao.updateLog(FileMangerActivity.this, OperationLog.TYPE_2,getString(R.string.file_import) + " " +  importFileNames);
            return null;
        }

        @Override
        protected void onProgressUpdate(final String... values) {
            super.onProgressUpdate(values);
        }

        @Override
        protected void onPostExecute(Integer integer) {
            super.onPostExecute(integer);
//            Toast.makeText(FileMangerActivity.this, copyResult,Toast.LENGTH_LONG).show();
            showImportResultDialog(copyResultSuccess,copyResultFail);
            progressDialog.dismiss();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.toolbar,menu);
        if (Constants.isAdmin){
            menu.findItem(R.id.item_all).setVisible(true);
            menu.findItem(R.id.item_list).setVisible(true);
        }else {
            menu.findItem(R.id.item_all).setVisible(false);
            menu.findItem(R.id.item_list).setVisible(false);
        }
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case R.id.item_all:
                showList = 0;
                if (isDelete){
                    getFile(rootPath);
                }else {
                    getFile(currentFolderPath);
                }
                break;
            case R.id.item_list:
                showList = 1;
                if (isDelete){
                    getFile(rootPath);
                }else {
                    getFile(currentFolderPath);
                }
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showNormalDialog(File file){

        final AlertDialog.Builder normalDialog =
                new AlertDialog.Builder(this);
        normalDialog.setTitle(getString(R.string.dialog_warning));
        normalDialog.setMessage(getString(R.string.dialog_delete));
        normalDialog.setPositiveButton(getString(R.string.dialog_confirm),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //...To-do
                        file.delete();
                        dao.updateLog(FileMangerActivity.this, OperationLog.TYPE_3,getString(R.string.file_delete) + " " +  file.getName());
                        getFile(rootPath);
                    }
                });
        normalDialog.setNegativeButton(getString(R.string.dialog_cancel),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //...To-do
                    }
                });
        normalDialog.show();
    }

    private void showDeleteListFileConfirmDialog(File[] fileArray){
        final AlertDialog.Builder normalDialog =
                new AlertDialog.Builder(this);
        String deleteFileNames = "";
        for (int i = 0; i < fileArray.length; i++) {
            LogUtil.e("delete file " + i + " --> " + fileArray[i].getName());
            deleteFileNames = deleteFileNames + fileArray[i].getName() + "\n";
        }
        LogUtil.e(" deleteFileNames : " + deleteFileNames);
        normalDialog.setTitle(getString(R.string.dialog_warning));
        normalDialog.setMessage(getString(R.string.dialog_delete) + " :\n" +  deleteFileNames);
        String finalDeleteFileNames = deleteFileNames;
        normalDialog.setPositiveButton(getString(R.string.dialog_confirm),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //...To-do
                        for (int i = 0; i < fileArray.length; i++) {
                            LogUtil.e("fileArray[i] exists " + fileArray[i].exists());
                            fileArray[i].delete();
                        }
                        dao.updateLog(FileMangerActivity.this, OperationLog.TYPE_3,getString(R.string.file_delete) + " " +  finalDeleteFileNames);
                        getFile(rootPath);
                    }
                });
        normalDialog.setNegativeButton(getString(R.string.dialog_cancel),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //...To-do
                    }
                });
        normalDialog.show();
    }

    public void getFile(String path ) {
        LogUtil.e("current path " + path);
        rootFile = new File( path + File.separator  )  ;
        new MyTask(rootFile).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR , "") ;
    }

    class MyTask extends AsyncTask {
        File file;

        MyTask(File file) {
            this.file = file;
        }

        @Override
        protected Object doInBackground(Object[] params) {
            List<FileBean> fileBeenList = new ArrayList<>();
            if ( file.isDirectory() ) {
                File[] filesArray = file.listFiles();
                if ( filesArray != null ){
                    List<File> fileList = new ArrayList<>() ;
                    Collections.addAll( fileList , filesArray ) ;  //把数组转化成list
                    Collections.sort( fileList , FileHelper.comparator );  //按照名字排序

                    for (File f : fileList ) {
                        if (f.isHidden()) {
                            continue;
                        }
                        LogUtil.e("showList " + showList);
                        if (!Constants.isAdmin){
                            if (FileHelper.getFileType(f) == FileType.list){
                                FileBean fileBean = new FileBean();
                                fileBean.setName(f.getName());
                                fileBean.setPath(f.getAbsolutePath());
                                fileBean.setFileType( FileHelper.getFileType( f ));
                                fileBean.setChildCount( FileHelper.getFileChildCount( f ));
                                fileBean.setSize( f.length() );
                                fileBean.setHolderType( 0 );

                                fileBeenList.add(fileBean);

                                FileBean lineBean = new FileBean();
                                lineBean.setHolderType( 1 );
                                fileBeenList.add(lineBean);
                            }
                        }else {
                            if (showList == 1){
                                if (FileHelper.getFileType(f) == FileType.list || FileHelper.getFileType(f) == FileType.directory){
                                    FileBean fileBean = new FileBean();
                                    fileBean.setName(f.getName());
                                    fileBean.setPath(f.getAbsolutePath());
                                    fileBean.setFileType( FileHelper.getFileType( f ));
                                    fileBean.setChildCount( FileHelper.getFileChildCount( f ));
                                    fileBean.setSize( f.length() );
                                    fileBean.setHolderType( 0 );
                                    fileBeenList.add(fileBean);
                                    FileBean lineBean = new FileBean();
                                    lineBean.setHolderType( 1 );
                                    fileBeenList.add(lineBean);
                                }
                            }else {
                                FileBean fileBean = new FileBean();
                                fileBean.setName(f.getName());
                                fileBean.setPath(f.getAbsolutePath());
                                fileBean.setFileType(FileHelper.getFileType(f));
                                fileBean.setChildCount(FileHelper.getFileChildCount(f));
                                fileBean.setSize(f.length());
                                fileBean.setHolderType(0);

                                fileBeenList.add(fileBean);

                                FileBean lineBean = new FileBean();
                                lineBean.setHolderType(1);
                                fileBeenList.add(lineBean);
                            }

                        }
                    }
                }
            }

            beanList = fileBeenList;
            return fileBeenList;
        }

        @Override
        protected void onPostExecute(Object o) {
            if ( beanList.size() > 0  ){
                empty_rel.setVisibility( View.GONE );
            }else {
                empty_rel.setVisibility( View.VISIBLE );
            }
            fileAdapter.refresh(beanList);
        }
    }

    void refreshTitleState( String title , String path ){
        TitlePath filePath = new TitlePath() ;
        filePath.setNameState( title + " > " );
        filePath.setPath( path );
        titleAdapter.addItem( filePath );
        title_recycler_view.smoothScrollToPosition( titleAdapter.getItemCount());
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK
                && event.getRepeatCount() == 0) {
            List<TitlePath> titlePathList = (List<TitlePath>) titleAdapter.getAdapterData();
            if ( titlePathList.size() == 1 ){
                finish();
            }else {
                titleAdapter.removeItem( titlePathList.size() - 1 );
                currentFolderPath = titlePathList.get(titlePathList.size() - 1 ).getPath();
                getFile( titlePathList.get(titlePathList.size() - 1 ).getPath() );
            }
            fileAdapter.notifyDataSetChanged();
            return true;
        }
        if (keyCode == KeyEvent.KEYCODE_HOME){
            finishAllActivity();
        }
        return super.onKeyDown(keyCode, event);
    }

    private void showNoSelectDialog(){
        final android.app.AlertDialog.Builder noDeviceDialog = new android.app.AlertDialog.Builder(FileMangerActivity.this);
        noDeviceDialog.setTitle(getString(R.string.file_not_select_file));
        noDeviceDialog.setMessage(getString(R.string.file_please_select_file));
        noDeviceDialog.setPositiveButton(getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        noDeviceDialog.show();
    }

    private void showMissingFileDialog(String message){
        final android.app.AlertDialog.Builder missingFileDialogBuilder = new android.app.AlertDialog.Builder(FileMangerActivity.this);
        missingFileDialogBuilder.setTitle(getString(R.string.file_missing));
        missingFileDialogBuilder.setMessage(message);
        missingFileDialogBuilder.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        android.app.AlertDialog missingDialog = missingFileDialogBuilder.create();
        missingDialog.show();
        missingDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                switch (keyCode){
                    case KeyEvent.KEYCODE_HOME:
                        missingDialog.dismiss();
                        finishAllActivity();
                        return true;
                    case KeyEvent.KEYCODE_BACK:
                        missingDialog.dismiss();
                        return true;
                }
                return false;
            }
        });
    }

    private void showImportResultDialog(String successMessage,String failMessage){
        final android.app.AlertDialog.Builder missingFileDialog = new android.app.AlertDialog.Builder(FileMangerActivity.this);
        missingFileDialog.setTitle(getString(R.string.file_import));
        missingFileDialog.setMessage(successMessage + failMessage);
        missingFileDialog.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        missingFileDialog.show();
    }

    private List<String> analysisListFile(FileBean fileBean){
        File file = new File(fileBean.getPath());
        String fileContent = null;
        String[] tempContent;
        String[] tags;
        List<String> fileNames = new ArrayList<>();
        try {
            fileContent = FileUtil.readFileByLines2(file);
        } catch (IOException e) {
            e.printStackTrace();
        }
        LogUtil.e(" fileContent:  " + fileContent);
        if (fileContent.contains("[Set]") ){
            tempContent = fileContent.split("\n");
            if (tempContent == null || tempContent.length < 1){
                return null;
            }
            for (String tempStr : tempContent) {
                tags = tempStr.split("=");
                if (tags != null && tags.length == 2){
                    if ("Type".equals(tags[0])){

                    }else if ("GroupNum".equals(tags[0])){
                        LogUtil.e(" GroupNum: " + tags[1]);
                    }else {
                        LogUtil.e("[Set] fileName " + tags[0]);
                        fileNames.add(tags[0]);
                    }
                }
            }
        }else if (fileContent.contains("[BootHex]")){
            tempContent = fileContent.split("\n");
            if (tempContent == null || tempContent.length < 1){
                return null;
            }
            for (String tempStr : tempContent) {
                tags = tempStr.split("=");
                if (tags != null && tags.length == 2){
                    if ("DownType".equals(tags[0])){

                    }else if ("Restore".equals(tags[0])){

                    }else if ("SlaveCount".equals(tags[0])){

                    } else if ("Master".equals(tags[0]) || tags[0].startsWith("Slave")){
                        LogUtil.e("[BootHex] fileName " + tags[1]);
                        fileNames.add(tags[1]);
                    }
                }
            }
        }
        return fileNames;
    }

}