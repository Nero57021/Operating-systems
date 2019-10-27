#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>


using namespace std;

static pthread_mutex_t g_lock=PTHREAD_MUTEX_INITIALIZER;  //group lock
static pthread_mutex_t p_lock[10];                        //position lock
static pthread_cond_t g_wait = PTHREAD_COND_INITIALIZER;  //group wait
static pthread_cond_t p_wait[10];                         //position wait
static bool database[10] = {false};                       //"database" with 10 available positions
static int db_used_by[10];                                // store the user id currently accesing the db
int sgroup;                                               //starting group
int gcount[2]={0,0};                                      //groups count   
int gcount_copy[2]={0,0};                                 // copy of group count
pthread_t tid[10];                                        //threads ids
bool file_reading=false;                                  //boolean for file reading true when input reading is done
int gwcount=0;                                            // count of users that waited due to their group
int pwcount=0;                                            //count of users that waited due to position lock


struct sector                                             //structre to be passed to thread
{
	int group;
	int pos;
	int sTime;
	int wTime;
	int id;

	sector(int ngroup,int  npos, int STime,int WTime, int nid)
	{
		group = ngroup;
		pos = npos;
		sTime = STime;
		wTime = WTime;
		id = nid;
	}
};

void *access_database(void *arg)
{
   sector c=*(sector *)arg;                                     //typecast void ptr to sector type
   cout<<"user"<<c.id <<" from group "<<c.group<<" arives to the DBMS"<<endl;

   pthread_mutex_lock(&g_lock);                                 //acquire group lock
   if(c.group!=sgroup){
   gwcount++;                                
   cout<<"user"<<c.id <<" is waiting due to its group "<<endl;
   pthread_cond_wait(&g_wait, &g_lock);                       //wait unitil the first group is finished
   
   }
   pthread_mutex_unlock(&g_lock);                             //release group lock

   pthread_mutex_lock(&p_lock[c.pos]);                       //acquire posion lock 
   if(database[c.pos]!=false){                               // if db position is not free
        pwcount++;
        cout<<"user"<<c.id <<" is waiting: position "<<c.pos<<" is being used by user "<<db_used_by[c.pos]<<endl;
        pthread_cond_wait(&p_wait[c.pos], &p_lock[c.pos]);  //wait until positinn is freed

   }
    pthread_mutex_unlock(&p_lock[c.pos]);                   //relase position lock 
   
    cout<<"user"<<c.id <<" is accesinng the position "<<c.pos<<" of the database for 5 secound(s)"<<endl;
    db_used_by[c.pos]=c.id;                                // store the id of user accessinng the position of db
    database[c.pos]=true;                                  // set accesed to true so other user can't acces this position
    sleep(c.wTime);                                        // sleep for request time
     cout<<"user"<<c.id <<" finished its execution"<<endl; // user is done
    pthread_mutex_lock(&p_lock[c.pos]);                    // acquire position lock
    pthread_cond_signal(&p_wait[c.pos]);                 // signal the users tha are waitinng for db postion that they can now acces db (line #63)
    pthread_mutex_unlock(&p_lock[c.pos]);                  //release position lock
    database[c.pos]=false;                                 //set position to free
    gcount[c.group-1]--;                                   // decremennt the request of users of his group
    pthread_mutex_lock(&g_lock);
    if(gcount[sgroup-1]==0 && file_reading){               // if first group finished and no request exist as file reading finished

          pthread_cond_signal(&g_wait);                    // signal the next  group users that they can now proceed(line #54)

        }
   pthread_mutex_unlock(&g_lock);
	
}

int main()
{
	
        for(int i=0;i<10;i++){

         p_wait[i]=PTHREAD_COND_INITIALIZER;
         p_lock[i]=PTHREAD_MUTEX_INITIALIZER;

        }

	cin >> sgroup;
	int group;
	int pos;
	int sTime; 
	int wTime;
	int total = 1; //userID number
        int i=0;
	while (cin >> group >> pos >> sTime >> wTime) // takes input
	{    sleep(sTime);                           //sleep until new request is initiated
             gcount[group-1]++;
             gcount_copy[group-1]++;
             sector temp(group,pos,sTime,wTime,total);
             if(pthread_create(&tid[i], NULL, access_database,(void *)&temp)) 
		{
			cerr << "Error creating thread\n";
			return 1;

		}
		total++;
                i++;
	}

      file_reading=true;   //all requests are read
      for (int i = 0; i <=total; i++)
        	pthread_join(tid[i], NULL);


       cout<<"\ntotal requests:"<<endl;
       cout<<"Group 1:"<<gcount_copy[0]<<"\nGroup 2: "<<gcount_copy[1]<<endl;
       cout<<"Requests that waited:"<<endl;
       cout<<"due to its group: "<<gwcount<<"\nDue to locked position: "<<pwcount<<endl;

	return 0;
}


