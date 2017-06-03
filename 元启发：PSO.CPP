#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <cstring>
using namespace std;

const int test_times_max = 50;		//测试次数上限
const int particle_number = 100;	//种群规模
const int times_max = 300;			//迭代次数上限
const double VMAX = 1;				//速度上限
const int XMAX = 4;					//最大范围
const float WMAX = 1.2;				//最大惯性
const float WMIN = 0.4;				//最小惯性
const int BOUND_MAX = 50;			//不变上线次数跳出

int job_number, machine_number, setp_number;

class OPERATION{
public:
	int order;
	double weight;
	int op_job_number;
	OPERATION(double w = 0, int o = 0, int op = 0) :weight(w), order(o), op_job_number(op){};
	OPERATION(OPERATION& temp){
		order = temp.order;
		weight = temp.weight;
		op_job_number = temp.op_job_number;
	}
	OPERATION& operator =(OPERATION& temp){
		if (this == &temp) return *this;
		else{
			order = temp.order;
			weight = temp.weight;
			op_job_number = temp.op_job_number;
		}
		return *this;
	}
};
int cmp(const void *a, const void *b){
	return (*(OPERATION *)b).weight - (*(OPERATION *)a).weight;//降序排
};
int cmp_order(const void *a, const void *b){
	return (*(OPERATION *)a).order - (*(OPERATION *)b).order;//升序排
};

class PARTICLE{
public:
	int size;
	OPERATION* anrry;
	PARTICLE(int n = 0){
		size = n;
		anrry = new OPERATION[size + 1];
	};
	PARTICLE(PARTICLE& temp){
		size = temp.size;
		anrry = new OPERATION[size + 1];
		for (int i = 1; i <= size; i++){
			anrry[i] = temp.anrry[i];
		}
	};
	PARTICLE& operator=(PARTICLE& temp){
		if (this == &temp) return *this;
		delete[] anrry;
		size = temp.size;
		anrry = new OPERATION[size + 1];
		for (int i = 1; i <= size; i++){
			anrry[i] = temp.anrry[i];
		}
		return *this;
	}
	void sort(){
		qsort(&anrry[1], size, sizeof(anrry[1]), cmp);
	}
	void sort(int begin){
		begin--;
		qsort(&anrry[1 + begin*job_number], job_number, sizeof(anrry[1]), cmp);//第一个元素的位置，元素个数，元素大小，比较大小函数
	};
	void sort_by_order(){
		qsort(&anrry[1], size, sizeof(anrry[1]), cmp_order);
	};
	void SET(int n = 0){
		delete[] anrry;
		size = n;
		anrry = new OPERATION[size + 1];
	}
	~PARTICLE(){
		delete[] anrry;
	}
};
class VELOCITY
{
public:
	int size;
	float* speed;
	VELOCITY(int n = 0){
		size = n;
		speed = new float[size + 1];
	};
	void SET(int n = 0){
		delete[] speed;
		size = n;
		speed = new float[size + 1];
	};
	~VELOCITY(){
		delete[] speed;
	};
};
class MACHINE//记录该机器是否正在工作，并记录所剩时间
{
public:
	int used;
	int need_time;
	int next_job;
	MACHINE(int u = 0, int t = 0, int nj = -1) :used(u), need_time(t), next_job(nj){};
	void machine_init(){
		used = 0;
		need_time = 0;
		next_job = -1;
	}

};
class JOB{
public:
	int used;
	int need_time;
	int now_setp;
	JOB(int u = 0, int t = 0, int ns = 1) :used(u), need_time(t), now_setp(ns){};
	void job_init(){
		used = 0;
		need_time = 0;
		now_setp = 1;
	}
};



// int machine_of_job_setp[1010][1010],time_of_job_machine[1010][1010];//job_number,machine_number
// PARTICLE particle[particle_number+1],pbest_position[particle_number+1],gbest_position;//job_number*machine_number
// VELOCITY velocity[particle_number+1]//对于每一个粒子，有自己的速度向量
// int pbest[particle_number+1],gbest;



int machine_of_job_setp[1010][1010], time_of_job_setp[1010][1010];//job_number,setp_number
PARTICLE particle[particle_number + 1], pbest_position[particle_number + 1], gbest_position;//job_number*machine_number
VELOCITY velocity[particle_number + 1];//对于每一个粒子，有自己的速度向量
int pbest[particle_number + 1], gbest, before_gbest, again_times;
MACHINE machine_anrry[1010];//记录该机器是否正在工作，并记录所剩时间
JOB job_anrry[1010];//记录该工件是否正在工作，并记录所剩时间
int setp_anrry[1010];//这个判定粒子序列中每个操作是第几个操作可以共用。so不写入类内
int times = 0;//记录粒子搜索次数

int function(PARTICLE& particle, int count);
void adjust_velocity(int t, VELOCITY& velocity, PARTICLE& pbest_position, PARTICLE& gbest_position, PARTICLE& particle);
int test_function(PARTICLE& particle);
int main(){
	int the_best = INT_MAX;
	int the_worst = 0;
	int the_global_sum_time = 0;
	int test_times = 1;
	//time_t t1 = time(0);
	SYSTEMTIME  sys1;
	GetLocalTime(&sys1);
	ofstream f2("outla20.txt");
	for (test_times = 1; test_times <= test_times_max; test_times++){
		ifstream f1;
		f1.open("inla20.txt");
		srand((int)time(0));
		//f2 << (int)time(0) << endl;
		char c;
		f1 >> job_number >> machine_number;// >> c>> c>> setp_number >> c
		setp_number = machine_number;
		for (int i = 1; i <= job_number; i++){
			for (int j = 1; j <= setp_number; j++){
				f1 >> machine_of_job_setp[i][j] >> time_of_job_setp[i][j];
				machine_of_job_setp[i][j]++;
			}
		}
		for (int i = 1; i <= job_number; i++){
			job_anrry[i].job_init();
		}
		for (int i = 1; i <= machine_number; i++){
			machine_anrry[i].machine_init();
		}

		// int i = 0;
		//输入表
		for (int i = 1; i <= particle_number; i++){//对于每一个粒子
			particle[i].SET(job_number*setp_number);
			velocity[i].SET(job_number*setp_number);
			//为粒子分配空间
			for (int j = 1; j <= job_number; j++){//对于每个工件
				for (int k = 1; k <= setp_number; k++){//对于工件的每一个操作的权重
					int temp_operator_number = (j - 1)*setp_number + k;
					particle[i].anrry[temp_operator_number].order = temp_operator_number;
					particle[i].anrry[temp_operator_number].weight = (2 * rand() / double(RAND_MAX) - 1)*XMAX;//?权重与速度关系何时为最佳！！！！
					velocity[i].speed[temp_operator_number] = (2 * rand() / double(RAND_MAX) - 1)*VMAX;//?
				}
			}
		}
		//FIFO初始数据
		int temp_count = 1;
		int temp_min_time = INT_MAX, min_time = INT_MAX, sum_time = 0;
		while (1){
			int complete = 0;//判断是否所有的工件都完成加工
			for (int i = 1; i <= job_number; ++i){
				if (job_anrry[i].now_setp>setp_number)complete++;
			}

			if (complete == job_number)break;//如果全部工件都加工完就退出

			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 0){
					temp_min_time = INT_MAX;
					// int fing_flag=0;//判断找到下一个工件
					for (int j = 1; j <= job_number; j++){//找当前机器的下一个加工工件
						if (job_anrry[j].used == 0 && \
							job_anrry[j].now_setp <= setp_number && \
							machine_of_job_setp[j][job_anrry[j].now_setp] == i && \
							time_of_job_setp[j][job_anrry[j].now_setp]<temp_min_time
							){
							machine_anrry[i].next_job = j;
							temp_min_time = time_of_job_setp[j][job_anrry[j].now_setp];
						}
					}
					if (temp_min_time != INT_MAX){
						int next_job = machine_anrry[i].next_job;;
						machine_anrry[i].used = 1;
						machine_anrry[i].need_time = time_of_job_setp[next_job][job_anrry[next_job].now_setp];
						job_anrry[next_job].used = 1;
						job_anrry[next_job].need_time = machine_anrry[i].need_time;
						//job_anrry[next_job].now_setp++;
						particle[1].anrry[temp_count].order = temp_count;
						particle[1].anrry[temp_count].weight = next_job;
						temp_count++;
						//f2 << next_job << " ";
					}
				}
			}

			//找到最小时间
			min_time = INT_MAX;
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 1 && \
					machine_anrry[i].need_time<min_time)
					min_time = machine_anrry[i].need_time;
			}
			sum_time += min_time;
			//使用时间
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 1){
					machine_anrry[i].need_time -= min_time;
					if (machine_anrry[i].need_time <= 0){
						machine_anrry[i].need_time = 0;
						machine_anrry[i].next_job = -1;
						machine_anrry[i].used = 0;
					}
				}
			}
			//
			for (int i = 1; i <= job_number; i++){
				if (job_anrry[i].used == 1){
					job_anrry[i].need_time -= min_time;
					if (job_anrry[i].need_time <= 0){
						job_anrry[i].used = 0;
						job_anrry[i].need_time = 0;
						job_anrry[i].now_setp++;
					}
				}
			}
			int debug = 1;
		}
		//FIFO初始化完毕
		//初始化完毕
		again_times = 0;
		before_gbest = INT_MAX;
		gbest = INT_MAX;
		gbest_position.SET(job_number*setp_number);
		//pbest_position的初始化在生成粒子部分
		for (int i = 1; i <= particle_number; i++){
			pbest[i] = INT_MAX;
			pbest_position[i].SET(job_number*setp_number);
		}
		//开始迭代
		for (times = 1; times <= times_max; times++){//迭代times_max次
			before_gbest = gbest;
			for (int j = 1; j <= particle_number; j++){//对于每一个粒子
				int now = function(particle[j], j);//第几个粒子
				if (now<pbest[j]){
					pbest[j] = now;
					pbest_position[j] = particle[j];
				}
				if (now<gbest){
					gbest = now;
					gbest_position = particle[j];
				}
			}
			int z = 1;
			for (int j = 1; j <= particle_number; j++){
				adjust_velocity(times, velocity[j], pbest_position[j], gbest_position, particle[j]);
				//修改v，修改p
			}
			if (before_gbest == gbest)again_times++;
			else again_times = 0;
			if (again_times >= BOUND_MAX)break;
		}

		//for(int i=1;i<=setp_number;i++){
		//	for (int j = 1; j <= job_number; j++){
		//		f2 << gbest_position.anrry[(i-1)*job_number+j].op_job_number << " ";
		//	}
		//	f2 << endl;
		//}
		//f2 << times << endl;//输出遍历次数
		// f2  << endl;
		f2 << "迭代次数: " << times << " 迭代过程中的最优解: " << gbest << endl;
		cout << "迭代次数: " << times << " 迭代过程中的最优解: " << gbest << endl;
		the_global_sum_time += gbest;
		if (the_best>gbest)the_best = gbest;
		if (the_worst<gbest)the_worst = gbest;
		f1.close();
	}


	SYSTEMTIME sys2;
	GetLocalTime(&sys2);
	//time_t t2 = time(0);
	cout << "文件开启时间: " << sys1.wMinute << sys1.wMilliseconds << endl;
	cout << "文件关闭时间: " << sys2.wMinute << sys2.wMilliseconds << endl;
	f2 << "最好解: " << the_best << endl;
	f2 << "最坏解: " << the_worst << endl;
	f2 << "平均解: " << the_global_sum_time / test_times_max << endl;
	cout << "最好解: " << the_best << endl;
	cout << "最坏解: " << the_worst << endl;
	cout << "平均解: " << the_global_sum_time / test_times_max << endl;
	//f2 << "平均时间: " << ( 60*double(sys2.wHour-sys1.wHour)+(double(sys2.wMinute-sys1.wMinute)+(double(sys2.wMilliseconds - sys1.wMilliseconds)/1000000)))/ test_times_max << endl;
	f2 << "平均时间: " << (60 * double(sys2.wMinute - sys1.wMinute) + (double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	cout << "平均时间: " << ((60 * double(sys2.wMinute - sys1.wMinute) + double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	f2.close();
	return 0;
}
int function(PARTICLE& particle, int count){//自适应函数，找到所用时间
	PARTICLE &temp = particle;
	temp.sort();
	for (int i = job_number; i >= 1; i--){
		for (int j = 1; j <= setp_number; j++){
			int temp_operator_number = (job_number - i)*setp_number + j;
			temp.anrry[temp_operator_number].op_job_number = i;
		}
	}
	temp.sort_by_order();
	//for(int i=1;i<=setp_number;i++){//整形
	//	temp.sort(i);
	//	for(int j=job_number;j>=1;--j){
	//		int temp_operator_number=(i-1)*job_number+(job_number-j+1);
	//		temp.anrry[temp_operator_number].op_job_number=j;
	//	}
	//}
	//temp.sort_by_order();
	//计算当前的工序序列所需的时间
	memset(job_anrry, 0, sizeof(job_anrry));
	memset(machine_anrry, 0, sizeof(machine_anrry));
	memset(setp_anrry, 0, sizeof(setp_anrry));
	int using_time = 0;
	for (int i = 1; i <= temp.size; i++){//对于粒子的每一项
		int now_job_number = temp.anrry[i].op_job_number;
		int now_machine_number = machine_of_job_setp[temp.anrry[i].op_job_number][++setp_anrry[now_job_number]];
		int now_need_time = time_of_job_setp[temp.anrry[i].op_job_number][setp_anrry[now_job_number]];

		if (machine_anrry[now_machine_number].used == 1 || job_anrry[now_job_number].used == 1){
			int reduce_time;//找到统一减去的时间
			if (machine_anrry[now_machine_number].need_time>job_anrry[now_job_number].need_time)reduce_time = machine_anrry[now_machine_number].need_time;//找到统一减去的时间
			else reduce_time = job_anrry[now_job_number].need_time;
			using_time += reduce_time;//时间累加

			for (int j = 1; j <= machine_number; j++){
				machine_anrry[j].need_time -= reduce_time;
				if (machine_anrry[j].need_time <= 0){
					machine_anrry[j].need_time = 0;
					machine_anrry[j].used = 0;
				}
			}

			for (int j = 1; j <= job_number; j++){
				job_anrry[j].need_time -= reduce_time;
				if (job_anrry[j].need_time <= 0){
					job_anrry[j].need_time = 0;
					job_anrry[j].used = 0;
				}
			}
		}
		job_anrry[now_job_number].used = 1;
		job_anrry[now_job_number].need_time = now_need_time;
		machine_anrry[now_machine_number].used = 1;
		machine_anrry[now_machine_number].need_time = now_need_time;
	}
	//把剩下的操作做完
	int last_max_time = 0;
	for (int i = 1; i <= machine_number; i++){
		if (machine_anrry[i].need_time>last_max_time)last_max_time = machine_anrry[i].need_time;
	}
	using_time += last_max_time;
	return using_time;
}
// adjust_velocity(velocity[j],pbest_position[j],gbest_position,particle[j]);
// //修改v，修改p

void adjust_velocity(int t, VELOCITY& velocity, PARTICLE& pbest_position, PARTICLE& gbest_position, PARTICLE& particle){
	int size = velocity.size;
	double w = WMAX - (WMAX - WMIN)*t / times_max;
	float c1 = 2, c2 = 2;
	double r1 = rand() / double(RAND_MAX), r2 = rand() / double(RAND_MAX);
	for (int i = 1; i <= size; i++){
		velocity.speed[i] = w*velocity.speed[i] + c1*r1*(pbest_position.anrry[i].weight - particle.anrry[i].weight) + c2*r2*(gbest_position.anrry[i].weight - particle.anrry[i].weight);
		if (velocity.speed[i] > VMAX)velocity.speed[i] = (1 - 0.1*rand() / double(RAND_MAX))*VMAX;// (1 - 0.1*rand() / double(RAND_MAX))*VMAX
		if (velocity.speed[i] < -VMAX)velocity.speed[i] = -(1 - 0.1*rand() / double(RAND_MAX))*VMAX;
		particle.anrry[i].weight += velocity.speed[i];
		//if (particle.anrry[i].weight>XMAX)particle.anrry[i].weight = (1 - 0.1*rand() / double(RAND_MAX))*XMAX;//(1-0.1*rand()/double(RAND_MAX))*XMAX
		//if (particle.anrry[i].weight<-XMAX)particle.anrry[i].weight = -(1 - 0.1*rand() / double(RAND_MAX))*XMAX;
	}
}

////////////////////////////////////////////////////////////
