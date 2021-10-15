package com.newland.download.bean;

import java.util.Arrays;

/**
 * @author lin
 * @version 2021/5/31
 */
public class DownloadFile {

    /**
     * 下载类型
     */
    private int nType;
    /**
     * 下载平台 低中高
     */
    private int nPlat;
    private String szDev;//首页设置
    private String szDownFile;
    private String szAppList;
    private boolean bClear;//首页设置
    private byte[] szResult;

    private String downloadFiles;
    private boolean isCanDownload;
    private String error;

    public int getnType() {
        return nType;
    }

    public void setnType(int nType) {
        this.nType = nType;
    }

    public int getnPlat() {
        return nPlat;
    }

    public void setnPlat(int nPlat) {
        this.nPlat = nPlat;
    }

    public String getSzDev() {
        return szDev;
    }

    public void setSzDev(String szDev) {
        this.szDev = szDev;
    }

    public String getSzDownFile() {
        return szDownFile;
    }

    public void setSzDownFile(String szDownFile) {
        this.szDownFile = szDownFile;
    }

    public String getSzAppList() {
        return szAppList;
    }

    public void setSzAppList(String szAppList) {
        this.szAppList = szAppList;
    }

    public boolean isbClear() {
        return bClear;
    }

    public void setbClear(boolean bClear) {
        this.bClear = bClear;
    }

    public byte[] getSzResult() {
        return szResult;
    }

    public void setSzResult(byte[] szResult) {
        this.szResult = szResult;
    }

    public boolean isCanDownload() {
        return isCanDownload;
    }

    public void setCanDownload(boolean canDownload) {
        isCanDownload = canDownload;
    }

    public String getError() {
        return error;
    }

    public void setError(String error) {
        this.error = error;
    }

    public String getDownloadFiles() {
        return downloadFiles;
    }

    public void setDownloadFiles(String downloadFiles) {
        this.downloadFiles = downloadFiles;
    }

    @Override
    public String toString() {
        return "DownloadFile{" +
                "nType=" + nType +
                ", nPlat=" + nPlat +
                ", szDev='" + szDev + '\'' +
                ", szDownFile='" + szDownFile + '\'' +
                ", szAppList='" + szAppList + '\'' +
                ", bClear=" + bClear +
                ", szResult=" + Arrays.toString(szResult) +
                ", downloadFiles='" + downloadFiles + '\'' +
                ", isCanDownload=" + isCanDownload +
                ", error='" + error + '\'' +
                '}';
    }
}
