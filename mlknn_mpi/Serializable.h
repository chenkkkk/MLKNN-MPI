//
// Created by 75961 on 2016/9/20.
//

#ifndef MLKNN_MPI_SERIALIZABLE_H
#define MLKNN_MPI_SERIALIZABLE_H
class Serializable
{
public:

    Serializable(void)
    {
    }

public:

    ~Serializable(void)
    {
    }

public:

    virtual void Serialize(unsigned char* _out, int& length) = 0;

    virtual void Deserialize(unsigned char* _in, int& length) = 0;

    virtual int Length() = 0;

};
#endif //MLKNN_MPI_SERIALIZABLE_H
