package com.example.listviewtest;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private String[] data={"Apple","Banana","Orange","Watermelon","Pear","Grape","Pineapple","Strawberry", "Cherry","Mango",
            "Apple","Banana","Orange","Watermelon","Pear","Grape","Pineapple","Strawberry", "Cherry","Mango"};
    private List<Fruit> fruitList=new ArrayList<>();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initFruits();//初始化水果数据
        FruitAdapter adapter=new FruitAdapter(MainActivity.this,R.layout.fruit_item,fruitList);
//        ArrayAdapter<String> adapter=new ArrayAdapter<String>(MainActivity.this, android.R.layout.simple_list_item_1,data);
        ListView listView=(ListView)findViewById(R.id.list_view);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Fruit fruit=fruitList.get(position);
                Toast.makeText(MainActivity.this,fruit.getName(),Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void initFruits(){
        for(int i=0;i<2;i++){
            Fruit apple=new Fruit("Apple",R.mipmap.apple_pic);
            fruitList.add(apple);
            Fruit banana=new Fruit("Banana",R.mipmap.banana_pic);
            fruitList.add(banana);
            Fruit orange=new Fruit("Orange",R.mipmap.orange_pic);
            fruitList.add(orange);
            Fruit watermelon=new Fruit("Watermelon",R.mipmap.watermelon_pic);
            fruitList.add(watermelon);
            Fruit pear=new Fruit("Pear",R.mipmap.pear_pic);
            fruitList.add(pear);
            Fruit grape=new Fruit("Grape",R.mipmap.grape_pic);
            fruitList.add(grape);
            Fruit pineapple=new Fruit("Pineapple",R.mipmap.pineapple_pic);
            fruitList.add(pineapple);
            Fruit strawberry=new Fruit("Strawberry",R.mipmap.strawberry_pic);
            fruitList.add(strawberry);
            Fruit cherry=new Fruit("Cherry",R.mipmap.cherry_pic);
            fruitList.add(cherry);
            Fruit mango=new Fruit("Mango",R.mipmap.mango_pic);
            fruitList.add(mango);
        }
    }

    public class Fruit{
        private String name;
        private int imageId;
        public Fruit(String name,int imageId){
            this.name=name;
            this.imageId=imageId;
        }

        public String getName(){
            return name;
        }

        public int getImageId(){
            return imageId;
        }
    }

    public class FruitAdapter extends ArrayAdapter<Fruit>{
        private int resoureId;
        public FruitAdapter(Context context, int textviewResourseId, List<Fruit> objects){
            super(context,textviewResourseId,objects);
            resoureId=textviewResourseId;
        }

        @Override
        public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
            Fruit fruit=getItem(position);//获取当前项的Fruit实例
//            View view= LayoutInflater.from(getContext()).inflate(resoureId,parent,false);
            View view;
//            if(convertView==null){
//                view=LayoutInflater.from(getContext()).inflate(resoureId,parent,false);
//            }
//            else {
//                view=convertView;
//            }
//            ImageView fruitImage=(ImageView)view.findViewById(R.id.fruit_image);
//            TextView fruitName=(TextView)view.findViewById(R.id.fruit_name);
//            fruitImage.setImageResource(fruit.getImageId());
//            fruitName.setText(fruit.getName());
            ViewHolder viewHolder;
            if(convertView==null){
                view=LayoutInflater.from(getContext()).inflate(resoureId,parent,false);
                viewHolder=new ViewHolder();
                viewHolder.fruitImage=(ImageView)view.findViewById(R.id.fruit_image);
                viewHolder.fruitName=(TextView)view.findViewById(R.id.fruit_name);
                view.setTag(viewHolder);//将ViewHolder存储在View中
            }
            else{
                view=convertView;
                viewHolder=(ViewHolder)view.getTag();//重新获取ViewHolder
            }
            viewHolder.fruitImage.setImageResource(fruit.getImageId());
            viewHolder.fruitName.setText(fruit.getName());
            return view;
        }

        class ViewHolder{
            ImageView fruitImage;
            TextView fruitName;
        }
    }


}

