//
// Created by 75961 on 2016/9/20.
//

#ifndef MLKNN_MPI_MLKNN_H
#define MLKNN_MPI_MLKNN_H


#include <fstream>
#include <iostream>
#include <mpich-x86_64/mpi.h>
#include "Similar.h"
#include "DataStruct.h"
#include <vector>
#include <string>
extern string TrainFile;
extern string TestFile;
//extern const int label_num;
extern int smooth;
class MLKNN {
public:
    Similar *p_Sim;
    int nnk;
    vector<OneSample> TrainData;
    vector<priorityQueue> TestQueue;
    vector<priorityQueue> TrainTestQueue;
    vector<int> prior_probability;//存放先验概率
    vector<vector<char> >test_label;


    vector<C> every_C;//所有标签的~all_14
    vector<P> all_p;//P--only 0 进程
    vector<vector<char > > SumEvery;

    //int *c,*c1;
    MLKNN(int _k) {
        nnk = _k;
        TrainData.clear();
        TestQueue.clear();
        TrainTestQueue.clear();
        p_Sim = new Similar();
        //c=new int[nnk+1];
        //c1=new int[nnk+1];
    }

    ~MLKNN() {
        delete p_Sim;
        //delete []c;
        //delete []c1;
    }

    void LoadTrainData(string _fileName) {
        ifstream fin(_fileName.c_str());
        string tmp;
        while (getline(fin, tmp)) {
            OneSample doc;
            doc.parse(tmp);

            TrainData.push_back(doc);
        }
        fin.close();
    }


    void start() {
        get_prior_probalitiry();
        Test(TrainFile, TrainTestQueue, nnk);
        Get_tran_knn(TrainTestQueue);
        CastGabolKnn(TrainTestQueue);
        //MPI_Barrier(MPI_COMM_WORLD);
        //
        //MPI_Abort(MPI_COMM_WORLD,10);
        SumEverySample();
        //
    }

    void SumEverySample() {

        int myRank;
        int num_proc;
        MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
        char temp_a[label_num];
        //vector<int> temp;

        for (int i = myRank; i < TrainTestQueue.size(); i += num_proc) {

            memset(temp_a, 0, sizeof(temp_a));
            for (int j = 0; j < TrainTestQueue[i].curSize; ++j) {
                for (int k = 0; k < TrainTestQueue[i].que[j].label.size(); ++k) {
                    temp_a[k] += TrainTestQueue[i].que[j].label[k];
                }
            }
            vector<char > temp(temp_a, temp_a + label_num);
            SumEvery.push_back(temp);
        }

        cout << "id =" << myRank << " ";
        cout << "sumEvery   size  and size ==" << SumEvery.size() << "  " << SumEvery[0].size() << endl << flush;


        for (int i = 0; i < label_num; ++i) {
            C tmp(nnk + 1);
            for (int j = 0; j < SumEvery.size(); ++j) {//375
                if (TrainData[j].vec_y[i] == 1) {
                    tmp.c[SumEvery[j][i]]++;
                } else {
                    tmp.c1[SumEvery[j][i]]++;
                }
            }
            every_C.push_back(tmp);//c数组应该要全局统计一次,规约到0进程
        }
        int *de = new int[nnk + 1];
        int *de_de = new int[nnk + 1];
        for (int i = 0; i < every_C.size(); ++i) {//label_num
            MPI_Reduce(&every_C[i].c[0], de, nnk + 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&every_C[i].c1[0], de_de, nnk + 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            if (myRank == 0) {
                for (int j = 0; j < every_C[i].c.size(); ++j) {
                    every_C[i].c[j] = de[j];
                    every_C[i].c1[j] = de_de[j];
                }
            }

        }
        delete[]de;
        delete[]de_de;

        if (myRank == 0) {
            for (int i = 0; i < every_C.size(); ++i) {

                P P_temp;
                int p_c = sum(every_C[i].c);
                int p_c1 = sum(every_C[i].c1);
                for (int j = 0; j < every_C[i].c.size(); ++j) {
                    P_temp.p.push_back((double) (smooth + every_C[i].c[j]) / (smooth * (nnk + 1) + p_c));
                    P_temp.p1.push_back((double) (smooth + every_C[i].c1[j]) / (smooth * (nnk + 1) + p_c1));
                }
                all_p.push_back(P_temp);
            }
        }


    }

    int sum(vector<int> a) {
        int re = 0;
        for (int i = 0; i < a.size(); ++i) {
            re += a[i];
        }
        return re;
    }


    priorityQueue Test(OneSample doc, int num_k) {
        priorityQueue ret(num_k);
        for (int i = 0; i < TrainData.size(); ++i) {
            SimilarAndLabel nn;
            nn.similar = p_Sim->sim(doc, TrainData[i]);
            if (nn.similar == 0)
                continue;
            nn.label = TrainData[i].vec_y;
            ret.Push(nn);
        }
        return ret;
    }

    void Test(string _testFile, vector<priorityQueue> &T, int num_k) {//获得邻近的样本
        ifstream fin(_testFile.c_str());
        string tmp;
        //vector<priorityQueue> ret;
        while (getline(fin, tmp)) {
            OneSample doc;
            doc.parse(tmp);
            priorityQueue tmpq = Test(doc, num_k);
            T.push_back(tmpq);
        }
        fin.close();
//        return ret;
    }

    void Test(string _testFile, int num_k) {//获得邻近的样本
        ifstream fin(_testFile.c_str());
        string tmp;
        int myRank;
        MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
        //vector<priorityQueue> ret;
        while (getline(fin, tmp)) {
            OneSample doc;
            doc.parse(tmp);
            if (myRank==0) test_label.push_back(doc.vec_y);
            priorityQueue tmpq = Test(doc, num_k);

            TestQueue.push_back(tmpq);
        }
        fin.close();
//        return ret;
    }

    void CastGabolKnn(vector<priorityQueue> &T) {
        int myRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
        int totalLen = 0;
        for (int i = 0; i < T.size(); ++i) {
            totalLen += T[i].Length();
        }
        //cout<<"TrainTestQueue  size and  totalLen==="<<T.size()<<"   "<<totalLen<<endl<<flush;
        unsigned char *buffer = new unsigned char[totalLen];
        if (myRank == 0) {
            int offset = 0;
            for (int i = 0; i < T.size(); ++i) {
                int vlen;
                T[i].Serialize((buffer + offset), vlen);
                offset += vlen;
            }
        }

        MPI_Bcast(buffer, totalLen, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
        if (myRank >= 1) {
            int offset = 0;
            //cout<<"BCast  to id=="<<myRank<<endl<<flush;
            for (int i = 0; i < T.size(); ++i)//TestQueue存储测试样本信息
            {
                priorityQueue nn(nnk);
                int vlen;
                nn.Deserialize((buffer + offset), vlen);
                //T[i].merge(nn);
                T[i] = nn;
                offset += vlen;
            }
//            for (int r=0; r<T[0].curSize; ++r)
//                cout<<"myid="<<myRank<<" T[0].que["<<r<<"].label="<<T[0].que[r].label[0]<<endl<<flush;
        }

        delete[] buffer;
    }

    void Get_tran_knn(vector<priorityQueue> &T) {//获得距离测试样本最近的K个训练样本。如同二分类KNN---T为记录样本的k个近邻的信息
        int myRank;//蒋最后的结果合并到0进程
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

        if (myRank != 0) {
            //非0进程发送结果
            int totalLen = 0;

            //cout<<"   T[i]..que.really  size=="<<T[0].que.size()<<"   T[i]..que.size=="<<T[0].size<<"  T[i]..que..cursize=="<<T[5].curSize<<endl<<flush;}
            for (int i = 0; i < T.size(); ++i) {
                totalLen += T[i].Length();
            }
            unsigned char *buffer = new unsigned char[totalLen];
            int offset = 0;
            for (int i = 0; i < T.size(); ++i) {
                int vlen;
                T[i].Serialize((buffer + offset), vlen);
                offset += vlen;
            }
            //cout << myRank << "  send T.size  "<<T.size()<< endl<<flush;
            MPI_Send(&totalLen, 1, MPI_INT, 0, myRank * 10 + 0, MPI_COMM_WORLD);
            cout << myRank << "  send buffer totallen==" << totalLen << endl;
            MPI_Send(buffer, totalLen, MPI_UNSIGNED_CHAR, 0, myRank * 10 + 1, MPI_COMM_WORLD);
            cout << myRank << "  done" << endl;
            delete[] buffer;
            } else {
            int totalProc = MPI::COMM_WORLD.Get_size();
            cout << myRank << " totalProc = " << totalProc << endl;
            for (int rnk = 1; rnk < totalProc; ++rnk) {
                int len;
                MPI_Status status;
                cout << myRank << " recv len form " << rnk << endl;
                MPI_Recv(&len, 1, MPI_INT, rnk, rnk * 10 + 0, MPI_COMM_WORLD, &status);
                cout << myRank << " len[" << rnk << "] = " << len << "   ";
                cout << myRank << " recv buffer form " << rnk << endl;
                unsigned char *buffer = new unsigned char[len];
                MPI_Recv(buffer, len, MPI_UNSIGNED_CHAR, rnk, rnk * 10 + 1, MPI_COMM_WORLD, &status);
                cout << myRank << " recv done " << rnk << endl;
                int offset = 0;
                for (int i = 0; i < T.size(); ++i)//TestQueue存储测试样本信息
                {
                    priorityQueue nn(nnk);
                    int vlen;
                    nn.Deserialize((buffer + offset), vlen);
                    T[i].merge(nn);
                    offset += vlen;
                }
                delete[] buffer;
            }
        }

    }
    void get_prior_probalitiry(){
        int myRank;
        MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
        int count[label_num];
        memset(count, 0, sizeof(count));
        for (int i = 0; i < TrainData.size(); ++i) {
            for (int j = 0; j < TrainData[i].vec_y.size(); ++j) {
                if (TrainData[i].vec_y[j] == 1)
                    count[j]++;
            }
        }
        int receive[label_num];

        memset(receive, 0, sizeof(receive));
//        MPI_Reduce_scatter(count,receive,&long_array,MPI_INT,)
        MPI_Allreduce(count, receive, label_num, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        for (int i = 0; i < label_num; ++i) {
            prior_probability.push_back(receive[i]);
        }
        //cout<<"prior_probablity  len  ="<<prior_probability.size()<<endl;
        /*if (myRank == 1)
            for (int i = 0; i < prior_probability.size(); ++i) {
                cout << prior_probability[i] << endl << flush;
            }*/
    }
};



#endif //MLKNN_MPI_MLKNN_H
