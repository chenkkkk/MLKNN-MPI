//
// Created by 75961 on 2016/9/20.
//

#ifndef MLKNN_MPI_DATASTRUCT_H
#define MLKNN_MPI_DATASTRUCT_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include "Serializable.h"

using namespace std;
extern const int label_num;
class OneSample{
public:
    vector<double> vec_x;
    vector<char> vec_y;
    friend ostream& operator<<(ostream &out,const OneSample&a){
        out<<"label: ";
        for (int i=0; i<a.vec_y.size(); ++i){
            out<<a.vec_y[i]<<" ";
        }
        return out;
    }
    void parse(string _str){
        stringstream ss(_str);
        double x;
        char y;
        int flag=0;
        string T;
        //vec_y.clear();
        //vec_x.clear();
        while(getline(ss,T,',')){//input vec_x
            ++flag;
            x=atof(T.c_str());
            vec_x.push_back(x);
            if (flag==120)
                break;
        }
        while(getline(ss,T,',')){//intput vec_y
            //++flag;
            y=atoi(T.c_str());
            vec_y.push_back(y);
        }

    }
};

class SimilarAndLabel: public Serializable//用于存储相似度和标签
{
public:
    double similar;
    vector<char > label;
    SimilarAndLabel(){
        label.resize(label_num);//标签的个数
        similar=-9999999999999;
    }
    bool operator < (const SimilarAndLabel& _n) const
    {
        return similar < _n.similar;
    }

    int Length()
    {
//        return sizeof(double) * 2;
        return sizeof(double) + sizeof(char)*label.size();
    }

    void Serialize(unsigned char* _out, int& _len)
    {
        _len = Length();
//        memcpy(_out, &label, sizeof(double));
        for (int i=0; i<label.size(); ++i)
            memcpy(_out+i* sizeof(char), &label[i], sizeof(char));
//        memcpy(_out + sizeof(double), &similar, sizeof(double));
        memcpy(_out + label.size()* sizeof(char), &similar, sizeof(double));
    }

    void Deserialize(unsigned char* _in, int& _len)
    {
        _len = Length();
//        memcpy(&label, _in, sizeof(double));
        for (int i=0; i<label.size(); ++i)
            memcpy(&label[i], _in+i* sizeof(char), sizeof(char));
//        memcpy(&similar, _in + sizeof(double), sizeof(double));
        memcpy(&similar, _in + label.size()* sizeof(char), sizeof(double));
    }
};

struct priorityQueue : public Serializable
{
    char size;//优先队列大小。。使用char
    char curSize;
    vector<SimilarAndLabel> que;

    priorityQueue(char _size)//优先队列的大小--  size  k 相等
    {
        size = _size;
        curSize = 0;
        que.resize(size);
    }

    void Push(SimilarAndLabel _val)
    {
        int i;
        for (i = 0; i < curSize; ++i)
        {
            if (que[i] < _val)
            {
                break;
            }
        }
        if (i >= size)
        {
            return;
        }
        for (int j = curSize; j >= i; --j)
        {
            if (j + 1 >= size)
            {
                continue;
            }
            que[j + 1] = que[j];
        }
        que[i] = _val;
        if (curSize < size)
        {
            curSize++;
        }
    }
   /* void delete_first(){
        que.erase(que.begin());
        if (curSize==size){
            --curSize;
            --size;
        }
        else{
            --curSize;
        }
    }*/
    SimilarAndLabel& operator[](int _id)
    {
        return que[_id];
    }

    void merge(priorityQueue& _o)
    {
        for (int i = 0; i < _o.size; ++i)
        {
            Push(_o[i]);
        }
    }

    int Length()
    {
        int ret = 0;
        ret += sizeof(char); //size
        ret += sizeof(char); //curSize
        for (int i = 0; i < curSize; ++i)//size
        {
            ret += ((SimilarAndLabel)que[i]).Length();
        }
        return ret;
    }

    void Serialize(unsigned char* _out, int& length)
    {
        int offset = 0;
        memcpy(_out + offset, &size, sizeof(char));
        offset += sizeof(char);
        memcpy(_out + offset, &curSize, sizeof(char));
        offset += sizeof(char);
        for (int i = 0; i < curSize; ++i)//size
        {
            int vlen;
            ((SimilarAndLabel)que[i]).Serialize(_out + offset, vlen);
            offset += vlen;
        }
        length = offset;
    }

    void Deserialize(unsigned char* _in, int& length)
    {
        int offset = 0;
        memcpy(&size, _in + offset, sizeof(char));
        offset += sizeof(char);
        memcpy(&curSize, _in + offset, sizeof(char));
        offset += sizeof(char);
        que.clear();
        for (int i = 0; i < curSize; ++i)
        {
            int vlen;
            SimilarAndLabel tpy;
            tpy.Deserialize(_in + offset, vlen);
            offset += vlen;
            que.push_back(tpy);
        }
        length = offset;
    }
};



struct C{
    vector<int> c;
    vector<int> c1;
    C(int size){
        c.resize(size);
        c1.resize(size);
        for (int i=0; i<size; ++i){
            c[i]=0;
            c1[i]=0;
        }
    }
};//每一个标签的~

struct P{
    vector<double> p;
    vector<double> p1;
    /*P(int size){
        p.resize(size);
        p1.resize(size);
        for (int i=0; i<size; ++i){
            p[i]=0;
            p1[i]=0;
        }
    }*/
};//每一个标签的~





#endif //MLKNN_MPI_DATASTRUCT_H
