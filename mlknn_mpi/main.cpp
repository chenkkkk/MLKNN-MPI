#include <iostream>
#include "MLKNN.h"
#include <mpi.h>
using namespace std;
string TrainFile;
int smooth;
const int  label_num=101;
string TestFile;
int k;
int totalsamplenumber;
int main(int argc,char *argv[]) {
    int myRank;
    k=10;
    //label_num=101;
    smooth=1 ;
    totalsamplenumber=30993;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
    //TestFile="C:\\Users\\75961\\Desktop\\yeast\\yeast-test-mpi.arff";/////////////////////////////////////
    TestFile="/home/chen/桌面/mediamill/mediamill-test-mpi.arff";
    //TestFile="/home/chen/桌面/yeast/yeast-test-mpi.arff";
    //TrainFile="C:\\Users\\75961\\Desktop\\yeast\\yeast-train-mpi.arff";///////////////////////////////////
    TrainFile="/home/chen/桌面/mediamill/mediamill-train-mpi.arff";
//    TrainFile="/home/chen/桌面/yeast/yeast-train-mpi.arff";
    MLKNN knn(k);
    stringstream trainFile;
    trainFile<<TrainFile<<"_"<<myRank;
    knn.LoadTrainData(trainFile.str());


    knn.start();
    //cout<<"first stage end"<<endl<<flush;
    //MPI_Barrier(MPI_COMM_WORLD);
    knn.Test(TestFile,k);
    knn.Get_tran_knn(knn.TestQueue);

    if (myRank==0){
        int hloss=0;
        int temp[label_num];
        for (int i=0; i<knn.TestQueue.size(); ++i){//
            memset(temp,0, sizeof(temp));
            for (int j=0; j<knn.TestQueue[i].curSize; ++j){
                for (int k=0; k<knn.TestQueue[i].que[j].label.size(); ++k){
                    temp[k]+=knn.TestQueue[i].que[j].label[k];
                }
            }
            //cout<<"num "<<i<<" :";
            for (int t=0; t<label_num; ++t){
                double ee=(double)(smooth+ knn.prior_probability[t])/(smooth*2+totalsamplenumber);
                //cout<<"p--temp["<<t<<"]= "<<knn.all_p[t].p[temp[t]]<<" ";
                double b1= ee*knn.all_p[t].p[temp[t]];//
                //double b0=(totalsamplenumber-knn.prior_probability[t])*knn.all_p[t].p1[temp[t]];
                double b0=(1.0-ee)*knn.all_p[t].p1[temp[t]];//
                //cout<<"b1="<<b1<<" b0="<<b0<<" ";
                if (b1>b0){
                    //cout<<"1 ";
                    if (knn.test_label[i][t]==0)hloss++;
                } else {
                    //cout << "0 ";
                    if (knn.test_label[i][t]==1)hloss++;
                }
            }
            //cout<<endl;

        }
        cout<<"hloss"<<(double)hloss/(label_num*knn.TestQueue.size());
    }

    MPI_Finalize();
    return 0;
}