//
// Created by 75961 on 2016/9/20.
//

#ifndef MLKNN_MPI_SIMILAR_H
#define MLKNN_MPI_SIMILAR_H


#include "DataStruct.h"

class Similar
{
public:

    Similar(void)
    {
    }

public:

    ~Similar(void)
    {
    }

public:
    template<typename T>
    T sqr(T _v){
        return _v * _v;
    }

    virtual double sim(vector<double>& _v1, vector<double>& _v2)
    {
        //不排序，默认下标有序
       /* int i1 = 0, i2 = 0;
        double ret = 0;
        for (;i1 < _v1.size() || i2 < _v2.size();)
        {
            if (i1 < _v1.size() && i2 < _v2.size())
            {
                if (_v1[i1].dim == _v2[i2].dim)
                {
                    ret += sqr(_v1[i1++].value - _v2[i2++].value);
                } else if (_v1[i1].dim < _v2[i2].dim)
                {
                    ret += sqr(_v1[i1++].value);
                }  else
                {
                    ret += sqr(_v2[i2++].value);
                }
            } else if (i1 < _v1.size())
            {
                ret += sqr(_v1[i1++].value);
            } else
            {
                ret += sqr(_v2[i2++].value);
            }
        }*/
        double ret=0;
        for (int i=0; i<_v1.size(); ++i){
            ret+=sqr(_v1[i]-_v2[i]);
        }
        return -ret;
    };

    virtual double sim(OneSample& _d1, OneSample& _d2)
    {
        return sim(_d1.vec_x, _d2.vec_x);
    };

};


#endif //MLKNN_MPI_SIMILAR_H
