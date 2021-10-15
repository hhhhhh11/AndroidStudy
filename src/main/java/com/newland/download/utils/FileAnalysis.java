package com.newland.download.utils;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import com.newland.download.R;
import com.newland.download.bean.DownType;
import com.newland.download.bean.DownloadFile;
import com.newland.download.bean.PosPlat;
import com.newland.download.utils.FileUtil;
import com.newland.download.utils.LogUtil;

import java.io.File;
import java.io.IOException;

/**
 * @author lin
 * @version 2021/5/31
 */
public class FileAnalysis {

    private String DOWNTYPE = "DownType";
    private String SLAVECOUNT = "SlaveCount";
    private String SLAVE = "Slave";
    private String MASTER = "Master";

    private String Type = "Type";
    private String GroupNum = "GroupNum";

    private String LIST = ".list";

    private Context mContext;

    public FileAnalysis(Context mContext) {
        this.mContext = mContext;
    }


    private DownloadFile parseError(String error){
        DownloadFile loadFile = new DownloadFile();
        loadFile.setCanDownload(false);
        loadFile.setError(error);
        return loadFile;
    }
    public DownloadFile analysis(String fileName, DownloadFile loadFile){


        File file = new File(fileName);
        if (!file.exists()){
            return parseError("file is no exit!");
        }

        if (fileName.toLowerCase().endsWith(LIST)){
            String content = null;
            try {
                content = FileUtil.readFileByLines2(file);
            } catch (IOException e) {
                e.printStackTrace();
            }
            LogUtil.e(content);
            if (content.contains("[BootHex]")){

                loadFile.setnType(DownType.DOWN_FIRM.getValue());
                loadFile.setDownloadFiles("");
                String[] tempContents2 = content.split("\n");
                if (tempContents2 == null || tempContents2.length < 1) {
                    return parseError("analysis File Error!");
                }
                for (String tempContent : tempContents2) {
                    String[] tags = tempContent.split("=");
                    if (tags != null && tags.length == 2) {
                        if (DOWNTYPE.equals(tags[0])) {
                            switch (tags[1]){
                                case "2":
                                    loadFile.setnPlat(PosPlat.PLAT_LINUX.getValue());
                                    break;
                                case "8":
                                    loadFile.setnPlat(PosPlat.PLAT_ANDROID.getValue());
                                    break;
                                case "5":
                                    loadFile.setnPlat(PosPlat.PLAT_MPOS.getValue());
                                    break;
                                default:
                                    return parseError("Unknown platform type");
                            }
                        }else if (MASTER.endsWith(tags[0]) || (tags[0].startsWith(SLAVE) && !SLAVECOUNT.equals(tags[0]))){
                            loadFile.setDownloadFiles(loadFile.getDownloadFiles() + tags[1] + "\n");
                        }
                    }
                }
            }else if (content.contains("[Set]")){
                loadFile.setnType(DownType.DOWN_APP.getValue());
                loadFile.setDownloadFiles("");

                String[] tempContents2 = content.split("\n");
                if (tempContents2 == null || tempContents2.length < 1) {
                    return parseError("analysis File Error!");
                }
                for (String tempContent : tempContents2) {
                    String[] tags = tempContent.split("=");
                    if (tags != null && tags.length == 2) {
                        if (Type.equals(tags[0])) {
                            switch (tags[1]){
                                case "3":
                                case "10":
                                    loadFile.setnPlat(PosPlat.PLAT_LINUX.getValue());
                                    break;
                                case "7":
                                    loadFile.setnPlat(PosPlat.PLAT_ANDROID.getValue());
                                    break;
                                case "6":
                                    loadFile.setnPlat(PosPlat.PLAT_MPOS.getValue());
                                    break;
                                default:
                                    return parseError("Unknown platform type");
                            }
                        }else if (GroupNum.equals(tags[0])){
                            LogUtil.e("GroupNum:" + tags[1]);
                        }else {
                            loadFile.setDownloadFiles(loadFile.getDownloadFiles() + tags[0] + "\n");
                        }
                    }

                }
            }else {
                return parseError("Node type error");
            }
        }else {
            if (fileName.endsWith(".NLP") && !fileName.startsWith("master") && !fileName.startsWith("mapp")){
                loadFile.setnPlat(PosPlat.PLAT_MPOS.getValue());
                showDialog(loadFile);
            }else if (fileName.endsWith(".NLD")){
                loadFile.setnPlat(PosPlat.PLAT_LINUX.getValue());
                loadFile.setnType(DownType.DOWN_APP.getValue());
            }else if (fileName.endsWith(".NLC")){
                loadFile.setnPlat(PosPlat.PLAT_LINUX.getValue());
                loadFile.setnType(DownType.DOWN_FIRM.getValue());
            }else if (fileName.endsWith(".ard")){
                loadFile.setnPlat(PosPlat.PLAT_ANDROID.getValue());
                loadFile.setnType(DownType.DOWN_FIRM.getValue());
            }else if ((fileName.startsWith("master") || fileName.startsWith("mapp"))){
                return parseError("Please download through the list configuration");
            }else {
                loadFile.setnPlat(PosPlat.PLAT_ANDROID.getValue());
                loadFile.setnType(DownType.DOWN_APP.getValue());
            }
            loadFile.setDownloadFiles(file.getName());
        }

        loadFile.setSzDownFile(fileName);
        loadFile.setSzAppList("");
        loadFile.setCanDownload(true);
        return loadFile;
    }


    private int nlpType = 0;
    private void showDialog(DownloadFile loadFile) {
        String[] item2 = new String[]{mContext.getString(R.string.main_list_type_mapp), mContext.getString(R.string.main_list_type_master)};
        AlertDialog.Builder builder2 = new AlertDialog.Builder(mContext);
        builder2.setTitle(mContext.getString(R.string.dialog_warning));
        builder2.setCancelable(false);
        builder2.setSingleChoiceItems(item2, 0, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub

                nlpType = which;
            }
        });

        builder2.setPositiveButton(mContext.getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                loadFile.setnType(nlpType);
            }
        });

        builder2.setNegativeButton(mContext.getString(R.string.dialog_cancel), new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub

            }
        });
        AlertDialog dialog2 = builder2.create();
        dialog2.show();
    }
}

