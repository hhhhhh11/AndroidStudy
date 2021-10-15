package com.newland.download.bean;


import com.newland.download.dao.ann.Column;
import com.newland.download.dao.ann.Table;

/**
 * @author lin
 * @version 2021/1/22
 */
@Table(name = "T_USER")
public class User {

    @Column(name = "ID", primaryKey = true)
    private Long id;

    /** 操作员号 */
    @Column(name = "USER_NO")
    private String userNo;

    /** 密码 */
    @Column(name = "PASSWORD")
    private String password;

    /** 00-主管，99-系统管理员，01-一般操作员 */
    @Column(name = "USER_TYPE")
    private Integer userType;

    @Column(name = "ISFIRSTUSE")
    private Integer isFistUse;

    public String getUserNo() {
        return userNo;
    }

    public void setUserNo(String userNo) {
        this.userNo = userNo;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public Integer getUserType() {
        return userType;
    }

    public void setUserType(Integer userType) {
        this.userType = userType;
    }

    public Integer getIsFirstUse() {
        return isFistUse;
    }

    public void setIsFirstUse(Integer isFistUse) {
        this.isFistUse = isFistUse;
    }

    @Override
    public String toString() {
        return "User{" +
                "id=" + id +
                ", userNo='" + userNo + '\'' +
                ", password='" + password + '\'' +
                ", userType=" + userType +
                ", isFistUse=" + isFistUse +
                '}';
    }
}
