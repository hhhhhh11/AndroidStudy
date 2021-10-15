package com.newland.download.dao;

import java.lang.reflect.Field;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

import com.newland.download.bean.OperationLog;
import com.newland.download.bean.User;
import com.newland.download.dao.ann.Column;
import com.newland.download.dao.ann.Table;


public class DbHelper extends SQLiteOpenHelper{
	
	public static final String TAG = DbHelper.class.getName();
	
	public static final String DB_NAME = "NlDownload.db";
	
	private static DbHelper helper;
	
	public static final int DB_VERSION = 1; // 数据库版本号

	
	public static synchronized DbHelper getInstance(Context context) {
		if (helper == null) {
			helper = new DbHelper(context);
		}
		return helper;
	}

	private DbHelper(Context context) {
		super(context, DB_NAME, null, DB_VERSION);//创建数据库
	}
	
	@Override
	public void onCreate(SQLiteDatabase db) {
		Log.i(TAG, "onCreate");
		try{

			db.execSQL(getCreateTableSql(User.class));
			db.execSQL(getCreateTableSql(OperationLog.class));
			
			initUersTable(db);
		}catch(Exception e){
			e.printStackTrace();
		}
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		Log.i(TAG, "数据库升级 old = "+oldVersion+", new = "+newVersion);
	}

	/**
	 * 初始化操作员表中默认操作员
	 * @param db
	 */
	private void initUersTable(SQLiteDatabase db){
		db.execSQL("INSERT INTO T_USER(USER_NO,PASSWORD,USER_TYPE,ISFIRSTUSE) VALUES('00','00000000','1','1')");
		db.execSQL("INSERT INTO T_USER(USER_NO,PASSWORD,USER_TYPE,ISFIRSTUSE) VALUES('00','00000000','2','1')");
	}

	public static String getCreateTableSql(Class<?> clazz){
		Table table = clazz.getAnnotation(Table.class);
		String tableName = table.name();
		String idColumn = "";
		Field[] fields = clazz.getDeclaredFields();
		for (Field field : fields) {
			Column c = field.getAnnotation(Column.class);
			if (c != null && c.primaryKey()) {
				idColumn = c.name();
				break;
			}
		}
		
		StringBuffer sqlBuffer = new StringBuffer();
        sqlBuffer.append("CREATE TABLE IF NOT EXISTS ");
        sqlBuffer.append(tableName);
        sqlBuffer.append(" ( ");

        sqlBuffer.append("\"").append(idColumn).append("\"  ").append("INTEGER PRIMARY KEY AUTOINCREMENT,");

        fields = clazz.getDeclaredFields();
		for (Field field : fields) {
			Column c = field.getAnnotation(Column.class);
			if (c != null && !c.primaryKey()) {
				sqlBuffer.append("\"").append(c.name()).append("\"  ");
				
	            sqlBuffer.append(getDBColumnType(field));
	            
	            if (c.unique()) {
	                sqlBuffer.append(" UNIQUE");
	            }
	            sqlBuffer.append(",");
			}
		}
        sqlBuffer.deleteCharAt(sqlBuffer.length() - 1);
        sqlBuffer.append(" )");
        
		return sqlBuffer.toString();
	}
	
	private static String getDBColumnType(Field field) {
		Class<?> javaType = field.getType();
		if (javaType == Integer.class) {
			return "INTEGER";
		} else if (javaType == Long.class) {
			return "INTEGER";
		} else if (javaType == String.class) {
			return "VARCHAR(" + field.getAnnotation(Column.class).len() + ")";
		} else {
			return "VARCHAR(" + field.getAnnotation(Column.class).len() + ")";
		}
	}
}
