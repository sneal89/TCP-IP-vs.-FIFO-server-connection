#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "NRC.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <mutex>
#include <stdio.h>
#include "wait.h"
using namespace std;

void * patient_function(int person, int n, double ecg, BoundedBuffer* buf, int check, mutex* mut)
{
    /* What will the patient threads do? */
    
    //cout << "thread " << check << endl;
    //double count = 0;
    char* cptr;
    
    double time = 0.0;
    for (int i = 0; i < n; i++)
    {
        datamsg data = datamsg(person, time, ecg);
        datamsg* dptr = &data;
        cptr = (char*)dptr;
        vector<char> dataVec(cptr, cptr + sizeof(datamsg)/*length*/);
        buf->push(dataVec);
        time += .004;
        //buf->offset = buf->offset + 0.004;
    }
    //cout << check << " <--- pat thread finished" << endl;
    //MESSAGE_TYPE quit = QUIT_MSG;
}
/*request buffer and particular channel to cwrite to*/
void *worker_function(BoundedBuffer* buf, NRC* chan, string fileToGet, HistogramCollection* collect /*pointer to histcollection*/)
{
    //ofstream retrieved;
     
    //retrieved.open("/home/sneal/PA2/received/" + fileToGet, ios::out | ios::binary);
    /*
    pop request off buffer
        Functionality of the worker threads 
        500 worker threads
    */
    //mutex mu;
    fileToGet = fileToGet;

     string strFile = "received/" + fileToGet;
        const char* stringFile = strFile.c_str();
        
    while(true)
    {
        //retrieved.open("/home/sneal/PA3/received/" + fileToGet, ios::out | ios::binary);
       


        vector<char> temp = buf->pop();
        char* data_message_char_ptr = reinterpret_cast<char*>(temp.data());
        
        if(*data_message_char_ptr == QUIT_MSG)
        {
            chan->cwrite (data_message_char_ptr, sizeof (MESSAGE_TYPE));
            delete chan;
            break;
        }
        else if(*data_message_char_ptr == FILE_MSG)
        {
            int fd = open(stringFile, O_RDWR | O_CREAT);
            int64_t offset2 = (int64_t)((filemsg*)temp.data())->offset;
            int lengthy = ((filemsg*)temp.data())->length;

            int c = lseek(fd ,offset2 ,SEEK_SET);
            //cout << c << " <-- this is c for lseek yay" << endl;
            filemsg* file_message_ptr = (filemsg*)data_message_char_ptr;
            filemsg file_message = *file_message_ptr;
            chan->cwrite (temp.data(), sizeof(filemsg) + sizeof(fileToGet));


            //char* output = (char*)chan->cread();
            write(fd, chan->cread(),lengthy);
            close(fd);

            //fwrite(output,1,temp.size(),(FILE*)fd);
            //cout << * (char*)output << endl;

        }
        else if(*data_message_char_ptr == DATA_MSG)
        {
        
            datamsg* data_message_ptr = (datamsg*)data_message_char_ptr;
            datamsg data_message = *data_message_ptr;
            //mu.lock();
            chan->cwrite(data_message_char_ptr,sizeof(datamsg));
            double* output = (double*) chan->cread();
            //get output to the histogram
            //put lock around adding values to the histogram
            //Histogram.lockheed_mutex.lock();
            //cout << *output << endl;
            //cout << data_message.person << "  <--LOOK HERE" <<  endl;
            collect->updateCollective(data_message_ptr->person , *output);
            
            //cout << help << "   ";
            //mu.unlock();
        }  
    }
    //retrieved.close();
    /* while true loop to check for a quit message
    histogram work should be done in the worker function
    histograms should be thread safe and use mutexes*/
}

void *file_function(BoundedBuffer* buf, NRC* chan, int b, string fileNameInput)
{
    //recieving seg fault 
    //copied file msg structure from PA2
    //only edited the cwrites to be pushed on the buffer instead and added the buffer pointer to msg size
    //cout << "First circle of hell" << endl;
    char*buffer;
    ofstream retrieved;
    string fname = fileNameInput;

    //int f_write = open("1.csv", O_RDONLY);

    // Open the file for WRITE and READ only. 
    //int f_read = open("1_recieved.csv", O_WRONLY);
    filemsg def = filemsg(0, 0);
    buffer = new char[sizeof(filemsg) + fname.length() + 1];
    memcpy(&buffer[0], &def, sizeof(filemsg));
    const char* c1 = fname.c_str();

    for (int i = 0; i < fname.length(); i++)
    {
        *(char*)(buffer + sizeof(def) + i) = fname.at(i);


    }

    //cout << "AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
    //vector<char> temp(buffer, buffer + sizeof(filemsg) + sizeof(fname)); //modified line
    //cout << "BHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
    //buf->push(temp);

    chan->cwrite(buffer,sizeof(filemsg) + sizeof(fname));
   
    int* fileSize = (int*)chan->cread(); //oof ouch it hurts
   
    //cout << "fileSize: " << *fileSize << endl; //we get here although i think the file size printed is wrong
   
    int len = MAX_MESSAGE;
    
    int offset = 0;
    int fsize = *fileSize;
   
    //out.open("/home/alriclobo/313/received/" + fname, ios::out | ios::binary);
    //gettimeofday(&start, NULL);
    //cout << "do we get before the four loop haha get it like the number idiot" << endl;
    while (len > 0)
    {
        //cout<<"THIS IS THE LENGTH!!!!!"<<len<<endl;
        
        if (fsize < MAX_MESSAGE)
        {
            //cout<<"OPTION 1"<<endl;
            //cout<<"THIS IS THE fsize!!!! option 1 "<<fsize<<endl;
            len = fsize;
            def = filemsg(offset, len);
            memcpy(&buffer[0], &def, sizeof(filemsg));
            vector<char> temp(buffer, buffer + sizeof(filemsg) + sizeof(fname));
            buf->push(temp);
            //out.write(chan.cread(), len);
            len = 0;
            //cout<<"THIS IS THE LENGTH!!!!! option 1 "<<len<<endl;
              //cout<<"THIS IS THE OFFSET!!!!! option 1 "<<offset<<endl;

        }
        else if (fsize == MAX_MESSAGE)
        {
            //cout<<"OPTION 2"<<endl;
            def = filemsg(offset, len);
            memcpy(&buffer[0], &def, sizeof(filemsg));
            vector<char> temp(buffer, buffer + sizeof(filemsg) + sizeof(fname));
            buf->push(temp);
            fsize = fsize - len;
            offset += len;
            break;
        }
        else
        {
            //cout<<"OPTION 3"<<endl;
           // cout<<"THIS IS THE LENGTH 0!!!!!"<<len<<endl;
             // cout<<"THIS IS THE OFFSET 0!!!!!"<<offset<<endl;
            def = filemsg(offset, len);
            memcpy(&buffer[0], &def, sizeof(filemsg));
            vector<char> temp(buffer, buffer + sizeof(filemsg) + sizeof(fname));
            buf->push(temp);
            //cout<<*temp.data()<<endl;
            fsize = fsize - len;
          // cout<<"THIS IS THE LENGTH!!!!! 1"<<len<<endl;
             // cout<<"THIS IS THE OFFSET!!!!! 1"<<offset<<endl;
            offset += len;

             //cout<<"THIS IS THE LENGTH!!!!! option 3 "<<len<<endl;
              //cout<<"THIS IS THE OFFSET!!!!! option 3 "<<offset<<endl;
        }
    }  
}

int main(int argc, char *argv[])
{
    int numOfForks = 1;
    cout << "How many clients to run?" << endl;
    cin >> numOfForks;
    
    struct timeval start, end;
    gettimeofday (&start, 0);
    
    for(int vv = 0; vv < numOfForks; vv++)
    {
        pid_t pidding = fork();
        if(pidding == 0)
        {
        int n = 100;            //default number of requests per "patient"
        int p = 15;             // number of patients [1,15]
        int w = 100;            //default number of worker threads
        int b = /*20*/ 20 /*1000*/;     // default capacity of the request buffer, you should change this default
        int m = MAX_MESSAGE;    // default capacity of the file buffer
        srand(time_t(NULL));
        bool fileTime = false;
    //ofstream retrieved;
        string fileToGet = " ";
        string port = "";
        string name = "";
        
        int c;
        while ((c = getopt (argc, argv, "n:p:w:b:f:h:r:")) != -1){
            switch (c)
            {
            case 'n':
                n = atoi(optarg);
                break;
            case 'p':
                p = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'f':
                fileToGet = optarg;
                fileTime = true;
            //what file
                break;
            case 'h':
                name = optarg;
                break;
            case 'r':
                port = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
                    return 1;
            default:
                abort ();
            }
        }
    
    //cout << n << " num requests per patient " << endl;
    //cout << p << " number of patients " << endl;
    //cout << w << " num of worker threads " << endl;
    //cout << b << " buffer cap " << endl;
    //cout << fileToGet << "   <--- file name to get " << endl;

    

    /*
    int pid = fork();
    if (pid == 0){
        // modify this to pass along m
        //execl ("dataserver", "dataserver", (char *)NULL);
        
    }
    */
    //NRC* chan = new NRC("control", NRC::CLIENT_SIDE);
        BoundedBuffer request_buffer(b);
        HistogramCollection hc;
    
        NRC * NRCquit = new NRC(name, port);
    
   

    /* Start all threads here */
    //patient_function(1, 10, 1.0, &request_buffer);
    //thread t[10];
    //mutex* mu;
        vector<thread*> threadVec;
        vector<thread*> threadWork;
        vector<NRC*> NRC_Fun;
        mutex mu;

        MESSAGE_TYPE q = QUIT_MSG;
        MESSAGE_TYPE qq = NEWCHANNEL_MSG;
    
    
        for(int y = 0; y < w; y++)
        {
        //chan->cwrite((char*)&qq, sizeof(MESSAGE_TYPE));
        //name = (char*) chan->cread();
            NRC_Fun.push_back(new NRC(name, port));
        }

    //cout << "ttttttttttttt" << endl;
        for (int j = 0; j < p; j++)
        {
            hc.add(new Histogram(10, -2.0, 2.0));
            threadVec.push_back(move(new thread(patient_function, j+1, n, 1.0, &request_buffer, j, &mu)));
        }
    
        vector<thread*> threadFile;/////////////////////////////////////////////////////////////////////////
        if(fileTime)
        {
        //threadFile.push_back(move(new thread(file_function, &request_buffer, new NRC(name, port), b, fileToGet)));////////////////////
            threadFile.push_back(move(new thread(file_function, &request_buffer, NRCquit, b, fileToGet)));////////////////////
        }
    
    //cout << "rrrrrrrrrr" << endl;
        for(int z = 0; z < w; z++)
        {
            threadWork.push_back(move(new thread(worker_function, &request_buffer, NRC_Fun[z], fileToGet, &hc )));
        }

    //cout<<threadVec.size()<<endl;
    //cout << "eeeeeeeee" << endl;

    /* Join all threads here */
    
        for (int k = 0; k < p; k++)
        {
        //cout << k << endl;
            threadVec[k]->join();
        }
    
    //cout << "wwwwwwwwwwwwwww" << endl;
        if(fileTime)
        {
            threadFile[0]->join(); /////////////////////////////////////////////////////////////////////////////
        }
    
     //push quit messages
        char* quitptr;
        for (int x = 0; x < w; x++)
        {
            MESSAGE_TYPE* mptr = &q;
            quitptr = (char*)mptr;
            vector<char> quitVec(quitptr, quitptr + sizeof(MESSAGE_TYPE));
            request_buffer.push(quitVec);
        }

        //cout << "qqqqqqqqq" << endl;

        for (int f = 0; f < w; f++)
        {
        //cout << f << endl;
            threadWork[f]->join();
        }
    
        for(int h = 0; h < p; h++)
        {

        }
    
        //gettimeofday (&end, 0);
        hc.print ();
        //int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
        //int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
        //cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;


    
        NRCquit->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
        //cout << "All Done!!!" << endl;
        delete NRCquit;

        hc.cleanHist(); 

        }
        else
        {
            wait(0);
        }



    

    }

    gettimeofday (&end, 0);
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
}
    

    




//ulimit -Sn
//ulimit -n 2500
//sudo ./client -n 15000 -p 2 -w 500 -b 50