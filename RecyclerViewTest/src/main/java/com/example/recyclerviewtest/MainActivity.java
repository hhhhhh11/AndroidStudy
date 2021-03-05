package com.example.recyclerviewtest;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.StaggeredGridLayoutManager;

import com.example.recyclerviewtest.FruitAdapter;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class MainActivity extends AppCompatActivity {
    private List<FruitAdapter.Fruit> fruitList=new ArrayList<>();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initFruits();
        RecyclerView recyclerView=(RecyclerView)findViewById(R.id.recycler_view);
//        LinearLayoutManager layoutManager=new LinearLayoutManager(this);
//        layoutManager.setOrientation(LinearLayoutManager.HORIZONTAL);
        StaggeredGridLayoutManager layoutManager=new StaggeredGridLayoutManager(3,StaggeredGridLayoutManager.VERTICAL);
        recyclerView.setLayoutManager(layoutManager);
        FruitAdapter adapter=new FruitAdapter(fruitList);
        recyclerView.setAdapter(adapter);
    }

    private void initFruits(){
        for(int i=0;i<2;i++){
            FruitAdapter.Fruit apple=new FruitAdapter.Fruit(getRandomLengthName("Apple"),R.mipmap.apple_pic);
            fruitList.add(apple);
            FruitAdapter.Fruit banana=new FruitAdapter.Fruit(getRandomLengthName("Banana"),R.mipmap.banana_pic);
            fruitList.add(banana);
            FruitAdapter.Fruit orange=new FruitAdapter.Fruit(getRandomLengthName("Orange"),R.mipmap.orange_pic);
            fruitList.add(orange);
            FruitAdapter.Fruit watermelon=new FruitAdapter.Fruit(getRandomLengthName("Watermelon"),R.mipmap.watermelon_pic);
            fruitList.add(watermelon);
            FruitAdapter.Fruit pear=new FruitAdapter.Fruit(getRandomLengthName("Pear"),R.mipmap.pear_pic);
            fruitList.add(pear);
            FruitAdapter.Fruit grape=new FruitAdapter.Fruit(getRandomLengthName("Grape"),R.mipmap.grape_pic);
            fruitList.add(grape);
            FruitAdapter.Fruit pineapple=new FruitAdapter.Fruit(getRandomLengthName("Pineapple"),R.mipmap.pineapple_pic);
            fruitList.add(pineapple);
            FruitAdapter.Fruit strawberry=new FruitAdapter.Fruit(getRandomLengthName("Strawberry"),R.mipmap.strawberry_pic);
            fruitList.add(strawberry);
            FruitAdapter.Fruit cherry=new FruitAdapter.Fruit(getRandomLengthName("Cherry"),R.mipmap.cherry_pic);
            fruitList.add(cherry);
            FruitAdapter.Fruit mango=new FruitAdapter.Fruit(getRandomLengthName("Mango"),R.mipmap.mango_pic);
            fruitList.add(mango);
        }
    }

    private String getRandomLengthName(String name){
        Random random=new Random();
        int length=random.nextInt(20)+1;
        StringBuilder builder=new StringBuilder();
        for (int i=0;i<length;i++){
            builder.append(name);
        }
        return builder.toString();
    }

}
