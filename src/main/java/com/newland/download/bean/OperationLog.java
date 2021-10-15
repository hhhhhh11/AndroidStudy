package com.newland.download.bean;

import com.newland.download.dao.ann.Column;
import com.newland.download.dao.ann.Table;

/**
 * @author lin
 * @version 2021/1/31
 *
 * 1.登录日志，type=1，user time content-成功/失败
 * 2.导入包，type=2，user time content-导入包名
 * 3.删除包，type=3，user time content-包名+内置/外置
 * 4.安装包，type=4 user time content-包名+成功/失败，是否清空
 */
@Table(name = "T_OPERATION")
public class OperationLog{
    public static final int TYPE_1 = 1;
    public static final int TYPE_2 = 2;
    public static final int TYPE_3 = 3;
    public static final int TYPE_4 = 4;

    @Column(name = "ID", primaryKey = true)
    private Long id;

    /** 日志类型 */
    @Column(name = "TYPE")
    private Integer type;
    /** 操作员号 */
    @Column(name = "USER_NO")
    private String userNo;

    /** 时间 */
    @Column(name = "TIME")
    private String time;

    /** 操作内容*/
    @Column(name = "CONTENT")
    private String content;

    /** 备注*/
    @Column(name = "REMARKS")
    private String remarks;


    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getUserNo() {
        return userNo;
    }

    public void setUserNo(String userNo) {
        this.userNo = userNo;
    }

    public String getTime() {
        return time;
    }

    public void setTime(String time) {
        this.time = time;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public String getRemarks() {
        return remarks;
    }

    public void setRemarks(String remarks) {
        this.remarks = remarks;
    }

    public Integer getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    @Override
    public String toString() {
        return id +
                ".User:" + userNo + "\nTime:" + time + "\n" + content;
    }
}

